///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_LUAINTERP_H
#define INCLUDED_LUAINTERP_H

#include "scriptapi.h"
#include "globalobjdef.h"

#include <cstdarg>

#ifdef _MSC_VER
#pragma once
#endif

typedef struct lua_State lua_State;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLuaInterpreter
//

class cLuaInterpreter : public cComObject2<IMPLEMENTS(IScriptInterpreter), IMPLEMENTS(IGlobalObject)>
{
   friend tResult ScriptAddFunction(const char * pszName, tScriptFn pfn);

public:
   cLuaInterpreter();
   ~cLuaInterpreter();

   DECLARE_NAME(LuaInterpreter)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult ExecFile(const tChar * pszFile);
   virtual tResult ExecString(const char * pszCode);
   virtual tResult CDECL CallFunction(const char * pszName, const char * pszArgDesc, ...);
   tResult CallFunction(const char * pszName, const char * pszArgDesc, va_list args);

   virtual tResult RegisterCustomClass(const tChar * pszClassName, IScriptableFactory * pFactory);
   virtual tResult RevokeCustomClass(const tChar * pszClassName);

   virtual tResult AddNamedItem(const char * pszName, double value);
   virtual tResult AddNamedItem(const char * pszName, const char * pszValue);
   virtual tResult AddNamedItem(const char * pszName, tScriptFn pfn);
   virtual tResult AddNamedItem(const char * pszName, IScriptable * pObject);
   virtual tResult RemoveNamedItem(const char * pszName);
   virtual tResult GetNamedItem(const char * pszName, cScriptVar * pValue) const;
   virtual tResult GetNamedItem(const char * pszName, double * pValue) const;
   virtual tResult GetNamedItem(const char * pszName, char * pValue, int cbMaxValue) const;

private:
   static void CleanupPreRegisteredFunctions();

   class cAutoCleanupPreRegisteredFunctions
   {
   public:
      ~cAutoCleanupPreRegisteredFunctions();
   };
   friend class cAutoCleanupPreRegisteredFunctions;
   static cAutoCleanupPreRegisteredFunctions g_autoCleanupPreRegisteredFunctions;

   static bool gm_bInitialized;

   struct sPreRegisteredFunction
   {
      char szName[100];
      tScriptFn pfn;
      struct sPreRegisteredFunction * pNext;
   };

   static struct sPreRegisteredFunction * gm_pPreRegisteredFunctions;

   lua_State * m_L;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_LUAINTERP_H
