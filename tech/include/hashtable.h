///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_HASHTBL_H
#define INCLUDED_HASHTBL_H

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
class cHashTable
{
public:
   enum
   {
      kInitialSizeSmall = 10,
      kInitialSizeMed   = 200,
      kInitialSizeLarge = 1000,
   };

   cHashTable(int initialSize = kInitialSizeSmall);
   ~cHashTable();

   void Clear();
   void Reset(int newInitialSize = kInitialSizeSmall);

   bool Set(const KEY & k, const VALUE & v);
   bool Insert(const KEY & k, const VALUE & v);
   bool Lookup(const KEY & k, VALUE * v) const;
   bool Delete(const KEY & k);

   void IterBegin(HANDLE * phIter) const;
   bool IterNext(HANDLE * phIter, KEY * pk, VALUE * pv) const;
   void IterEnd(HANDLE * phIter) const;

private:
   uint Probe(const KEY & k) const;
   bool Grow(int newSize);
   bool Equal(const KEY & k1, const KEY & k2) const;

   struct sHashElement
   {
      KEY key;
      VALUE value;
   };

   uint8 * m_eltFull; // interpreted as bools
   sHashElement * m_elts;
   uint m_size;
   uint m_count;
#ifndef NDEBUG
   mutable ulong m_cItersActive;
#endif
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_HASHTBL_H
