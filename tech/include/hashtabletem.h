////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
// Explicit template instantiations for basic types

////////////////////////////////////////

#define HASHFUNCTION_FOR_SIMPLE_TYPE(type) \
template <> inline uint cHashFunction<type>::Hash(const type & a, uint initHash) \
{ return hash((byte *)&a, sizeof(a), initHash); } \
template <> inline bool cHashFunction<type>::Equal(const type & a, const type & b) \
{ return (a == b); }

HASHFUNCTION_FOR_SIMPLE_TYPE(int)
HASHFUNCTION_FOR_SIMPLE_TYPE(uint)
HASHFUNCTION_FOR_SIMPLE_TYPE(long)
HASHFUNCTION_FOR_SIMPLE_TYPE(ulong)
HASHFUNCTION_FOR_SIMPLE_TYPE(short)
HASHFUNCTION_FOR_SIMPLE_TYPE(ushort)

////////////////////////////////////////

typedef const char * tPCSTR;

template <>
inline uint cHashFunction<tPCSTR>::Hash(const tPCSTR & a, uint initVal)
{
   return hash((byte *)a, strlen(a), initVal);
}

template <>
inline bool cHashFunction<tPCSTR>::Equal(const tPCSTR & a, const tPCSTR & b)
{
   return (stricmp(a, b) == 0);
}


////////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cHashIterator
//

////////////////////////////////////////

template <typename HASHELEMENT>
cHashIterator<HASHELEMENT>::cHashIterator()
 : m_pElement(NULL)
 , m_pBegin(NULL)
 , m_pEnd(NULL)
{
}

////////////////////////////////////////

template <typename HASHELEMENT>
cHashIterator<HASHELEMENT>::cHashIterator(const cHashIterator & other)
 : m_pElement(other.m_pElement)
 , m_pBegin(other.m_pBegin)
 , m_pEnd(other.m_pEnd)
{
}

////////////////////////////////////////

template <typename HASHELEMENT>
cHashIterator<HASHELEMENT>::cHashIterator(HASHELEMENT * pElement, HASHELEMENT * pBegin, HASHELEMENT * pEnd)
 : m_pElement(pElement)
 , m_pBegin(pBegin)
 , m_pEnd(pEnd)
{
   while ((m_pElement < m_pEnd) && !m_pElement->inUse)
   {
      ++m_pElement;
   }
}

////////////////////////////////////////

template <typename HASHELEMENT>
const cHashIterator<HASHELEMENT> & cHashIterator<HASHELEMENT>::operator =(const cHashIterator & other)
{
   m_pElement = other.m_pElement;
   m_pBegin = other.m_pBegin;
   m_pEnd = other.m_pEnd;
   return *this;
}

////////////////////////////////////////

template <typename HASHELEMENT>
typename cHashIterator<HASHELEMENT>::pointer cHashIterator<HASHELEMENT>::operator ->() const
{
   return m_pElement;
}

////////////////////////////////////////

template <typename HASHELEMENT>
typename cHashIterator<HASHELEMENT>::reference cHashIterator<HASHELEMENT>::operator *() const
{
   return *m_pElement;
}

////////////////////////////////////////
// preincrement

template <typename HASHELEMENT>
const cHashIterator<HASHELEMENT> & cHashIterator<HASHELEMENT>::operator ++()
{
   do {
      ++m_pElement;
   } while ((m_pElement < m_pEnd) && !m_pElement->inUse);
   return *this;
}

////////////////////////////////////////
// postincrement

template <typename HASHELEMENT>
const cHashIterator<HASHELEMENT> cHashIterator<HASHELEMENT>::operator ++(int)
{
   const cHashIterator temp(*this);
   operator ++();
   return temp;
}

////////////////////////////////////////
// predecrement

template <typename HASHELEMENT>
const cHashIterator<HASHELEMENT> & cHashIterator<HASHELEMENT>::operator --()
{
   do {
      --m_pElement;
   } while ((m_pElement >= m_pBegin) && !m_pElement->inUse);
   return *this;
}

////////////////////////////////////////
// postdecrement

template <typename HASHELEMENT>
const cHashIterator<HASHELEMENT> cHashIterator<HASHELEMENT>::operator --(int)
{
   const cHashIterator temp(*this);
   operator --();
   return temp;
}

////////////////////////////////////////

template <typename HASHELEMENT>
bool cHashIterator<HASHELEMENT>::operator ==(const cHashIterator & other) const
{
   return (m_pElement == other.m_pElement);
}

////////////////////////////////////////

template <typename HASHELEMENT>
bool cHashIterator<HASHELEMENT>::operator !=(const cHashIterator & other) const
{
   return (!(*this == other));
}


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cHashTable
//

#define HASHTABLE_TEMPLATE_DECL \
   template <typename KEY, typename VALUE, typename HASHFN, class ALLOCATOR>
#define HASHTABLE_TEMPLATE_CLASS \
   cHashTable<KEY, VALUE, HASHFN, ALLOCATOR>
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
#define HASHTABLE_TEMPLATE_MEMBER_TYPE(Type) \
   HASHTABLE_TEMPLATE_CLASS::Type
#else
#define HASHTABLE_TEMPLATE_MEMBER_TYPE(Type) \
   typename HASHTABLE_TEMPLATE_CLASS::Type
#endif

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_CLASS::cHashTable(uint initialSize)
 : m_elts(NULL)
 , m_maxSize(0)
 , m_size(0)
{
   Grow(initialSize);
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_CLASS::cHashTable(const cHashTable & other)
 : m_elts(NULL)
 , m_maxSize(0)
 , m_size(0)
{
   Grow(other.m_maxSize);
   for (uint i = 0; i < m_maxSize; i++)
   {
      m_allocator.construct(&m_elts[i], other.m_elts[i]);
   }
   m_size = other.m_size;
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_CLASS::~cHashTable()
{
   Reset(0);
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
std::pair<HASHTABLE_TEMPLATE_MEMBER_TYPE(const_iterator), bool>
HASHTABLE_TEMPLATE_CLASS::insert(const KEY & k, const VALUE & v)
{
   uint h = Probe(k);
   if (m_elts[h].inUse)
   {
      return std::make_pair(const_iterator(&m_elts[h], &m_elts[0], &m_elts[m_maxSize]), false);
   }

   if ((m_size * 100) > (m_maxSize * kFullnessThreshold))
   {
      // grow in proportion to fullness
      Grow(m_maxSize + (m_size * 100 / kFullnessThreshold));

      // re-do the probe
      h = Probe(k);
   }

   m_elts[h].first = k;
   m_elts[h].second = v;
   m_elts[h].inUse = true;
   m_size++;
   return std::make_pair(const_iterator(&m_elts[h], &m_elts[0], &m_elts[m_maxSize]), true);
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
VALUE & HASHTABLE_TEMPLATE_CLASS::operator [](const KEY & k)
{
   if ((m_size * 100) > (m_maxSize * kFullnessThreshold))
   {
      // grow in proportion to fullness
      Grow(m_maxSize + (m_size * 100 / kFullnessThreshold));
   }

   uint h = Probe(k);
   m_elts[h].first = k;
   if (!m_elts[h].inUse)
   {
      m_elts[h].inUse = true;
      m_size++;
   }
   return m_elts[h].second;
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_MEMBER_TYPE(iterator) HASHTABLE_TEMPLATE_CLASS::find(const KEY & k)
{
   uint h = Probe(k);
   if (m_elts[h].inUse)
   {
      return iterator(&m_elts[h], &m_elts[0], &m_elts[m_maxSize]);
   }
   else
   {
      return end();
   }
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_MEMBER_TYPE(const_iterator) HASHTABLE_TEMPLATE_CLASS::find(const KEY & k) const
{
   uint h = Probe(k);
   if (m_elts[h].inUse)
   {
      return const_iterator(&m_elts[h], &m_elts[0], &m_elts[m_maxSize]);
   }
   else
   {
      return end();
   }
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_MEMBER_TYPE(size_type) HASHTABLE_TEMPLATE_CLASS::erase(const KEY & k)
{
   uint h = Probe(k);

   if (!m_elts[h].inUse)
   {
      return 0;
   }

   m_allocator.destroy(&m_elts[h]);
   m_elts[h].inUse = false;
   m_size--;

   Assert(IsPowerOfTwo(m_maxSize));
   uint rawHash = HASHFN::Hash(k) & (m_maxSize - 1); // hash w/o linear probing for collisions

   h += 1;
   bool bWrapped = false;
   for (uint start = h, prev = (h - 1); true; prev = h, h++)
   {
      if (h == m_maxSize)
      {
         h = 0;
         bWrapped = true;
      }

      if (bWrapped && (h >= start))
      {
         DebugMsg("ERROR: cHashTable is 100% full!!!\n");
         Assert(!"ERROR: cHashTable is 100% full!!!");
         break;
      }

      if (!m_elts[h].inUse)
      {
         break;
      }
      else
      {
         uint rawHash2 = HASHFN::Hash(m_elts[h].first) & (m_maxSize - 1); // hash w/o linear probing for collisions
         if (rawHash2 != rawHash)
         {
            break;
         }
         else
         {
            m_allocator.construct(&m_elts[prev], m_elts[h]);
            m_elts[prev].inUse = true;
            m_allocator.destroy(&m_elts[h]);
            m_elts[h].inUse = false;
         }
      }
   }


   return 1;
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
void HASHTABLE_TEMPLATE_CLASS::clear()
{
   Reset();
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
bool HASHTABLE_TEMPLATE_CLASS::empty() const
{
   return (m_size == 0);
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_MEMBER_TYPE(size_type) HASHTABLE_TEMPLATE_CLASS::size() const
{
   return m_size;
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_MEMBER_TYPE(size_type) HASHTABLE_TEMPLATE_CLASS::max_size() const
{
   return m_maxSize;
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_MEMBER_TYPE(iterator) HASHTABLE_TEMPLATE_CLASS::begin()
{
   return (m_elts != NULL) ? iterator(&m_elts[0], &m_elts[0], &m_elts[m_maxSize]) : iterator();
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_MEMBER_TYPE(iterator) HASHTABLE_TEMPLATE_CLASS::end()
{
   return (m_elts != NULL) ? iterator(&m_elts[m_maxSize], &m_elts[0], &m_elts[m_maxSize]) : iterator();
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_MEMBER_TYPE(const_iterator) HASHTABLE_TEMPLATE_CLASS::begin() const
{
   return (m_elts != NULL) ? const_iterator(&m_elts[0], &m_elts[0], &m_elts[m_maxSize]) : const_iterator();
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_MEMBER_TYPE(const_iterator) HASHTABLE_TEMPLATE_CLASS::end() const
{
   return (m_elts != NULL) ? const_iterator(&m_elts[m_maxSize], &m_elts[0], &m_elts[m_maxSize]) : const_iterator();
}

////////////////////////////////////////
// Probe computes the hash for a key and resolves collisions

HASHTABLE_TEMPLATE_DECL
uint HASHTABLE_TEMPLATE_CLASS::Probe(const KEY & k) const
{
   Assert(IsPowerOfTwo(m_maxSize));
   uint h = HASHFN::Hash(k) & (m_maxSize - 1);

#ifdef _DEBUG
   uint start = h;
   bool wrapped = false;
#endif

   // resolve collisions with linear probing
   while (m_elts[h].inUse && !Equal(k, m_elts[h].first))
   {
      h++;
      if (h == m_maxSize)
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

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
void HASHTABLE_TEMPLATE_CLASS::Grow(uint newSize)
{
   uint actualNewSize = NearestPowerOfTwo(newSize);
   Assert(actualNewSize <= newSize); // NearestPowerOfTwo returns lower than argument

   if (actualNewSize < newSize)
   {
      actualNewSize *= 2;
   }

   value_type * newElts = m_allocator.allocate(actualNewSize, m_elts);

   // Use placement new to call the constructor for each element
#ifdef DBGALLOC_MAPPED
#undef new
#endif
   newElts = new(newElts)value_type[actualNewSize];
#ifdef DBGALLOC_MAPPED
#define new DebugNew
#endif

#ifdef _DEBUG
   int oldCount = m_size;
#endif
   int oldSize = m_maxSize;
   value_type * oldElts = m_elts;

   m_size = 0;
   m_maxSize = actualNewSize;
   m_elts = newElts;

   for (int i = 0; i < oldSize; i++)
   {
      if (oldElts[i].inUse)
      {
         insert(oldElts[i].first, oldElts[i].second);
      }
   }

   Assert(m_size == oldCount);

   m_allocator.deallocate(oldElts, oldSize);
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
bool HASHTABLE_TEMPLATE_CLASS::Equal(const KEY & k1, const KEY & k2) const
{
   return HASHFN::Equal(k1, k2);
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
void HASHTABLE_TEMPLATE_CLASS::Reset(uint newInitialSize)
{
   m_allocator.deallocate(m_elts, m_maxSize);
   m_elts = NULL;
   m_maxSize = 0;
   m_size = 0;

   if (newInitialSize > 0)
   {
      Grow(newInitialSize);
   }
}

////////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_HASHTABLETEM_H
