///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GLOBALOBJ_H
#define INCLUDED_GLOBALOBJ_H

#include "techdll.h"
#include "comtools.h"
#include "constraints.h"
#include "globalobjreg.h"

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

inline tResult RegisterGlobalObject(REFIID iid, IUnknown * pUnk)
{
   return AccessGlobalObjectRegistry()->Register(iid, pUnk);
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

const int kMaxGlobalObjName = 150; // should be big enough for a GUID if necessary

struct sGlobalObjDesc
{
   tChar name[kMaxGlobalObjName];
   sConstraint * constraints;
};

interface IGlobalObject : IUnknown
{
   virtual tResult Init() = 0;
   virtual tResult Term() = 0;
   
   virtual const sGlobalObjDesc * GetGlobalObjDesc() const = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cGlobalObject
//
// Global object base class

template <class INTERFACE, const IID * PIID, class SERVICES = cDefaultComServices>
class cGlobalObject : public cComObject2<INTERFACE, PIID,
                                         IGlobalObject, &IID_IGlobalObject,
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

   virtual const sGlobalObjDesc * GetGlobalObjDesc() const { return &m_desc; }

private:
   sGlobalObjDesc m_desc;
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
   memset(&m_desc, 0, sizeof(m_desc));

   tChar szGUID[kGuidStringLength];
   if (pszName == NULL)
   {
      Verify(GUIDToString(*PIID, szGUID, _countof(szGUID)));
      pszName = szGUID;
   }

   _tcscpy(m_desc.name, pszName);

   if (pRegistry == NULL)
      pRegistry = AccessGlobalObjectRegistry();

   if (pRegistry != NULL)
   {
      Verify(SUCCEEDED(pRegistry->Register(*PIID, static_cast<INTERFACE *>(this))));
   }

   if (pConstraints != NULL)
   {
      m_desc.constraints = new sConstraint[nConstraints + 1];
      memcpy(m_desc.constraints, pConstraints, nConstraints * sizeof(sConstraint));
      memset(&m_desc.constraints[nConstraints], 0, sizeof(sConstraint));
   }
}

///////////////////////////////////////

GLOBALOBJECT_TEMPLATE::~cGlobalObject()
{
   if (m_desc.constraints != NULL)
   {
      delete [] m_desc.constraints;
      m_desc.constraints = NULL;
   }
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

///////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_GLOBALOBJ_H
