///////////////////////////////////////////////////////////////////////////////
// $Id$

///////////////////////////////////////////////////////////////////////////////

#ifdef DBGALLOC_MAPPED
   #undef malloc
   #undef calloc
   #undef realloc
   #undef _expand
   #undef free
   #undef _msize
   #undef new
   #undef DebugNew
#endif

///////////////////////////////////////////////////////////////////////////////
