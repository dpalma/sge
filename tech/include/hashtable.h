///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_HASHTBL_H
#define INCLUDED_HASHTBL_H

#include <memory>

#ifdef _MSC_VER
#pragma once
#endif

#ifndef HASHTBLTEST
#error ("Do not use cHashTable! Use a STL container instead!")
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cHashTable
//

template <typename KEY, typename VALUE>
struct sHashElement
{
   sHashElement();

   KEY key;
   VALUE value;
   bool inUse;
};

template <typename KEY, typename VALUE>
sHashElement<KEY, VALUE>::sHashElement()
 : inUse(false)
{
}

template <typename KEY, typename VALUE,
          class ALLOCATOR = std::allocator< sHashElement<KEY, VALUE> > >
class cHashTable
{
public:
   enum
   {
      kInitialSizeSmall = 16,
      kInitialSizeMed   = 256,
      kInitialSizeLarge = 1024,
   };

   cHashTable(int initialSize = kInitialSizeSmall);
   ~cHashTable();

   void Clear();

   bool Set(const KEY & k, const VALUE & v);
   bool Insert(const KEY & k, const VALUE & v);
   bool Lookup(const KEY & k, VALUE * v) const;
   bool Delete(const KEY & k);

   void IterBegin(HANDLE * phIter) const;
   bool IterNext(HANDLE * phIter, KEY * pk, VALUE * pv) const;
   void IterEnd(HANDLE * phIter) const;

private:
   uint Probe(const KEY & k) const;
   void Grow(uint newSize);
   bool Equal(const KEY & k1, const KEY & k2) const;

   void Reset(int newInitialSize = kInitialSizeSmall);

   typedef struct sHashElement<KEY, VALUE> tHashElement;

   ALLOCATOR m_allocator;

   tHashElement * m_elts;
   uint m_size;
   uint m_count;
#ifndef NDEBUG
   mutable ulong m_nItersActive;
#endif
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_HASHTBL_H
