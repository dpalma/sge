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
   kNil, // Lua-specific; used as a return value because Lua's nil means false, too
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
   // private constructor used only to initialize statics (like Nil)
   cScriptVar(eScriptVarType type);

public:
   static cScriptVar Nil; // Lua-specific; see comments for kNil above

   cScriptVar();
   cScriptVar(double _d);
   cScriptVar(char * _psz);
   cScriptVar(wchar_t * _pwsz);
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
   const cScriptVar & operator =(wchar_t * _pwsz);
   const cScriptVar & operator =(IUnknown * _pUnk);
   const cScriptVar & operator =(const cScriptVar & other);

   operator int() const;
   operator uint() const;
   operator float() const;
   operator double() const;
   operator const char *() const;
   operator const wchar_t *() const;
   operator IUnknown *() const;

   bool IsNumber() const;
   bool IsString() const;
   bool IsInterface() const;
   bool IsNil() const;

private:
   void Clear();

   void * m_pTempBuffer;
   mutable void * m_pConversionBuffer;
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

///////////////////////////////////////

inline bool cScriptVar::IsInterface() const
{
   return (type == kInterface);
}

///////////////////////////////////////

inline bool cScriptVar::IsNil() const
{
   return (type == kNil);
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCRIPTVAR_H
