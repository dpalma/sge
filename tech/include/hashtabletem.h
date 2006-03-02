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
   while ((m_pElement < m_pEnd) && (m_pElement->state != kHES_InUse))
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
   } while ((m_pElement < m_pEnd) && (m_pElement->state != kHES_InUse));
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
   } while ((m_pElement >= m_pBegin) && (m_pElement->state != kHES_InUse));
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
HASHTABLE_TEMPLATE_CLASS::cHashTable()
 : m_elts(NULL)
 , m_maxSize(0)
 , m_size(0)
 , m_loadFactor(kDefaultLoadFactor)
{
   reserve(kInitialSizeSmall);
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_CLASS::cHashTable(size_type initialSize)
 : m_elts(NULL)
 , m_maxSize(0)
 , m_size(0)
 , m_loadFactor(kDefaultLoadFactor)
{
   reserve(initialSize);
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_CLASS::cHashTable(size_type initialSize, const allocator_type & alloc)
 : m_allocator(alloc)
 , m_elts(NULL)
 , m_maxSize(0)
 , m_size(0)
 , m_loadFactor(kDefaultLoadFactor)
{
   reserve(initialSize);
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_CLASS::cHashTable(const cHashTable & other)
 : m_elts(NULL)
 , m_maxSize(0)
 , m_size(0)
 , m_loadFactor(other.m_loadFactor)
{
   reserve(other.m_maxSize);
   for (size_type i = 0; i < m_maxSize; i++)
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
void HASHTABLE_TEMPLATE_CLASS::set_load_factor(byte loadFactor)
{
   m_loadFactor = loadFactor;
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_MEMBER_TYPE(allocator_type) HASHTABLE_TEMPLATE_CLASS::get_allocator() const
{
   return m_allocator;
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
void HASHTABLE_TEMPLATE_CLASS::reserve(size_type capacity)
{
   size_type actual = NearestPowerOfTwo(capacity);
   Assert(actual <= capacity); // NearestPowerOfTwo returns lower than argument

   if (actual < capacity)
   {
      actual *= 2;
   }

   if (actual <= m_maxSize)
   {
      return;
   }

   element_type * newElts = m_allocator.allocate(actual, m_elts);

   size_type i;
   for (i = 0; i < actual; i++)
   {
      m_allocator.construct(&newElts[i], element_type());
   }

#ifdef _DEBUG
   size_type oldSize = m_size;
#endif
   size_type oldMaxSize = m_maxSize;
   element_type * oldElts = m_elts;

   m_size = 0;
   m_maxSize = actual;
   m_elts = newElts;

   for (i = 0; i < oldMaxSize; i++)
   {
      if (oldElts[i].state == kHES_InUse)
      {
         insert(oldElts[i].first, oldElts[i].second);
      }
      m_allocator.destroy(&oldElts[i]);
   }

   Assert(m_size == oldSize);

   m_allocator.deallocate(oldElts, oldMaxSize);
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
void HASHTABLE_TEMPLATE_CLASS::collect_stats(sHashTableStats * pStats) const
{
   if (pStats == NULL)
   {
      return;
   }

   uint prevRawHash = m_maxSize, chainStart = m_maxSize;
   uint chainLengthSum = 0, minChainLength = m_maxSize, maxChainLength = 0;
   size_type nEmpty = 0, nInUse = 0, nErased = 0, nChains = 0;
   for (size_type i = 0; i < m_maxSize; i++)
   {
      uint newChainStart = m_maxSize;
      bool bEndChain = false;
      switch (m_elts[i].state)
      {
         case kHES_Empty:
         {
            nEmpty++;
            // Chain ends on empty cell
            if (chainStart != m_maxSize)
            {
               bEndChain = true;
            }
            prevRawHash = m_maxSize;
            break;
         }
         case kHES_InUse:
         {
            nInUse++;
            uint rawHash = HASHFN::Hash(m_elts[i].first) & (m_maxSize - 1);
            if (prevRawHash != m_maxSize)
            {
               // Chain ends on differing raw hash
               if (rawHash != prevRawHash)
               {
                  bEndChain = true;
                  // This cell is in use so is also a potential chain starter
                  newChainStart = i;
               }
            }
            else
            {
               // Potential chain starter because last cell was empty or erased
               chainStart = i;
            }
            prevRawHash = rawHash;
            break;
         }
         case kHES_Erased:
         {
            nErased++;
            // Erased entries don't break a chain (are ignored in search)
            break;
         }
      }

      if (bEndChain)
      {
         uint chainLength = i - chainStart;
         if (chainLength > 1)
         {
            if (chainLength > maxChainLength)
            {
               maxChainLength = chainLength;
            }
            if (chainLength < minChainLength)
            {
               minChainLength = chainLength;
            }
            chainLengthSum += chainLength;
            nChains++;
         }
         chainStart = newChainStart;
      }
   }

   pStats->load = (size() * 255) / max_size();
   pStats->nEmpty = nEmpty;
   pStats->nInUse = nInUse;
   pStats->nErased = nErased;
   pStats->nChains = nChains;
   pStats->minChain = minChainLength;
   pStats->maxChain = maxChainLength;
   pStats->avgChain = static_cast<float>(chainLengthSum) / nChains;
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
std::pair<HASHTABLE_TEMPLATE_MEMBER_TYPE(const_iterator), bool>
HASHTABLE_TEMPLATE_CLASS::insert(const KEY & k, const VALUE & v)
{
   return Insert(k, v, false);
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_MEMBER_TYPE(tMutableHashElementProxy) HASHTABLE_TEMPLATE_CLASS::operator [](const KEY & k)
{
   return tMutableHashElementProxy(this, k);
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
const VALUE & HASHTABLE_TEMPLATE_CLASS::operator [](const KEY & k) const
{
   const_iterator iter = find(k);
   Assert(iter != m_pHashTable->end());
   return iter->second;
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
HASHTABLE_TEMPLATE_MEMBER_TYPE(iterator) HASHTABLE_TEMPLATE_CLASS::find(const KEY & k)
{
   uint h = Probe(k, true);
   if (m_elts[h].state == kHES_InUse)
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
   if (m_elts[h].state == kHES_InUse)
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
   uint h = Probe(k, true);

   if (m_elts[h].state != kHES_InUse)
   {
      return 0;
   }

   m_allocator.destroy(&m_elts[h]);
   m_elts[h].state = kHES_Erased;
   m_size--;

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

HASHTABLE_TEMPLATE_DECL
std::pair<HASHTABLE_TEMPLATE_MEMBER_TYPE(const_iterator), bool>
HASHTABLE_TEMPLATE_CLASS::Insert(const KEY & k, const VALUE & v, bool bOverwriteExisting)
{
   if ((m_size * 255) > (m_maxSize * m_loadFactor))
   {
      // grow in proportion to fullness
      reserve(m_maxSize + (m_size * 255 / m_loadFactor));
   }

   uint h = Probe(k, false);
   if ((m_elts[h].state == kHES_InUse) && !bOverwriteExisting)
   {
      return std::make_pair(const_iterator(&m_elts[h], &m_elts[0], &m_elts[m_maxSize]), false);
   }

   m_elts[h].first = k;
   m_elts[h].second = v;
   if (m_elts[h].state != kHES_InUse)
   {
      m_elts[h].state = kHES_InUse;
      m_size++;
   }
   return std::make_pair(const_iterator(&m_elts[h], &m_elts[0], &m_elts[m_maxSize]), true);
}

////////////////////////////////////////
// Probe computes the hash for a key and resolves collisions

HASHTABLE_TEMPLATE_DECL
uint HASHTABLE_TEMPLATE_CLASS::Probe(const KEY & k, bool bSkipErased) const
{
   Assert(IsPowerOfTwo(m_maxSize));
   uint h = HASHFN::Hash(k) & (m_maxSize - 1);

#ifdef _DEBUG
   uint start = h;
   bool wrapped = false;
#endif

   // resolve collisions with linear probing
   while (((m_elts[h].state == kHES_InUse) && !Equal(k, m_elts[h].first))
      || (bSkipErased && (m_elts[h].state == kHES_Erased)))
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
bool HASHTABLE_TEMPLATE_CLASS::Equal(const KEY & k1, const KEY & k2) const
{
   return HASHFN::Equal(k1, k2);
}

////////////////////////////////////////

HASHTABLE_TEMPLATE_DECL
void HASHTABLE_TEMPLATE_CLASS::Reset(size_type newInitialSize)
{
   for (size_type i = 0; i < m_maxSize; i++)
   {
      m_allocator.destroy(&m_elts[i]);
   }

   m_allocator.deallocate(m_elts, m_maxSize);
   m_elts = NULL;
   m_maxSize = 0;
   m_size = 0;

   if (newInitialSize > 0)
   {
      reserve(newInitialSize);
   }
}

////////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_HASHTABLETEM_H
