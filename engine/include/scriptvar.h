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

   void Assign(int value);
   void Assign(uint value);
   void Assign(float value);
   void Assign(double value);

   int ToInt() const;
   uint ToUint() const;
   float ToFloat() const;
   double ToDouble() const;

   const cScriptVar & operator =(int value);
   const cScriptVar & operator =(uint value);
   const cScriptVar & operator =(float value);
   const cScriptVar & operator =(double value);
   const cScriptVar & operator =(char * _psz);
   const cScriptVar & operator =(IUnknown * _pUnk);
   const cScriptVar & operator =(const cScriptVar & other);

   operator int() const;
   operator uint() const;
   operator float() const;
   operator double() const;
   operator const char *() const;
   operator IUnknown *() const;

   bool IsNumber() const;
   bool IsString() const;

private:
   void Clear();
};

///////////////////////////////////////

inline const cScriptVar & cScriptVar::operator =(int value)
{
   Assign(value);
   return *this;
}

///////////////////////////////////////

inline const cScriptVar & cScriptVar::operator =(uint value)
{
   Assign(value);
   return *this;
}

///////////////////////////////////////

inline const cScriptVar & cScriptVar::operator =(float value)
{
   Assign(value);
   return *this;
}

///////////////////////////////////////

inline const cScriptVar & cScriptVar::operator =(double value)
{
   Assign(value);
   return *this;
}

///////////////////////////////////////

inline cScriptVar::operator int() const
{
   return ToInt();
}

///////////////////////////////////////

inline cScriptVar::operator uint() const
{
   return ToUint();
}

///////////////////////////////////////

inline cScriptVar::operator float() const
{
   return ToFloat();
}

///////////////////////////////////////

inline cScriptVar::operator double() const
{
   return ToDouble();
}

///////////////////////////////////////

inline bool cScriptVar::IsNumber() const
{
   return (type == kNumber);
}

///////////////////////////////////////

inline bool cScriptVar::IsString() const
{
   return (type == kString);
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCRIPTVAR_H
