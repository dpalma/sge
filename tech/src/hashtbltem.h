///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_HASHTBLTEM_H
#define INCLUDED_HASHTBLTEM_H

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

HASHTABLE_TEMPLATE::cHashTable(int initialSize)
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
   Clear();
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(void)::Clear()
{
   Reset(0);
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(void)::Reset(int newInitialSize)
{
   Assert(m_nItersActive == 0); // don't clear while iterating

   m_allocator.deallocate(m_elts, m_size);
   m_elts = NULL;
   m_size = 0;
   m_count = 0;

   if (newInitialSize > 0)
      Grow(newInitialSize);
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(bool)::Set(const KEY & k, const VALUE & v)
{
   if (m_count*100 > m_size*kFullnessThreshold)
      Grow(m_size + m_count*100/kFullnessThreshold); // grow in proportion to fullness

   uint hash = Probe(k);

   m_elts[hash].key = k;
   m_elts[hash].value = v;
   m_elts[hash].inUse = true;

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
   uint hash = Probe(k);

   if (!m_elts[hash].inUse)
      return false;

   if (v != NULL)
      *v = m_elts[hash].value;

   return true;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(bool)::Delete(const KEY & k)
{
   Assert(m_nItersActive == 0); // don't delete while iterating

   uint hash = Probe(k);

   if (!m_elts[hash].inUse)
      return false;

   m_elts[hash].inUse = false;
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

HASHTABLE_TEMPLATE_(bool)::IterNext(HANDLE * phIter, KEY * pk, VALUE * pv) const
{
   unsigned int & index = (unsigned int &)*phIter;
   if (index < m_size)
   {
      while (!m_elts[index].inUse && index < m_size)
         index++;
      if (index == m_size)
         return false;
      *pk = m_elts[index].key;
      *pv = m_elts[index].value;
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
   uint hash = Hash(k) & (m_size - 1);

#ifdef _DEBUG
   uint start = hash;
   bool wrapped = false;
#endif

   // resolve collisions with linear probing
   while (m_elts[hash].inUse && !Equal(k, m_elts[hash].key))
   {
      hash++;
      if (hash == m_size)
      {
         hash = 0;
#ifdef _DEBUG
         wrapped = true;
#endif
      }

#ifdef _DEBUG
      if (wrapped && (hash >= start))
      {
         DebugMsg("ERROR: cHashTable is 100% full!!!\n");
         Assert(!"ERROR: cHashTable is 100% full!!!");
      }
#endif
   }

   return hash;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(void)::Grow(uint newSize)
{
   uint actualNewSize = NearestPowerOfTwo(newSize);
   Assert(actualNewSize <= newSize); // NearestPowerOfTwo returns lower than argument
   newSize = actualNewSize * 2;

   tHashElement * newElts = m_allocator.allocate(newSize, m_elts);

   // Use placement new to call the constructor for each element
#undef new
   newElts = new(newElts)tHashElement[newSize];
#define new DebugNew

   int oldSize = m_size;
   int oldCount = m_count;
   tHashElement * oldElts = m_elts;

   m_count = 0;
   m_size = newSize;
   m_elts = newElts;

   for (int i = 0; i < oldSize; i++)
   {
      if (oldElts[i].inUse)
      {
         Insert(oldElts[i].key, oldElts[i].value);
      }
   }

   m_allocator.deallocate(oldElts, oldSize);
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(bool)::Equal(const KEY & k1, const KEY & k2) const
{
   return k1 == k2;
}

///////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_HASHTBLTEM_H
