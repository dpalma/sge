///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCRIPTVAR_H
#define INCLUDED_SCRIPTVAR_H

#include "enginedll.h"

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
};

struct sScriptVar
{
   eScriptVarType type;
   union
   {
      double d;
      char * psz;
   };
};

class ENGINE_API cScriptVar : public sScriptVar
{
public:
   cScriptVar();
   cScriptVar(double _d);
   cScriptVar(char * _psz);
   cScriptVar(const cScriptVar & other);

   const cScriptVar & operator =(double _d);
   const cScriptVar & operator =(char * _psz);
   const cScriptVar & operator =(const cScriptVar & other);

   operator double() const;
   operator const char *() const;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCRIPTVAR_H
