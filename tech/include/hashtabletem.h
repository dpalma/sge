///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_HASHTABLETEM_H
#define INCLUDED_HASHTABLETEM_H

#include "hash.h"
#include "techmath.h"

#include <cmath>

#include "dbgalloc.h" // must be last header

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cHashTable
//

// for constructors/destructor
#define HASHTABLE_TEMPLATE \
   template <typename KEY, typename VALUE, class ALLOCATOR> cHashTable<KEY, VALUE, ALLOCATOR>
// for methods
#define HASHTABLE_TEMPLATE_(RetType) \
   template <typename KEY, typename VALUE, class ALLOCATOR> RetType cHashTable<KEY, VALUE, ALLOCATOR>

///////////////////////////////////////

const int kFullnessThreshold = 70;

///////////////////////////////////////

HASHTABLE_TEMPLATE::cHashTable(uint initialSize)
 : m_elts(NULL), m_size(0), m_count(0)
#ifndef NDEBUG
   ,m_nItersActive(0)
#endif
{
   Grow(initialSize);
}

///////////////////////////////////////

HASHTABLE_TEMPLATE::~cHashTable()
{
   Reset(0);
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(void)::Clear()
{
   Reset();
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(bool)::Set(const KEY & k, const VALUE & v)
{
   if ((m_count * 100) > (m_size * kFullnessThreshold))
   {
      // grow in proportion to fullness
      Grow(m_size + (m_count * 100 / kFullnessThreshold));
   }

   uint h = Probe(k);

   m_elts[h].key = k;
   m_elts[h].value = v;
   m_elts[h].inUse = true;

   return true;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(bool)::Insert(const KEY & k, const VALUE & v)
{
   Assert(m_nItersActive == 0); // don't insert while iterating

   if (!Lookup(k, NULL))
   {
      if (Set(k, v))
      {
         m_count++;
         return true;
      }
   }
   return false;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(bool)::Lookup(const KEY & k, VALUE * v) const
{
   uint h = Probe(k);

   if (!m_elts[h].inUse)
   {
      return false;
   }

   if (v != NULL)
   {
      *v = m_elts[h].value;
   }

   return true;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(bool)::Delete(const KEY & k)
{
   Assert(m_nItersActive == 0); // don't delete while iterating

   uint h = Probe(k);

   if (!m_elts[h].inUse)
   {
      return false;
   }

   m_elts[h].inUse = false;
   m_count--;

   return true;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(void)::IterBegin(HANDLE * phIter) const
{
#ifndef NDEBUG
   m_nItersActive++;
#endif
   *phIter = 0;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(bool)::IterNext(HANDLE * phIter, KEY * pKey, VALUE * pValue) const
{
   unsigned int & index = (unsigned int &)*phIter;
   if (index < m_size)
   {
      while (!m_elts[index].inUse && (index < m_size))
      {
         index++;
      }
      Assert(index <= m_size);
      if (index == m_size)
      {
         return false;
      }
      if (pKey != NULL)
      {
         *pKey = m_elts[index].key;
      }
      if (pValue != NULL)
      {
         *pValue = m_elts[index].value;
      }
      index++;
      return true;
   }
   return false;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(void)::IterEnd(HANDLE * phIter) const
{
   *phIter = (HANDLE)-1;
#ifndef NDEBUG
   m_nItersActive--;
#endif
}

///////////////////////////////////////
// Probe computes the hash for a key and resolves collisions

HASHTABLE_TEMPLATE_(uint)::Probe(const KEY & k) const
{
   Assert(IsPowerOfTwo(m_size));
   uint h = Hash(k) & (m_size - 1);

#ifdef _DEBUG
   uint start = h;
   bool wrapped = false;
#endif

   // resolve collisions with linear probing
   while (m_elts[h].inUse && !Equal(k, m_elts[h].key))
   {
      h++;
      if (h == m_size)
      {
         h = 0;
#ifdef _DEBUG
         wrapped = true;
#endif
      }

#ifdef _DEBUG
      if (wrapped && (h >= start))
      {
         DebugMsg("ERROR: cHashTable is 100% full!!!\n");
         Assert(!"ERROR: cHashTable is 100% full!!!");
      }
#endif
   }

   return h;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(void)::Grow(uint newSize)
{
   uint actualNewSize = NearestPowerOfTwo(newSize);
   Assert(actualNewSize <= newSize); // NearestPowerOfTwo returns lower than argument

   if (actualNewSize < newSize)
   {
      actualNewSize *= 2;
   }

   tHashElement * newElts = m_allocator.allocate(actualNewSize, m_elts);

   // Use placement new to call the constructor for each element
#ifdef DBGALLOC_MAPPED
#undef new
#endif
   newElts = new(newElts)tHashElement[actualNewSize];
#ifdef DBGALLOC_MAPPED
#define new DebugNew
#endif

#ifdef _DEBUG
   int oldCount = m_count;
#endif
   int oldSize = m_size;
   tHashElement * oldElts = m_elts;

   m_count = 0;
   m_size = actualNewSize;
   m_elts = newElts;

   for (int i = 0; i < oldSize; i++)
   {
      if (oldElts[i].inUse)
      {
         Insert(oldElts[i].key, oldElts[i].value);
      }
   }

   Assert(m_count == oldCount);

   m_allocator.deallocate(oldElts, oldSize);
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(bool)::Equal(const KEY & k1, const KEY & k2) const
{
   return k1 == k2;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(void)::Reset(uint newInitialSize)
{
   Assert(m_nItersActive == 0); // don't clear while iterating

   m_allocator.deallocate(m_elts, m_size);
   m_elts = NULL;
   m_size = 0;
   m_count = 0;

   if (newInitialSize > 0)
   {
      Grow(newInitialSize);
   }
}

///////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_HASHTABLETEM_H
