///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"
#include "globalobjreg.h"
#include "globalobj.h"
#include "digraph.h"
#include "toposort.h"
#include <vector>
#include <map>

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

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Template specialization for pair that compares IID/Interface pairs

bool CDECL operator==(const pair<const IID *, IGlobalObject *> & P1,
                      const pair<const IID *, IGlobalObject *> & P2)
{
   if (!CTIsEqualGUID(*(P1.first), *(P2.first)))
      return false;

   if (!CTIsSameObject(P1.second, P2.second))
      return false;

   return true;
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

   virtual tResult Register(REFIID iid, IUnknown * pUnk);
   virtual IUnknown * Lookup(REFIID iid);

   virtual tResult InitAll();
   virtual tResult TermAll();

private:
   enum eState
   {
      kPreInit,
      kLive,
      kPostTerm,
   };

   void SetState(eState state) { m_state = state; }
   eState GetState() const { return m_state; }

   struct less_iid
   {
      bool operator()(const IID * pIID1, const IID * pIID2) const
      {
         return (memcmp(pIID1, pIID2, sizeof(IID)) < 0) ? true : false;
      }
   };

   typedef map<const IID *, IUnknown *, less_iid> tObjMap;
   tObjMap m_objMap;

   typedef vector<IGlobalObject *> tInitOrder;
   tInitOrder m_initOrder;

   eState m_state;
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
   Assert(!CTIsEqualUnknown(iid));
   Assert(pUnk != NULL);

   if (GetState() == kPreInit)
   {
      // Object must support IGlobalObject
      cAutoIPtr<IGlobalObject> pGlobalObject;
      if (SUCCEEDED(pUnk->QueryInterface(IID_IGlobalObject, (void**)&pGlobalObject)))
      {
         IUnknown * pPostQI = NULL;
         if (SUCCEEDED(pUnk->QueryInterface(iid, (void**)&pPostQI)))
         {
            if (m_objMap.find(&iid) == m_objMap.end())
            {
               m_objMap.insert(make_pair(&iid, pPostQI));
               return S_OK;
            }

            DebugMsg("Unable to register global object. Duplicate IID.\n");
            SafeRelease(pPostQI);
         }
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGlobalObjectRegistry::InitAll()
{
   Assert(GetState() == kPreInit);

   tResult result = S_OK; // a little cheesy to assume success...

   typedef cDigraph<const IID *, IGlobalObject *> tInitGraph;
   tInitGraph initGraph;

   // add nodes
   tObjMap::iterator iter;
   for (iter = m_objMap.begin(); iter != m_objMap.end(); iter++)
   {
      const IID * pIID = iter->first;
      IUnknown * pUnk = iter->second;

      // If the object doesn't support IGlobalObject it shouldn't have 
      // made it past the Register() step.
      IGlobalObject * pGlobalObj = NULL;
      Verify(SUCCEEDED(pUnk->QueryInterface(IID_IGlobalObject, (void**)&pGlobalObj)));
      initGraph.AddNode(pIID, pGlobalObj);

      // Release the reference from the QI call above even though the pointer
      // was added to "initGraph", because a reference to the same object is
      // held by the "m_objMap" array.
      pGlobalObj->Release();
   }

   // add constraints as edges
   for (iter = m_objMap.begin(); iter != m_objMap.end(); iter++)
   {
      const IID * pIID = iter->first;
      IUnknown * pUnk = iter->second;

      cAutoIPtr<IGlobalObject> pGlobalObj;
      Verify(SUCCEEDED(pUnk->QueryInterface(IID_IGlobalObject, (void**)&pGlobalObj)));

      std::vector<sConstraint> constraints;
      if (pGlobalObj->GetConstraints(&constraints) > 0)
      {
         std::vector<sConstraint>::iterator iter;
         for (iter = constraints.begin(); iter != constraints.end(); iter++)
         {
            IUnknown * pTargetUnk = NULL;
            const GUID * pTargetGUID = NULL;

            if (iter->against == kCA_Guid)
            {
               tObjMap::iterator found = m_objMap.find(iter->pGUID);
               if (found != m_objMap.end())
               {
                  pTargetUnk = found->second;
                  pTargetGUID = found->first;
               }
            }
            else if (iter->against == kCA_Name)
            {
               // TODO: allow lookup by name as well as by GUID so this
               // linear search isn't necessary
               for (tObjMap::iterator jter = m_objMap.begin(); jter != m_objMap.end(); jter++)
               {
                  cAutoIPtr<IGlobalObject> pGlobalObj;
                  Verify(SUCCEEDED(jter->second->QueryInterface(IID_IGlobalObject, (void**)&pGlobalObj)));

                  if (strcmp(pGlobalObj->GetName(), iter->pszName) == 0)
                  {
                     pTargetUnk = jter->second;
                     pTargetGUID = jter->first;
                     break;
                  }
               }
            }

            if (pTargetUnk != NULL)
            {
#ifndef NDEBUG
               cAutoIPtr<IGlobalObject> pTargetGlobalObj;
               Verify(SUCCEEDED(pTargetUnk->QueryInterface(IID_IGlobalObject, (void**)&pTargetGlobalObj)));
#endif

               // add edge
               if (iter->when == kCW_Before)
               {
                  LocalMsg2("%s initialized before %s\n",
                     pGlobalObj->GetName(), pTargetGlobalObj->GetName());
                  initGraph.AddEdge(pIID, pTargetGUID);
               }
               else if (iter->when == kCW_After)
               {
                  LocalMsg2("%s initialized after %s\n",
                     pGlobalObj->GetName(), pTargetGlobalObj->GetName());
                  initGraph.AddEdge(pTargetGUID, pIID);
               }
            }
#ifndef NDEBUG
            else
            {
               char szGUID[kGuidStringLength];
               GUIDToString(*(iter->pGUID), szGUID, _countof(szGUID));

               DebugMsg3("%s initialized %s non-existent object %s\n",
                  pGlobalObj->GetName(),
                  (iter->when == kCW_Before) ? "before" : "after",
                  (iter->against == kCA_Guid) ? szGUID : iter->pszName);
            }
#endif
         }
      }
   }

   cTopoSorter<tInitGraph> sorter;
   typedef vector<const IID *> tInitOrderIIDVector;
   tInitOrderIIDVector initOrderIIDs;
   sorter.TopoSort(&initGraph, &initOrderIIDs);

   // Let the state be live during initialization so the global objects
   // may refer to each other.
   SetState(kLive);

   Assert(m_initOrder.size() == 0);
   for (tInitOrderIIDVector::size_type i = 0; i < initOrderIIDs.size(); i++)
   {
      IGlobalObject * pGlobalObj = initGraph.GetNodeData(initOrderIIDs[i]);

      LocalMsg1("Initializing global object %s\n", pGlobalObj->GetName());

      if (SUCCEEDED(pGlobalObj->Init()))
      {
         m_initOrder.push_back(pGlobalObj);
      }
      else
      {
         // if the initialization failed, exit the loop and return an error
         DebugMsg("Init failed in cGlobalObjectRegistry::InitAll!!!\n");
         result = E_FAIL;
         break;
      }
   }

   return result;
}

///////////////////////////////////////

tResult cGlobalObjectRegistry::TermAll()
{
   if (GetState() == kLive)
   {
      Assert(m_objMap.size() == m_initOrder.size());
      // Terminate in reverse order
      tInitOrder::reverse_iterator riter;
      for (riter = m_initOrder.rbegin(); riter != m_initOrder.rend(); riter++)
      {
         IGlobalObject * p = (*riter);
         LocalMsg1("Releasing global object %s\n", p->GetName());
         (*riter)->Term();
         (*riter)->Release();
      }
      m_initOrder.clear();
      m_objMap.clear();
      SetState(kPostTerm);
   }
#ifndef NDEBUG
   else
   {
      Assert(m_initOrder.empty());
      Assert(m_objMap.empty());
      Assert(GetState() == kPostTerm);
   }
#endif
   return S_OK;
}

///////////////////////////////////////

IUnknown * cGlobalObjectRegistry::Lookup(REFIID iid)
{
   tObjMap::iterator iter = m_objMap.find(&iid);
   if (iter != m_objMap.end())
   {
      Assert(iter->second != NULL);
      iter->second->AddRef();
      return iter->second;
   }
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cStackGlobalObjectRegistry
//

class cStackGlobalObjectRegistry : public cGlobalObjectRegistry
{
public:
   virtual ULONG STDMETHODCALLTYPE AddRef() { return 2; }
   virtual ULONG STDMETHODCALLTYPE Release() { return 1; }
};

static cStackGlobalObjectRegistry g_globalObjectRegistry;

IGlobalObjectRegistry * AccessGlobalObjectRegistry()
{
   return &g_globalObjectRegistry;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////

// {A2A64E3E-4549-4a54-B43A-313DD78E9192}
EXTERN_C const GUID IID_IFoo = 
{ 0xa2a64e3e, 0x4549, 0x4a54, { 0xb4, 0x3a, 0x31, 0x3d, 0xd7, 0x8e, 0x91, 0x92 } };

interface IFoo : IUnknown
{
   virtual void Foo() = 0;
};

BEGIN_CONSTRAINTS_NAMED(g_fooConstraints)
   BEFORE_NAME("Bar")
END_CONSTRAINTS()

class cFoo : public cGlobalObject<IMPLEMENTS(IFoo)>
{
public:
   cFoo(IGlobalObjectRegistry * pReg = NULL)
    : cGlobalObject<IMPLEMENTS(IFoo)>("Foo", CONSTRAINTS_NAMED(g_fooConstraints), pReg)
   {
   }

   virtual void Foo()
   {
      DebugMsg("Foo\n");
   }
};

///////////////////////////////////////////////////////////////////////////////

// {D0C9B0BF-B0C6-4d28-B379-3C5F01DA0890}
EXTERN_C const GUID IID_IBar = 
{ 0xd0c9b0bf, 0xb0c6, 0x4d28, { 0xb3, 0x79, 0x3c, 0x5f, 0x1, 0xda, 0x8, 0x90 } };

interface IBar : IUnknown
{
   virtual void Bar() = 0;
};

BEGIN_CONSTRAINTS_NAMED(g_barConstraints)
   AFTER_GUID(IID_IFoo) // redundant with BEFORE_NAME("Bar") above--for testing
END_CONSTRAINTS()

class cBar : public cGlobalObject<IMPLEMENTS(IBar)>
{
public:
   cBar(IGlobalObjectRegistry * pReg = NULL)
    : cGlobalObject<IMPLEMENTS(IBar)>("Bar", CONSTRAINTS_NAMED(g_barConstraints), pReg)
   {
   }

   virtual void Bar()
   {
      DebugMsg("Bar\n");
   }
};

///////////////////////////////////////////////////////////////////////////////

class cGlobalObjectRegistryTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cGlobalObjectRegistryTests);
      CPPUNIT_TEST(TestGlobalObjReg);
   CPPUNIT_TEST_SUITE_END();

   void TestGlobalObjReg()
   {
      cAutoIPtr<IBar> pBar = new cBar(m_pRegistry);
      cAutoIPtr<IFoo> pFoo = new cFoo(m_pRegistry);

      CPPUNIT_ASSERT(m_pRegistry->InitAll() == S_OK);

      cAutoIPtr<IFoo> pFoo2 = (IFoo *)m_pRegistry->Lookup(IID_IFoo);
      CPPUNIT_ASSERT(CTIsSameObject(pFoo, pFoo2));

      cAutoIPtr<IBar> pBar2 = (IBar *)m_pRegistry->Lookup(IID_IBar);
      CPPUNIT_ASSERT(CTIsSameObject(pBar, pBar2));

      CPPUNIT_ASSERT(m_pRegistry->TermAll() == S_OK);
   }

   cAutoIPtr<IGlobalObjectRegistry> m_pRegistry;

public:
   virtual void setUp()
   {
      m_pRegistry = new cGlobalObjectRegistry;
   }

   virtual void tearDown()
   {
      SafeRelease(m_pRegistry);
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(cGlobalObjectRegistryTests);

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNIT
