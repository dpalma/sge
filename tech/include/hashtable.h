///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_HASHTABLE_H
#define INCLUDED_HASHTABLE_H

#include <memory>

#ifdef _MSC_VER
#pragma once
#endif

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

////////////////////////////////////////

template <typename KEY, typename VALUE>
sHashElement<KEY, VALUE>::sHashElement()
 : inUse(false)
{
}

////////////////////////////////////////

template <typename KEY, typename VALUE,
          typename HASHFN = cHashFunction<KEY>,
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

   cHashTable(uint initialSize = kInitialSizeSmall);
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

   void Reset(uint newInitialSize = kInitialSizeSmall);

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

#endif // !INCLUDED_HASHTABLE_H
