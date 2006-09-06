///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "globalobjdef.h"
#include "digraph.h"
#include "hashtable.h"
#include "hashtabletem.h"
#include "toposort.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <vector>
#include <algorithm>

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(GlobalObjReg);

#define LocalMsg(s)              DebugMsgEx(GlobalObjReg,s)
#define LocalMsg1(s,a)           DebugMsgEx1(GlobalObjReg,s,(a))
#define LocalMsg2(s,a,b)         DebugMsgEx2(GlobalObjReg,s,(a),(b))
#define LocalMsg3(s,a,b,c)       DebugMsgEx3(GlobalObjReg,s,(a),(b),(c))
#define LocalMsg4(s,a,b,c,d)     DebugMsgEx4(GlobalObjReg,s,(a),(b),(c),(d))

F_DECLARE_INTERFACE(IGlobalObjectRegistry);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cBeforeAfterConstraint
//

////////////////////////////////////////

cBeforeAfterConstraint::cBeforeAfterConstraint(const GUID * pGuid, eBeforeAfter beforeAfter)
 : m_pGuid(pGuid)
 , m_pszName(NULL)
 , m_beforeAfter(beforeAfter)
{
}

////////////////////////////////////////

cBeforeAfterConstraint::cBeforeAfterConstraint(const tChar * pszName, eBeforeAfter beforeAfter)
 : m_pGuid(NULL)
 , m_pszName(pszName)
 , m_beforeAfter(beforeAfter)
{
}

////////////////////////////////////////

cBeforeAfterConstraint::cBeforeAfterConstraint(const cBeforeAfterConstraint & other)
 : m_pGuid(other.m_pGuid)
 , m_pszName(other.m_pszName)
 , m_beforeAfter(other.m_beforeAfter)
{
}

////////////////////////////////////////

cBeforeAfterConstraint::~cBeforeAfterConstraint()
{
}

////////////////////////////////////////

const cBeforeAfterConstraint & cBeforeAfterConstraint::operator =(const cBeforeAfterConstraint & other)
{
   m_pGuid = other.m_pGuid;
   m_pszName = other.m_pszName;
   m_beforeAfter = other.m_beforeAfter;
   return *this;
}


///////////////////////////////////////////////////////////////////////////////

typedef const GUID * tGuidPtr;

template <>
uint cHashFunction<tGuidPtr>::Hash(const tGuidPtr & a, uint initVal)
{
   return hash((byte*)a, sizeof(GUID), initVal);
}

template <>
bool cHashFunction<tGuidPtr>::Equal(const tGuidPtr & a, const tGuidPtr & b)
{
   return CTIsEqualGUID(*a, *b);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGlobalObjectRegistry
//

class cGlobalObjectRegistry : public cComObject<IMPLEMENTS(IGlobalObjectRegistry)>
{
public:
   cGlobalObjectRegistry();
   ~cGlobalObjectRegistry();

   virtual tResult Register(REFGUID iid, IUnknown * pUnk);
   virtual IUnknown * Lookup(REFGUID iid);

   virtual tResult InitAll();
   virtual tResult TermAll();

private:
   bool LookupByName(const tChar * pszName, IUnknown * * ppUnk, const GUID * * ppGuid) const;

   struct sLessGuid
   {
      bool operator()(const GUID * pLhs, const GUID * pRhs) const
      {
         return (memcmp(pLhs, pRhs, sizeof(GUID)) < 0);
      }
   };

   typedef cDigraph<const GUID *, int, sLessGuid> tConstraintGraph;
   void BuildConstraintGraph(tConstraintGraph * pGraph);

   typedef cHashTable<const GUID *, IUnknown *> tObjMap;
   tObjMap m_objMap;

   typedef std::vector<const GUID *> tInitOrder;
   tInitOrder m_initOrder;
};

///////////////////////////////////////

cGlobalObjectRegistry::cGlobalObjectRegistry()
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

tResult cGlobalObjectRegistry::Register(REFGUID iid, IUnknown * pUnk)
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
   tConstraintGraph constraintGraph;
   BuildConstraintGraph(&constraintGraph);

   cTopoSorter<tConstraintGraph::node_type> sorter(&m_initOrder);
   constraintGraph.topological_sort(sorter);

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
   Assert(m_objMap.size() == m_initOrder.size());

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
            LocalMsg1("Terminating global object %s\n", pGO->GetName());

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

   return S_OK;
}

///////////////////////////////////////

IUnknown * cGlobalObjectRegistry::Lookup(REFGUID iid)
{
   tObjMap::iterator iter = m_objMap.find(&iid);
   if (iter != m_objMap.end())
   {
      return CTAddRef(iter->second);
   }
   return NULL;
}

///////////////////////////////////////

bool cGlobalObjectRegistry::LookupByName(const tChar * pszName, IUnknown * * ppUnk, const GUID * * ppGuid) const
{
   // TODO: Index by name as well as by GUID so a linear search isn't necessary
   for (tObjMap::const_iterator iter = m_objMap.begin(); iter != m_objMap.end(); iter++)
   {
      cAutoIPtr<IGlobalObject> pGlobalObj;
      Verify(SUCCEEDED(iter->second->QueryInterface(IID_IGlobalObject, (void**)&pGlobalObj)));

      if (_tcscmp(pGlobalObj->GetName(), pszName) == 0)
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
      pGraph->insert(iter->first);
   }

   Assert(pGraph->size() == m_objMap.size());

#ifdef _DEBUG
   {
      tObjMap::iterator iter;
      for (iter = m_objMap.begin(); iter != m_objMap.end(); iter++)
      {
         Assert(pGraph->find(iter->first) != pGraph->end());
      }
   }
#endif

   // add constraints as edges
   for (iter = m_objMap.begin(); iter != m_objMap.end(); iter++)
   {
      cAutoIPtr<IGlobalObject> pGlobalObj;
      Verify(SUCCEEDED(iter->second->QueryInterface(IID_IGlobalObject, (void**)&pGlobalObj)));

      tBeforeAfterConstraints constraints;
      if (pGlobalObj->GetConstraints(&constraints) > 0)
      {
         tBeforeAfterConstraints::iterator citer;
         for (citer = constraints.begin(); citer != constraints.end(); citer++)
         {
            const GUID * pTargetGuid = NULL;

            if (citer->GetGuid() != NULL)
            {
               cAutoIPtr<IUnknown> pTargetUnk(Lookup(*(citer->GetGuid())));
               if (!!pTargetUnk)
               {
                  pTargetGuid = citer->GetGuid();
               }
            }
            else if (citer->GetName() != NULL)
            {
               LookupByName(citer->GetName(), NULL, &pTargetGuid);
            }

            if (pTargetGuid != NULL)
            {
               if (citer->Before())
               {
                  pGraph->insert_edge(iter->first, pTargetGuid, 0);
               }
               else
               {
                  pGraph->insert_edge(pTargetGuid, iter->first, 0);
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
IGlobalObjectRegistry * g_pGlobalObjectRegistry = cSingletonGlobalObjectRegistry::Access();

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

///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////

IGlobalObjectRegistry * AccessGlobalObjectRegistry()
{
   return cSingletonGlobalObjectRegistry::Access();
}

////////////////////////////////////////

tResult RegisterGlobalObject(REFGUID iid, IUnknown * pUnk)
{
   return AccessGlobalObjectRegistry()->Register(iid, pUnk);
}

////////////////////////////////////////

tResult StartGlobalObjects()
{
   return AccessGlobalObjectRegistry()->InitAll();
}

////////////////////////////////////////

tResult StopGlobalObjects()
{
   return AccessGlobalObjectRegistry()->TermAll();
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////

// Used to track the order in which calls to IGlobalObject::Init are made
static uint g_initCounter = 0;

///////////////////////////////////////////////////////////////////////////////

// {A2A64E3E-4549-4a54-B43A-313DD78E9192}
EXTERN_C const GUID IID_IFooGlobalObj = 
{ 0xa2a64e3e, 0x4549, 0x4a54, { 0xb4, 0x3a, 0x31, 0x3d, 0xd7, 0x8e, 0x91, 0x92 } };

interface IFooGlobalObj : IUnknown
{
   virtual void Foo() = 0;
};

////////////////////////////////////////

class cFooGlobalObj : public cComObject2<IMPLEMENTS(IFooGlobalObj), IMPLEMENTS(IGlobalObject)>
{
public:
   cFooGlobalObj(IGlobalObjectRegistry * pRegistry);
   ~cFooGlobalObj();

   static uint gm_initCount;

   DECLARE_NAME(FooGlobalObj)
   DECLARE_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual void Foo();
};

////////////////////////////////////////

uint cFooGlobalObj::gm_initCount = 0;

////////////////////////////////////////

BEGIN_CONSTRAINTS(cFooGlobalObj)
   BEFORE_NAME(BarGlobalObj)
END_CONSTRAINTS()

////////////////////////////////////////

cFooGlobalObj::cFooGlobalObj(IGlobalObjectRegistry * pRegistry)
{
   LocalMsg("cFooGlobalObj::cFooGlobalObj()\n");
   Verify(pRegistry->Register(IID_IFooGlobalObj, static_cast<IFooGlobalObj*>(this)) == S_OK);
}

////////////////////////////////////////

cFooGlobalObj::~cFooGlobalObj()
{
   LocalMsg("cFooGlobalObj::~cFooGlobalObj()\n");
}

////////////////////////////////////////

tResult cFooGlobalObj::Init()
{
   LocalMsg("cFooGlobalObj::Init()\n");
   gm_initCount = ++g_initCounter;
   return S_OK;
}

////////////////////////////////////////

tResult cFooGlobalObj::Term()
{
   LocalMsg("cFooGlobalObj::Term()\n");
   return S_OK;
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

class cBarGlobalObj : public cComObject2<IMPLEMENTS(IBarGlobalObj), IMPLEMENTS(IGlobalObject)>
{
public:
   cBarGlobalObj(IGlobalObjectRegistry * pRegistry);
   ~cBarGlobalObj();

   static uint gm_initCount;

   DECLARE_NAME(BarGlobalObj)
   DECLARE_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual void Bar();
};

////////////////////////////////////////

uint cBarGlobalObj::gm_initCount = 0;

////////////////////////////////////////

BEGIN_CONSTRAINTS(cBarGlobalObj)
   AFTER_GUID(IID_IFooGlobalObj) // redundant with BEFORE_NAME(BarGlobalObj) above--for testing
END_CONSTRAINTS()

////////////////////////////////////////

cBarGlobalObj::cBarGlobalObj(IGlobalObjectRegistry * pRegistry)
{
   LocalMsg("cBarGlobalObj::cBarGlobalObj()\n");
   Verify(pRegistry->Register(IID_IBarGlobalObj, static_cast<IBarGlobalObj*>(this)) == S_OK);
}

////////////////////////////////////////

cBarGlobalObj::~cBarGlobalObj()
{
   LocalMsg("cBarGlobalObj::~cBarGlobalObj()\n");
}

////////////////////////////////////////

tResult cBarGlobalObj::Init()
{
   LocalMsg("cBarGlobalObj::Init()\n");
   gm_initCount = ++g_initCounter;
   LocalMsg("cBarGlobalObj::Init()\n");
   return S_OK;
}

////////////////////////////////////////

tResult cBarGlobalObj::Term()
{
   LocalMsg("cBarGlobalObj::Term()\n");
   return S_OK;
}

////////////////////////////////////////

void cBarGlobalObj::Bar()
{
   LocalMsg("cBarGlobalObj::Bar()\n");
}

///////////////////////////////////////////////////////////////////////////////

TEST(GlobalObjectRegistry)
{
   cAutoIPtr<IGlobalObjectRegistry> pRegistry(
      static_cast<IGlobalObjectRegistry *>(new cGlobalObjectRegistry));

   // Register the "Bar" object before "Foo" for testing because the 
   // initialization constraints should actually make its init happen after.
   cAutoIPtr<IBarGlobalObj> pBar(static_cast<IBarGlobalObj *>(new cBarGlobalObj(pRegistry)));
   cAutoIPtr<IFooGlobalObj> pFoo(static_cast<IFooGlobalObj *>(new cFooGlobalObj(pRegistry)));

   CHECK(pRegistry->InitAll() == S_OK);

   cAutoIPtr<IFooGlobalObj> pFoo2(static_cast<IFooGlobalObj *>(pRegistry->Lookup(IID_IFooGlobalObj)));
   CHECK(CTIsSameObject(pFoo, pFoo2));

   cAutoIPtr<IBarGlobalObj> pBar2(static_cast<IBarGlobalObj *>(pRegistry->Lookup(IID_IBarGlobalObj)));
   CHECK(CTIsSameObject(pBar, pBar2));

   // "Bar" should be inititalized after "Foo"
   CHECK(cBarGlobalObj::gm_initCount > cFooGlobalObj::gm_initCount);

   CHECK(pRegistry->TermAll() == S_OK);

   // After "TermAll", nothing should work
   CHECK(pRegistry->Lookup(IID_IFooGlobalObj) == NULL);
   CHECK(pRegistry->Lookup(IID_IBarGlobalObj) == NULL);
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_UNITTESTPP
