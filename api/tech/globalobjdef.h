///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GLOBALOBJDEF_H
#define INCLUDED_GLOBALOBJDEF_H

#include "techdll.h"
#include "comtools.h"
#include "globalobj.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IGlobalObject);

///////////////////////////////////////////////////////////////////////////////

tResult RegisterGlobalObject(REFGUID iid, IUnknown * pUnk);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cBeforeAfterConstraint
//

enum eBeforeAfter
{
   kBefore,
   kAfter
};

class TECH_API cBeforeAfterConstraint
{
public:
   cBeforeAfterConstraint(const GUID * pGuid, eBeforeAfter beforeAfter);
   cBeforeAfterConstraint(const tChar * pszName, eBeforeAfter beforeAfter);
   cBeforeAfterConstraint(const cBeforeAfterConstraint & other);
   ~cBeforeAfterConstraint();

   const cBeforeAfterConstraint & operator =(const cBeforeAfterConstraint & other);

   const GUID * GetGuid() const { return m_pGuid; }
   const tChar * GetName() const { return m_pszName; }
   bool Before() const { return m_beforeAfter == kBefore; }
   bool After() const { return m_beforeAfter == kAfter; }

private:
   const GUID * m_pGuid;
   const tChar * m_pszName;
   eBeforeAfter m_beforeAfter;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGlobalObject
//

interface IGlobalObject : IUnknown
{
   virtual tResult Init() = 0;
   virtual tResult Term() = 0;

   virtual const tChar * GetName() const = 0;
   virtual tResult GetConstraints(const cBeforeAfterConstraint * * ppConstraints, size_t * pnConstraints) const = 0;
};

///////////////////////////////////////

#define DECLARE_NAME(name) \
   virtual const tChar * GetName() const \
   { \
      return _T(#name); \
   }

///////////////////////////////////////

#define DECLARE_NAME_STRING(szName) \
   virtual const tChar * GetName() const \
   { \
      return szName; \
   }

///////////////////////////////////////

#define DECLARE_NO_CONSTRAINTS() \
   virtual tResult GetConstraints(const cBeforeAfterConstraint * * ppConstraints, size_t * pnConstraints) const \
   { \
      return E_NOTIMPL; \
   }

///////////////////////////////////////

#define DECLARE_CONSTRAINTS() \
   virtual tResult GetConstraints(const cBeforeAfterConstraint * * ppConstraints, size_t * pnConstraints) const;

///////////////////////////////////////

#define BEGIN_CONSTRAINTS(GlobalObjectClass) \
   tResult GlobalObjectClass::GetConstraints(const cBeforeAfterConstraint * * ppConstraints, size_t * pnConstraints) const \
   { \
      static cBeforeAfterConstraint constraints[] = { \

///////////////////////////////////////

#define AFTER_GUID(guid) \
   cBeforeAfterConstraint(&guid, kAfter),

#define BEFORE_GUID(guid) \
   cBeforeAfterConstraint(&guid, kBefore),

#define AFTER_NAME(name) \
   cBeforeAfterConstraint(#name, kAfter),

#define BEFORE_NAME(name) \
   cBeforeAfterConstraint(#name, kBefore),

///////////////////////////////////////

#define END_CONSTRAINTS() \
      }; \
      if (ppConstraints == NULL || pnConstraints == NULL) \
         return E_POINTER; \
      *ppConstraints = &constraints[0]; \
      *pnConstraints = _countof(constraints); \
      return S_OK; \
   }

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GLOBALOBJDEF_H
