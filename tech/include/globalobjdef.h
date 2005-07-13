///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GLOBALOBJDEF_H
#define INCLUDED_GLOBALOBJDEF_H

#include "techdll.h"
#include "comtools.h"
#include "globalobj.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IGlobalObject);

///////////////////////////////////////////////////////////////////////////////

TECH_API tResult RegisterGlobalObject(REFGUID iid, IUnknown * pUnk);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGlobalObject
//

///////////////////////////////////////

enum eBeforeAfter
{
   kBefore,
   kAfter
};

class TECH_API cGlobalObjectInitConstraint
{
public:
   cGlobalObjectInitConstraint(const GUID * pGuid, eBeforeAfter beforeAfter);
   cGlobalObjectInitConstraint(const tChar * pszName, eBeforeAfter beforeAfter);
   cGlobalObjectInitConstraint(const cGlobalObjectInitConstraint & other);
   ~cGlobalObjectInitConstraint();

   const cGlobalObjectInitConstraint & operator =(const cGlobalObjectInitConstraint & other);

   const GUID * GetGuid() const { return m_pGuid; }
   const tChar * GetName() const { return m_pszName; }
   bool InitBefore() const { return m_beforeAfter == kBefore; }
   bool InitAfter() const { return m_beforeAfter == kAfter; }

private:
   const GUID * m_pGuid;
   const tChar * m_pszName;
   eBeforeAfter m_beforeAfter;
};

typedef std::vector<cGlobalObjectInitConstraint> tGlobalObjectInitConstraints;

///////////////////////////////////////

interface IGlobalObject : IUnknown
{
   virtual tResult Init() = 0;
   virtual tResult Term() = 0;

   virtual const tChar * GetName() const = 0;
   virtual size_t GetConstraints(tGlobalObjectInitConstraints * pConstraints) const = 0;
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
   virtual size_t GetConstraints(tGlobalObjectInitConstraints *) const \
   { \
      return 0; \
   }

///////////////////////////////////////

#define DECLARE_CONSTRAINTS() \
   virtual size_t GetConstraints(tGlobalObjectInitConstraints * pConstraints) const;

///////////////////////////////////////

#define BEGIN_CONSTRAINTS(GlobalObjectClass) \
   size_t GlobalObjectClass::GetConstraints(tGlobalObjectInitConstraints * pConstraints) const \
   { \
      static cGlobalObjectInitConstraint constraints[] = { \

///////////////////////////////////////

#define AFTER_GUID(guid) \
   cGlobalObjectInitConstraint(&guid, kAfter),

#define BEFORE_GUID(guid) \
   cGlobalObjectInitConstraint(&guid, kBefore),

#define AFTER_NAME(name) \
   cGlobalObjectInitConstraint(#name, kAfter),

#define BEFORE_NAME(name) \
   cGlobalObjectInitConstraint(#name, kBefore),

///////////////////////////////////////

#define END_CONSTRAINTS() \
      }; \
      if (pConstraints != NULL) \
         for (int i = 0; i < _countof(constraints); i++) \
            pConstraints->push_back(constraints[i]); \
      return _countof(constraints); \
   }

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GLOBALOBJDEF_H
