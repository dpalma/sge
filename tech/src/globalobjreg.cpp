///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "globalobjreg.h"
#include "globalobj.h"
#include "digraph.h"
#include "toposort.h"

#include <vector>
#include <map>
#include <algorithm>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(GlobalObjReg);

#define LocalMsg(s)              DebugMsgEx(GlobalObjReg,(s))
#define LocalMsg1(s,a)           DebugMsgEx1(GlobalObjReg,(s),(a))
#define LocalMsg2(s,a,b)         DebugMsgEx2(GlobalObjReg,(s),(a),(b))
#define LocalMsg3(s,a,b,c)       DebugMsgEx3(GlobalObjReg,(s),(a),(b),(c))
#define LocalMsg4(s,a,b,c,d)     DebugMsgEx4(GlobalObjReg,(s),(a),(b),(c),(d))

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGlobalObjectRegistry
//

class cGlobalObjectRegistry : public cComObject<IMPLEMENTS(IGlobalObjectRegistry)>
{
public:
   cGlobalObjectRegistry();
   ~cGlobalObjectRegistry();

   virtual tResult Register(REFIID iid, IUnknown * pUnk);
   virtual IUnknown * Lookup(REFIID iid);

   virtual tResult InitAll();
   virtual tResult TermAll();

private:
   bool LookupByName(const char * pszName, IUnknown * * ppUnk, const GUID * * ppGuid) const;

   typedef cDigraph<const IID *> tConstraintGraph;
   void BuildConstraintGraph(tConstraintGraph * pGraph);

   enum eState
   {
      kPreInit,
      kLive,
      kTerminating,
      kTerminated
   };

   eState m_state;

   void SetState(eState state) { m_state = state; }
   eState GetState() const { return m_state; }

   struct sLessIid
   {
      bool operator()(const IID * pIID1, const IID * pIID2) const
      {
         return (memcmp(pIID1, pIID2, sizeof(IID)) < 0) ? true : false;
      }
   };

   typedef std::map<const IID *, IUnknown *, sLessIid> tObjMap;
   tObjMap m_objMap;

   typedef std::vector<const GUID *> tInitOrder;
   tInitOrder m_initOrder;
};

///////////////////////////////////////

cGlobalObjectRegistry::cGlobalObjectRegistry()
 : m_state(kPreInit)
{
}

///////////////////////////////////////

cGlobalObjectRegistry::~cGlobalObjectRegistry()
{
   // If either of these assertions fire, either something went
   // wrong with TermAll, or it wasn't even called.
   Assert(m_objMap.empty());
   Assert(m_initOrder.empty());
}

///////////////////////////////////////

tResult cGlobalObjectRegistry::Register(REFIID iid, IUnknown * pUnk)
{
   if (CTIsEqualUnknown(iid))
   {
      // Cannot register IID_IUnknown
      return E_INVALIDARG;
   }

   if (pUnk == NULL)
   {
      return E_POINTER;
   }

   if (GetState() != kPreInit)
   {
      return E_FAIL;
   }

   cAutoIPtr<IGlobalObject> pGO;
   if (FAILED(pUnk->QueryInterface(IID_IGlobalObject, (void**)&pGO)))
   {
      // Object must support IGlobalObject
      return E_INVALIDARG;
   }
   SafeRelease(pGO);

   if (m_objMap.find(&iid) != m_objMap.end())
   {
      DebugMsg("Unable to register global object: duplicate IID.\n");
      return S_FALSE;
   }

   cAutoIPtr<IUnknown> pPostQI;
   if (FAILED(pUnk->QueryInterface(iid, (void**)&pPostQI)))
   {
      // Object must support its own interface
      return E_INVALIDARG;
   }

   // Store the QI'ed pointer
   m_objMap[&iid] = CTAddRef(pPostQI);

   return S_OK;
}

///////////////////////////////////////

tResult cGlobalObjectRegistry::InitAll()
{
   Assert(GetState() == kPreInit);

   tConstraintGraph constraintGraph;
   BuildConstraintGraph(&constraintGraph);

   cTopoSorter<tConstraintGraph>().TopoSort(&constraintGraph, &m_initOrder);

   // Let the state be live during initialization so the global objects
   // may refer to each other.
   SetState(kLive);

   tInitOrder::iterator iter;
   for (iter = m_initOrder.begin(); iter != m_initOrder.end(); iter++)
   {
      cAutoIPtr<IUnknown> pUnk(Lookup(*(*iter)));
      if (!!pUnk)
      {
         cAutoIPtr<IGlobalObject> pGO;
         if (pUnk->QueryInterface(IID_IGlobalObject, (void**)&pGO) == S_OK)
         {
            LocalMsg1("Initializing global object %s\n", pGO->GetName());

            tResult initResult = pGO->Init();
            if (initResult == S_FALSE)
            {
               m_objMap.erase(*iter);
            }
            else if (FAILED(initResult))
            {
               ErrorMsg1("%s failed to initialize\n", pGO->GetName());
               return initResult;
            }
         }
      }
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGlobalObjectRegistry::TermAll()
{
   if (GetState() == kLive)
   {
      Assert(m_objMap.size() == m_initOrder.size());

      SetState(kTerminating);

      // Terminate in reverse order
      tInitOrder::reverse_iterator iter;
      for (iter = m_initOrder.rbegin(); iter != m_initOrder.rend(); iter++)
      {
         cAutoIPtr<IUnknown> pUnk(Lookup(*(*iter)));
         if (!!pUnk)
         {
            cAutoIPtr<IGlobalObject> pGO;
            if (pUnk->QueryInterface(IID_IGlobalObject, (void**)&pGO) == S_OK)
            {
               pGO->Term();
            }
         }
      }

      // Release references in m_objMap (order doesn't matter here)
      tObjMap::iterator oiter;
      for (oiter = m_objMap.begin(); oiter != m_objMap.end(); oiter++)
      {
         oiter->second->Release();
      }

      m_initOrder.clear();
      m_objMap.clear();

      SetState(kTerminated);

      return S_OK;
   }
#ifndef NDEBUG
   else
   {
      Assert(m_initOrder.empty());
      Assert(m_objMap.empty());
      Assert(GetState() == kTerminated);
   }
#endif

   return S_FALSE;
}

///////////////////////////////////////

IUnknown * cGlobalObjectRegistry::Lookup(REFIID iid)
{
   tObjMap::iterator iter = m_objMap.find(&iid);
   if (iter != m_objMap.end())
   {
      Assert(iter->second != NULL);
#ifdef _DEBUG
      // is the interface pointer callable?
      iter->second->AddRef();
      iter->second->Release();
#endif
      return CTAddRef(iter->second);
   }
   return NULL;
}

///////////////////////////////////////

bool cGlobalObjectRegistry::LookupByName(const char * pszName, IUnknown * * ppUnk, const GUID * * ppGuid) const
{
   // TODO: Index by name as well as by GUID so a linear search isn't necessary
   for (tObjMap::const_iterator iter = m_objMap.begin(); iter != m_objMap.end(); iter++)
   {
      cAutoIPtr<IGlobalObject> pGlobalObj;
      Verify(SUCCEEDED(iter->second->QueryInterface(IID_IGlobalObject, (void**)&pGlobalObj)));

      if (strcmp(pGlobalObj->GetName(), pszName) == 0)
      {
         if (ppUnk != NULL)
         {
            iter->second->QueryInterface(IID_IUnknown, (void * *)ppUnk);
         }
         if (ppGuid != NULL)
         {
            *ppGuid = iter->first;
         }
         return true;
      }
   }
   return false;
}

///////////////////////////////////////

void cGlobalObjectRegistry::BuildConstraintGraph(tConstraintGraph * pGraph)
{
   // add nodes
   tObjMap::iterator iter;
   for (iter = m_objMap.begin(); iter != m_objMap.end(); iter++)
   {
      pGraph->AddNode(iter->first);
   }

   // add constraints as edges
   for (iter = m_objMap.begin(); iter != m_objMap.end(); iter++)
   {
      cAutoIPtr<IGlobalObject> pGlobalObj;
      Verify(SUCCEEDED(iter->second->QueryInterface(IID_IGlobalObject, (void**)&pGlobalObj)));

      typedef std::vector<sConstraint> tConstraints;

      tConstraints constraints;
      if (pGlobalObj->GetConstraints(&constraints) > 0)
      {
         tConstraints::iterator citer;
         for (citer = constraints.begin(); citer != constraints.end(); citer++)
         {
            const GUID * pTargetGuid = NULL;

            if (citer->against == kCA_Guid)
            {
               cAutoIPtr<IUnknown> pTargetUnk(Lookup(*(citer->pGUID)));
               if (!!pTargetUnk)
               {
                  pTargetGuid = citer->pGUID;
               }
            }
            else if (citer->against == kCA_Name)
            {
               LookupByName(citer->pszName, NULL, &pTargetGuid);
            }

            if (pTargetGuid != NULL)
            {
               if (citer->when == kCW_Before)
               {
                  pGraph->AddEdge(iter->first, pTargetGuid);
               }
               else if (citer->when == kCW_After)
               {
                  pGraph->AddEdge(pTargetGuid, iter->first);
               }
            }
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSingletonGlobalObjectRegistry
//

class cSingletonGlobalObjectRegistry : public cGlobalObjectRegistry
{
public:
   static IGlobalObjectRegistry * Access();
   virtual void DeleteThis();
   static void TermAllAtExit();
   virtual tResult InitAll();
private:
   static cSingletonGlobalObjectRegistry gm_instance;
};

////////////////////////////////////////

cSingletonGlobalObjectRegistry cSingletonGlobalObjectRegistry::gm_instance;

////////////////////////////////////////

IGlobalObjectRegistry * cSingletonGlobalObjectRegistry::Access()
{
   return static_cast<IGlobalObjectRegistry *>(&gm_instance);
}

////////////////////////////////////////

void cSingletonGlobalObjectRegistry::DeleteThis()
{
   // Do nothing
}

///////////////////////////////////////

void cSingletonGlobalObjectRegistry::TermAllAtExit()
{
   Access()->TermAll();
}

///////////////////////////////////////

tResult cSingletonGlobalObjectRegistry::InitAll()
{
   atexit(TermAllAtExit);
   return cGlobalObjectRegistry::InitAll();
}

////////////////////////////////////////

IGlobalObjectRegistry * AccessGlobalObjectRegistry()
{
   return cSingletonGlobalObjectRegistry::Access();
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////

// {A2A64E3E-4549-4a54-B43A-313DD78E9192}
EXTERN_C const GUID IID_IFooGlobalObj = 
{ 0xa2a64e3e, 0x4549, 0x4a54, { 0xb4, 0x3a, 0x31, 0x3d, 0xd7, 0x8e, 0x91, 0x92 } };

interface IFooGlobalObj : IUnknown
{
   virtual void Foo() = 0;
};

////////////////////////////////////////

class cFooGlobalObj : public cGlobalObject<IMPLEMENTS(IFooGlobalObj)>
{
public:
   cFooGlobalObj(IGlobalObjectRegistry * pReg);
   ~cFooGlobalObj();

   virtual void Foo();
};

////////////////////////////////////////

BEGIN_CONSTRAINTS_NAMED(g_fooConstraints)
   BEFORE_NAME("BarGlobalObj")
END_CONSTRAINTS()

////////////////////////////////////////

cFooGlobalObj::cFooGlobalObj(IGlobalObjectRegistry * pReg)
 : cGlobalObject<IMPLEMENTS(IFooGlobalObj)>("FooGlobalObj", CONSTRAINTS_NAMED(g_fooConstraints), pReg)
{
   LocalMsg("cFooGlobalObj::cFooGlobalObj()\n");
}

////////////////////////////////////////

cFooGlobalObj::~cFooGlobalObj()
{
   LocalMsg("cFooGlobalObj::~cFooGlobalObj()\n");
}

////////////////////////////////////////

void cFooGlobalObj::Foo()
{
   LocalMsg("cFooGlobalObj::Foo()\n");
}

///////////////////////////////////////////////////////////////////////////////

// {D0C9B0BF-B0C6-4d28-B379-3C5F01DA0890}
EXTERN_C const GUID IID_IBarGlobalObj = 
{ 0xd0c9b0bf, 0xb0c6, 0x4d28, { 0xb3, 0x79, 0x3c, 0x5f, 0x1, 0xda, 0x8, 0x90 } };

interface IBarGlobalObj : IUnknown
{
   virtual void Bar() = 0;
};

////////////////////////////////////////

class cBarGlobalObj : public cGlobalObject<IMPLEMENTS(IBarGlobalObj)>
{
public:
   cBarGlobalObj(IGlobalObjectRegistry * pReg);
   ~cBarGlobalObj();

   virtual void Bar();
};

////////////////////////////////////////

BEGIN_CONSTRAINTS_NAMED(g_barConstraints)
   AFTER_GUID(IID_IFooGlobalObj) // redundant with BEFORE_NAME("Bar") above--for testing
END_CONSTRAINTS()

////////////////////////////////////////

cBarGlobalObj::cBarGlobalObj(IGlobalObjectRegistry * pReg)
 : cGlobalObject<IMPLEMENTS(IBarGlobalObj)>("BarGlobalObj", CONSTRAINTS_NAMED(g_barConstraints), pReg)
{
   LocalMsg("cBarGlobalObj::cBarGlobalObj()\n");
}

////////////////////////////////////////

cBarGlobalObj::~cBarGlobalObj()
{
   LocalMsg("cBarGlobalObj::~cBarGlobalObj()\n");
}

////////////////////////////////////////

void cBarGlobalObj::Bar()
{
   LocalMsg("cBarGlobalObj::Bar()\n");
}

///////////////////////////////////////////////////////////////////////////////

class cGlobalObjectRegistryTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cGlobalObjectRegistryTests);
      CPPUNIT_TEST(TestGlobalObjReg);
   CPPUNIT_TEST_SUITE_END();

   void TestGlobalObjReg();
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cGlobalObjectRegistryTests);

////////////////////////////////////////

void cGlobalObjectRegistryTests::TestGlobalObjReg()
{
   cAutoIPtr<IGlobalObjectRegistry> pRegistry(
      static_cast<IGlobalObjectRegistry *>(new cGlobalObjectRegistry));

   cAutoIPtr<IBarGlobalObj> pBar(static_cast<IBarGlobalObj *>(new cBarGlobalObj(pRegistry)));
   cAutoIPtr<IFooGlobalObj> pFoo(static_cast<IFooGlobalObj *>(new cFooGlobalObj(pRegistry)));

   CPPUNIT_ASSERT(pRegistry->InitAll() == S_OK);

   cAutoIPtr<IFooGlobalObj> pFoo2 = (IFooGlobalObj *)pRegistry->Lookup(IID_IFooGlobalObj);
   CPPUNIT_ASSERT(CTIsSameObject(pFoo, pFoo2));

   cAutoIPtr<IBarGlobalObj> pBar2 = (IBarGlobalObj *)pRegistry->Lookup(IID_IBarGlobalObj);
   CPPUNIT_ASSERT(CTIsSameObject(pBar, pBar2));

   CPPUNIT_ASSERT(pRegistry->TermAll() == S_OK);
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNIT
