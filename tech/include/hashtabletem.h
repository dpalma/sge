///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_HASHTBLTEM_H
#define INCLUDED_HASHTBLTEM_H

#include "hash.h"

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
   template <typename KEY, typename VALUE> cHashTable<KEY, VALUE>
// for methods
#define HASHTABLE_TEMPLATE_(RetType) \
   template <typename KEY, typename VALUE> RetType cHashTable<KEY, VALUE>

///////////////////////////////////////

const int kFullnessThreshold = 70;

///////////////////////////////////////

HASHTABLE_TEMPLATE::cHashTable(int initialSize)
 : m_eltFull(NULL), m_elts(NULL), m_size(0), m_count(0)
#ifndef NDEBUG
   ,m_cItersActive(0)
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
   Assert(m_cItersActive == 0); // don't clear while iterating

   delete [] m_elts;
   m_elts = NULL;
   delete [] m_eltFull;
   m_eltFull = NULL;
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
   m_eltFull[hash] = true;

   return true;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(bool)::Insert(const KEY & k, const VALUE & v)
{
   Assert(m_cItersActive == 0); // don't insert while iterating

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

   if (!m_eltFull[hash])
      return false;

   if (v != NULL)
      *v = m_elts[hash].value;

   return true;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(bool)::Delete(const KEY & k)
{
   Assert(m_cItersActive == 0); // don't delete while iterating

   uint hash = Probe(k);

   if (!m_eltFull[hash])
      return false;

   m_eltFull[hash] = FALSE;
   m_count--;

   // @TODO (dpalma 6/22/01): To be extra thorough, the value should be removed
   // from m_elts. However, we can probably get away with just marking it as
   // not used, because to remove it would (i think) require re-hashing all
   // remaining elements.

   return true;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(void)::IterBegin(HANDLE * phIter) const
{
#ifndef NDEBUG
   m_cItersActive++;
#endif
   *phIter = 0;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(bool)::IterNext(HANDLE * phIter, KEY * pk, VALUE * pv) const
{
   unsigned int & index = (unsigned int &)*phIter;
   if (index < m_size)
   {
      while (!m_eltFull[index] && index < m_size)
         index++;
      if (index == m_size)
         return FALSE;
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
   m_cItersActive--;
#endif
}

///////////////////////////////////////
// Probe computes the hash for a key and resolves collisions

HASHTABLE_TEMPLATE_(uint)::Probe(const KEY & k) const
{
   uint hash = ::Hash(k) % m_size;

   // resolve collisions with linear probing
   // @TODO (dpalma 6/15/00): This doesn't handle the case where the hash table
   // is 100% full. Would likely go infinite in that case.
   while (m_eltFull[hash] && !Equal(k, m_elts[hash].key))
   {
      hash++;
      if (hash == m_size)
         hash = 0;
   }

   return hash;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(bool)::Grow(int newSize)
{
   sHashElement * newElts = new sHashElement[newSize];
   uint8 * newEltFull = new uint8[newSize];
   memset(newEltFull, 0, sizeof(uint8) * newSize);

   int oldSize = m_size;
   int oldCount = m_count;
   uint8 * oldEltFull = m_eltFull;
   sHashElement* oldElts = m_elts;

   m_count = 0;
   m_size = newSize;
   m_elts = newElts;
   m_eltFull = newEltFull;

   for (int i = 0; i < oldSize; i++)
   {
      if (oldEltFull[i])
         Insert(oldElts[i].key, oldElts[i].value);
   }

   delete [] oldElts;
   delete [] oldEltFull;

   return true;
}

///////////////////////////////////////

HASHTABLE_TEMPLATE_(bool)::Equal(const KEY & k1, const KEY & k2) const
{
   return k1 == k2;
}

///////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_HASHTBLTEM_H
