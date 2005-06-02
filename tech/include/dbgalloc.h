///////////////////////////////////////////////////////////////////////////////
// $Id$

#if defined(_WIN32) && !defined(_WIN32_WCE) && !defined(__GNUC__)

#include <crtdbg.h>

///////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
   #define DBGALLOC_MAPPED

   #define malloc(s)          _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
   #define calloc(c, s)       _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
   #define realloc(p, s)      _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
   #define _expand(p, s)      _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
   #define free(p)            _free_dbg(p, _NORMAL_BLOCK)
   #define _msize(p)          _msize_dbg(p, _NORMAL_BLOCK)

   #ifdef __cplusplus
      #undef new
      #define DebugNew new(_NORMAL_BLOCK, __FILE__, __LINE__)
      #define new DebugNew
   #endif

#endif

#endif // defined(_WIN32) && !defined(_WIN32_WCE)

///////////////////////////////////////////////////////////////////////////////
