///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCRIPTVAR_H
#define INCLUDED_SCRIPTVAR_H

#include "enginedll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cScriptVar
//

enum eScriptVarType
{
   kEmpty,
   kNumber,
   kString,
   kInterface,
};

struct sScriptVar
{
   eScriptVarType type;
   union
   {
      double d;
      char * psz;
      IUnknown * pUnk;
   };
};

class ENGINE_API cScriptVar : public sScriptVar
{
public:
   cScriptVar();
   cScriptVar(double _d);
   cScriptVar(char * _psz);
   cScriptVar(IUnknown * _pUnk);
   cScriptVar(const cScriptVar & other);
   ~cScriptVar();

   const cScriptVar & operator =(double _d);
   const cScriptVar & operator =(char * _psz);
   const cScriptVar & operator =(IUnknown * _pUnk);
   const cScriptVar & operator =(const cScriptVar & other);

   operator double() const;
   operator const char *() const;
   operator IUnknown *() const;

private:
   void Clear();
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCRIPTVAR_H
