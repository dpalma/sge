///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_HASHTABLE_H
#define INCLUDED_HASHTABLE_H

#include <memory>
#include <xutility>

#ifdef _MSC_VER
#pragma once
#endif

// REFERENCES
// http://uk.builder.com/programming/c/0,39029981,20266440,00.htm#


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

template <typename KEY, typename VALUE>
struct sHashElement
{
	typedef KEY key_type;
	typedef VALUE value_type;
   sHashElement() : inUse(false) {}
   KEY key;
   VALUE value;
   bool inUse;
};


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cHashConstIterator
//

template <class HASHELEMENT>
class cHashConstIterator
{
   typedef std::pair<HASHELEMENT::key_type, HASHELEMENT::value_type> tPair;

public:
   typedef std::forward_iterator_tag iterator_category;
   typedef tPair value_type;
   typedef const tPair & const_reference;
   typedef const tPair * const_pointer;
   typedef int difference_type;

   cHashConstIterator();
   cHashConstIterator(const cHashConstIterator & other);
   cHashConstIterator(HASHELEMENT * pElement, const HASHELEMENT * pEnd);

   const cHashConstIterator operator =(const cHashConstIterator & other);

   const_pointer operator ->() const;
   const_reference operator *() const;

   const cHashConstIterator & operator ++(); // preincrement
   const cHashConstIterator operator ++(int); // postincrement

   bool operator ==(const cHashConstIterator & other) const;
   bool operator !=(const cHashConstIterator & other) const;

private:
   HASHELEMENT * m_pElement;
   const HASHELEMENT * m_pEnd;
   tPair m_pair;
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
   typedef size_t size_type;
   typedef struct sHashElement<KEY, VALUE> value_type;
   typedef struct sHashElement<KEY, VALUE> & reference;
   typedef const struct sHashElement<KEY, VALUE> & const_reference;
   typedef cHashConstIterator< sHashElement<KEY, VALUE> > const_iterator;

   enum
   {
      kInitialSizeSmall = 16,
      kInitialSizeMed   = 256,
      kInitialSizeLarge = 1024,
   };

   cHashTable(uint initialSize = kInitialSizeSmall);
   ~cHashTable();

   std::pair<const_iterator, bool> insert(const KEY & k, const VALUE & v);

   bool Lookup(const KEY & k, VALUE * v) const;

   size_type erase(const KEY & k);
   void clear();

   bool empty() const;
   size_type size() const;
   size_type max_size() const;

   const_iterator begin() const;
   const_iterator end() const;

private:
   uint Probe(const KEY & k) const;
   void Grow(uint newSize);
   bool Equal(const KEY & k1, const KEY & k2) const;

   void Reset(uint newInitialSize = kInitialSizeSmall);

   ALLOCATOR m_allocator;

   value_type * m_elts;
   uint m_maxSize;
   uint m_size;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_HASHTABLE_H
