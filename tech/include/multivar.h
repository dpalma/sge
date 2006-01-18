///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MULTIVAR_H
#define INCLUDED_MULTIVAR_H

#include "techdll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMultiVar
//

enum eMultiVarType
{
   kMVT_Empty,
   kMVT_Int,
   kMVT_Float,
   kMVT_Double,
   kMVT_String,
   kMVT_Interface,
};

class TECH_API cMultiVar
{
public:
   cMultiVar();
   cMultiVar(const cMultiVar & other);
   explicit cMultiVar(int i);
   explicit cMultiVar(float f);
   explicit cMultiVar(double d);
   explicit cMultiVar(const char * psz);
   explicit cMultiVar(const wchar_t * pwsz);
   explicit cMultiVar(IUnknown * pUnk);
   ~cMultiVar();

   void Assign(const cMultiVar & multiVar);
   void Assign(int value);
   void Assign(float value);
   void Assign(double value);
   void Assign(const char * pszValue);
   void Assign(const wchar_t * pwszValue);
   void Assign(IUnknown * pUnk);

   int ToInt() const;
   float ToFloat() const;
   double ToDouble() const;
   const char * ToString() const;
   const wchar_t * ToWideString() const;

   const cMultiVar & operator =(const cMultiVar & other);
   const cMultiVar & operator =(int value);
   const cMultiVar & operator =(float value);
   const cMultiVar & operator =(double value);
   const cMultiVar & operator =(const char * psz);
   const cMultiVar & operator =(const wchar_t * pwsz);
   const cMultiVar & operator =(IUnknown * pUnk);

   operator int() const;
   operator float() const;
   operator double() const;
   operator const char *() const;
   operator const wchar_t *() const;
   operator IUnknown *() const;

   bool IsEmpty() const;
   bool IsInt() const;
   bool IsFloat() const;
   bool IsDouble() const;
   bool IsString() const;
   bool IsInterface() const;

   eMultiVarType GetType() const;

   void Clear();

private:
   void * m_pTempBuffer; // for temp storage of native type
   mutable void * m_pConversionBuffer; // for temp storage of cross-type conversion

   eMultiVarType m_type;

   union
   {
      int i;
      float f;
      double d;
      char * psz;
      IUnknown * pUnk;
   } m_value;
};

////////////////////////////////////////

inline const cMultiVar & cMultiVar::operator =(const cMultiVar & other)
{
   Assign(other);
   return *this;
}

///////////////////////////////////////

inline const cMultiVar & cMultiVar::operator =(int value)
{
   Assign(value);
   return *this;
}

///////////////////////////////////////

inline const cMultiVar & cMultiVar::operator =(float value)
{
   Assign(value);
   return *this;
}

///////////////////////////////////////

inline const cMultiVar & cMultiVar::operator =(double value)
{
   Assign(value);
   return *this;
}

///////////////////////////////////////

inline const cMultiVar & cMultiVar::operator =(const char * psz)
{
   Assign(psz);
   return *this;
}

///////////////////////////////////////

inline const cMultiVar & cMultiVar::operator =(const wchar_t * pwsz)
{
   Assign(pwsz);
   return *this;
}

////////////////////////////////////////

inline const cMultiVar & cMultiVar::operator =(IUnknown * pUnk)
{
   Assign(pUnk);
   return *this;
}

///////////////////////////////////////

inline cMultiVar::operator int() const
{
   return ToInt();
}

///////////////////////////////////////

inline cMultiVar::operator float() const
{
   return ToFloat();
}

///////////////////////////////////////

inline cMultiVar::operator double() const
{
   return ToDouble();
}

////////////////////////////////////////

inline cMultiVar::operator const char *() const
{
   return ToString();
}

////////////////////////////////////////

inline cMultiVar::operator const wchar_t *() const
{
   return ToWideString();
}

///////////////////////////////////////

inline bool cMultiVar::IsEmpty() const
{
   return (m_type == kMVT_Empty);
}

///////////////////////////////////////

inline bool cMultiVar::IsInt() const
{
   return (m_type == kMVT_Int);
}

///////////////////////////////////////

inline bool cMultiVar::IsFloat() const
{
   return (m_type == kMVT_Float);
}

///////////////////////////////////////

inline bool cMultiVar::IsDouble() const
{
   return (m_type == kMVT_Double);
}

///////////////////////////////////////

inline bool cMultiVar::IsString() const
{
   return (m_type == kMVT_String);
}

///////////////////////////////////////

inline bool cMultiVar::IsInterface() const
{
   return (m_type == kMVT_Interface);
}

///////////////////////////////////////

inline eMultiVarType cMultiVar::GetType() const
{
   return m_type;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MULTIVAR_H
