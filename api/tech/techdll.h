///////////////////////////////////////////////////////////////////////////////
// $Id$

// The following ifdef block is the standard way of creating macros which make
// exporting from a DLL simpler. All files within this DLL are compiled with
// the TECH_EXPORTS symbol defined on the command line. this symbol should not
// be defined on any project that uses this DLL. This way any other project
// whose source files include this file see TECH_API functions as being
// imported from a DLL, whereas this DLL sees symbols defined with this macro
// as being exported.
#ifdef NO_AUTO_EXPORTS
#define TECH_API
#else
#ifdef TECH_EXPORTS
#define TECH_API __declspec(dllexport)
#else
#define TECH_API __declspec(dllimport)
#endif
#endif
