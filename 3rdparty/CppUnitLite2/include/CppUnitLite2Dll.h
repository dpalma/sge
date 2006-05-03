///////////////////////////////////////////////////////////////////////////////
// $Id: CppUnitLite2Dll.h,v 1.1 2006/02/28 15:41:13 dpalma Exp $

// The following ifdef block is the standard way of creating macros which make
// exporting from a DLL simpler. All files within this DLL are compiled with
// the CPPUNITLITE2_EXPORTS symbol defined on the command line. this symbol should not
// be defined on any project that uses this DLL. This way any other project
// whose source files include this file see CPPUNITLITE2_API functions as being
// imported from a DLL, whereas this DLL sees symbols defined with this macro
// as being exported.
#ifdef NO_AUTO_EXPORTS
#define CPPUNITLITE2_API
#else
#ifdef CPPUNITLITE2_EXPORTS
#define CPPUNITLITE2_API __declspec(dllexport)
#else
#define CPPUNITLITE2_API __declspec(dllimport)
#endif
#endif
