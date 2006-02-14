///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_HASHTABLE_H
#define INCLUDED_HASHTABLE_H

#include <memory>
#include <utility>

#ifdef _MSC_VER
#pragma once
#endif

// REFERENCES
// http://uk.builder.com/programming/c/0,39029981,20266440,00.htm


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cHashFunction
//

template <typename T>
class cHashFunction
{
public:
   static uint Hash(const T & a, uint initHash = 0xDEADBEEF)
   {
      Assert(!"Cannot use default cHashFunction<>::Hash()!");
      return ~0;
   }

   static bool Equal(const T & a, const T & b)
   {
      Assert(!"Cannot use default cHashFunction<>::Equal()!");
      return false;
   }
};


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: sHashElement
//

enum eHashElementState
{
   kHES_Empty,
   kHES_InUse,
   kHES_Erased,
};

template <typename KEY, typename VALUE>
struct sHashElement : public std::pair<KEY, VALUE>
{
   sHashElement() : state(kHES_Empty) {}
   byte state;
};


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cHashIterator
//

template <typename HASHELEMENT>
class cHashIterator
{
public:
   typedef std::bidirectional_iterator_tag iterator_category;
   typedef HASHELEMENT element_type;
   typedef HASHELEMENT & reference;
   typedef HASHELEMENT * pointer;
   typedef ptrdiff_t difference_type;

   cHashIterator();
   cHashIterator(const cHashIterator & other);
   cHashIterator(HASHELEMENT * pElement, HASHELEMENT * pBegin, HASHELEMENT * pEnd);

   const cHashIterator & operator =(const cHashIterator & other);

   template <typename HASHELEMENTOTHER> friend class cHashIterator;
   template <typename HASHELEMENTOTHER>
   cHashIterator(const cHashIterator<HASHELEMENTOTHER> & other)
    : m_pElement(const_cast<HASHELEMENT*>(other.m_pElement))
    , m_pBegin(const_cast<HASHELEMENT*>(other.m_pBegin))
    , m_pEnd(const_cast<HASHELEMENT*>(other.m_pEnd))
   {
   }

   pointer operator ->() const;
   reference operator *() const;

   const cHashIterator & operator ++(); // preincrement
   const cHashIterator operator ++(int); // postincrement

   const cHashIterator & operator --(); // predecrement
   const cHashIterator operator --(int); // postdecrement

   bool operator ==(const cHashIterator & other) const;
   bool operator !=(const cHashIterator & other) const;

   template <typename HASHELEMENTOTHER>
   bool operator ==(const cHashIterator<HASHELEMENTOTHER> & other) const
   {
      return (m_pElement == other.m_pElement);
   }
   template <typename HASHELEMENTOTHER>
   bool operator !=(const cHashIterator<HASHELEMENTOTHER> & other) const
   {
      return !(*this == other);
   }

private:
   HASHELEMENT * m_pElement;
   HASHELEMENT * m_pBegin;
   HASHELEMENT * m_pEnd;
};


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cHashTable
//

template <typename KEY, typename VALUE,
          typename HASHFN = cHashFunction<KEY>,
          class ALLOCATOR = std::allocator< sHashElement<KEY, VALUE> > >
class cHashTable
{
public:
   typedef struct sHashElement<KEY, VALUE> element_type;
   typedef element_type & reference;
   typedef const element_type & const_reference;
   typedef cHashIterator<element_type> iterator;
   typedef cHashIterator<const element_type> const_iterator;
   typedef typename ALLOCATOR::template rebind<element_type>::other allocator_type;
   typedef typename allocator_type::size_type size_type;

   enum
   {
      kInitialSizeSmall = 16,
      kInitialSizeMed   = 256,
      kInitialSizeLarge = 1024,
      kFullnessThreshold = 70, // hash table only ever allowed to get 70% full
   };

   cHashTable(size_type initialSize = kInitialSizeSmall);
   explicit cHashTable(const cHashTable & other);
   ~cHashTable();

   allocator_type get_allocator() const;

   void reserve(size_type capacity);

   std::pair<const_iterator, bool> insert(const KEY & k, const VALUE & v);

   inline std::pair<const_iterator, bool> insert(const std::pair<KEY, VALUE> & p)
   {
      return insert(p.first, p.second);
   }

   VALUE & operator [](const KEY & k);

   iterator find(const KEY & k);
   const_iterator find(const KEY & k) const;

   size_type erase(const KEY & k);
   void clear();

   bool empty() const;
   size_type size() const;
   size_type max_size() const;

   iterator begin();
   iterator end();

   const_iterator begin() const;
   const_iterator end() const;

private:
   uint Probe(const KEY & k, bool bSkipErased) const;
   bool Equal(const KEY & k1, const KEY & k2) const;

   void Reset(size_type newInitialSize = kInitialSizeSmall);

   allocator_type m_allocator;

   element_type * m_elts;
   size_type m_maxSize;
   size_type m_size;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_HASHTABLE_H
