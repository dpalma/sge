///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GLOBALOBJ_H
#define INCLUDED_GLOBALOBJ_H

#include "techdll.h"
#include "comtools.h"
#include "constraints.h"
#include "globalobjreg.h"

#include <vector>

#include "dbgalloc.h" // must be last header

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IGlobalObject);

///////////////////////////////////////////////////////////////////////////////

TECH_API IGlobalObjectRegistry * AccessGlobalObjectRegistry();

inline tResult StartGlobalObjects()
{
   return AccessGlobalObjectRegistry()->InitAll();
}

inline tResult StopGlobalObjects()
{
   return AccessGlobalObjectRegistry()->TermAll();
}

inline IUnknown * FindGlobalObject(REFIID iid)
{
   return AccessGlobalObjectRegistry()->Lookup(iid);
}

#define UseGlobal_(ObjBaseName, VarName) \
   cAutoIPtr<I##ObjBaseName> VarName((I##ObjBaseName*)FindGlobalObject(IID_I##ObjBaseName))

#define UseGlobal(ObjBaseName) \
   UseGlobal_(ObjBaseName, p##ObjBaseName)

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGlobalObject
//

interface IGlobalObject : IUnknown
{
   virtual tResult Init() = 0;
   virtual tResult Term() = 0;

   virtual const char * GetName() const = 0;
   virtual int GetConstraints(std::vector<sConstraint> * pConstraints) const = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cGlobalObject
//
// Global object base class

template <class INTERFACE, const IID * PIID, class SERVICES = cDefaultComServices>
class cGlobalObject : public cComObject2<INTERFACE, PIID,
                                         IMPLEMENTS(IGlobalObject),
                                         SERVICES>
{
public:
   cGlobalObject(const tChar * pszName = NULL,
                 const sConstraint * pConstraints = NULL,
                 int nConstraints = 0,
                 IGlobalObjectRegistry * pRegistry = NULL);
   virtual ~cGlobalObject();

   virtual tResult Init();
   virtual tResult Term();

   virtual const char * GetName() const;
   virtual int GetConstraints(std::vector<sConstraint> * pConstraints) const;

private:
   enum { kMaxGlobalObjName = 150 }; // should be big enough for a GUID if necessary
   tChar m_szName[kMaxGlobalObjName];
   std::vector<sConstraint> m_constraints;
};

///////////////////////////////////////

// for constructors/destructor
#define GLOBALOBJECT_TEMPLATE \
   template <class INTERFACE, const IID * PIID, class SERVICES> cGlobalObject<INTERFACE, PIID, SERVICES>
// for methods
#define GLOBALOBJECT_TEMPLATE_(RetType) \
   template <class INTERFACE, const IID * PIID, class SERVICES> RetType cGlobalObject<INTERFACE, PIID, SERVICES>

///////////////////////////////////////

GLOBALOBJECT_TEMPLATE::cGlobalObject(const tChar * pszName, 
                                     const sConstraint * pConstraints, 
                                     int nConstraints,
                                     IGlobalObjectRegistry * pRegistry)
{
   m_szName[0] = 0;

   if (pszName != NULL)
   {
      strncpy(m_szName, pszName, _countof(m_szName) - 1);
      m_szName[_countof(m_szName) - 1] = 0;
   }
   else
   {
      Verify(GUIDToString(*PIID, m_szName, _countof(m_szName)));
   }

   if (pConstraints != NULL && nConstraints > 0)
   {
      for (int i = 0; i < nConstraints; ++i)
      {
         m_constraints.push_back(pConstraints[i]);
      }
   }

   if (pRegistry == NULL)
   {
      pRegistry = AccessGlobalObjectRegistry();
   }

   if (pRegistry != NULL)
   {
      Verify(SUCCEEDED(pRegistry->Register(*PIID, static_cast<INTERFACE *>(this))));
   }
}

///////////////////////////////////////

GLOBALOBJECT_TEMPLATE::~cGlobalObject()
{
}

///////////////////////////////////////

GLOBALOBJECT_TEMPLATE_(tResult)::Init()
{
   // Derived classes may do something here
   return S_OK;
}

///////////////////////////////////////

GLOBALOBJECT_TEMPLATE_(tResult)::Term()
{
   // Derived classes may do something here
   return S_OK;
}

///////////////////////////////////////

GLOBALOBJECT_TEMPLATE_(const char *)::GetName() const
{
   return m_szName;
}

///////////////////////////////////////

GLOBALOBJECT_TEMPLATE_(int)::GetConstraints(std::vector<sConstraint> * pConstraints) const
{
   Assert(pConstraints != NULL);
   if (pConstraints == NULL)
   {
      return m_constraints.size();
   }
   pConstraints->clear();
   if (!m_constraints.empty())
   {
      pConstraints->resize(m_constraints.size());
      std::copy(m_constraints.begin(), m_constraints.end(), pConstraints->begin());
   }
   return pConstraints->size();
}

///////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_GLOBALOBJ_H
