///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_HASH_H
#define INCLUDED_HASH_H

#include "techdll.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;   /* unsigned 1-byte quantities */

ub4 hash(register ub1 * k, register ub4 length, register ub4 initval);

inline uint Hash(const void * key, int size) { return hash((ub1*)key, size, 0xDEADBEEF); }
inline uint Hash(const void * ptr) { return hash((ub1*)ptr, sizeof(ptr), 0xDEADBEEF); }

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_HASH_H
