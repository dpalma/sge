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

protected:
   enum eState
   {
      kPreInit,
      kLive,
      kPostTerm,
   };

   void SetState(eState state) { m_state = state; }
   eState GetState() const { return m_state; }

private:
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

   enum eState m_state;
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
         const sGlobalObjDesc * pDesc = pGlobalObject->GetGlobalObjDesc();

         if (pDesc != NULL)
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
#ifndef NDEBUG
         else
         {
            char szGUID[kGuidStringLength];
            if (GUIDToString(iid, szGUID, _countof(szGUID)))
            {
               DebugMsg1("Global object %s does not describe itself\n", szGUID);
            }
            else
            {
               DebugMsg("Global object <UNKNOWN> does not describe itself\n");
            }
         }
#endif
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGlobalObjectRegistry::InitAll()
{
   Assert(GetState() == kPreInit);

   tResult result = S_OK; // @HACK: kind of cheesy to assume success

   typedef cDigraph<const IID *, IGlobalObject *> tInitGraph;
   tInitGraph initGraph;

   // add nodes
   tObjMap::iterator iter;
   for (iter = m_objMap.begin(); iter != m_objMap.end(); iter++)
   {
      const IID * pIID = iter->first;
      IUnknown * pUnk = iter->second;

      // If the object doesn't support IGlobalObject it shouldn't have got past
      // the Register step.
      IGlobalObject * pGlobalObj = NULL;
      Verify(SUCCEEDED(pUnk->QueryInterface(IID_IGlobalObject, (void**)&pGlobalObj)));
      initGraph.AddNode(pIID, pGlobalObj);

      // Not for the faint of heart. This release ensures that this registry holds
      // only a single reference to any object. The reference obtained by the QI in
      // the Register method will be the only one held by this registry.
      pGlobalObj->Release();
   }

   // add constraints as edges
   for (iter = m_objMap.begin(); iter != m_objMap.end(); iter++)
   {
      const IID * pIID = iter->first;
      IUnknown * pUnk = iter->second;

      cAutoIPtr<IGlobalObject> pGlobalObj;
      Verify(SUCCEEDED(pUnk->QueryInterface(IID_IGlobalObject, (void**)&pGlobalObj)));

      const sGlobalObjDesc * pDesc = pGlobalObj->GetGlobalObjDesc();
      Assert(pDesc != NULL);

      if (pDesc->constraints != NULL)
      {
         const sConstraint * pConstr;
         for (pConstr = pDesc->constraints; pConstr->pGUID != NULL; pConstr++)
         {
            IUnknown * pTargetUnk = NULL;
            const GUID * pTargetGUID = NULL;

            if (pConstr->against == kCA_Guid)
            {
               tObjMap::iterator found = m_objMap.find(pConstr->pGUID);
               if (found != m_objMap.end())
               {
                  pTargetUnk = found->second;
                  pTargetGUID = found->first;
               }
            }
            else if (pConstr->against == kCA_Name)
            {
               for (tObjMap::iterator iter = m_objMap.begin(); iter != m_objMap.end(); iter++)
               {
                  cAutoIPtr<IGlobalObject> pGlobalObj;
                  Verify(SUCCEEDED(iter->second->QueryInterface(IID_IGlobalObject, (void**)&pGlobalObj)));

                  const sGlobalObjDesc * pDesc = pGlobalObj->GetGlobalObjDesc();
                  Assert(pDesc != NULL);

                  if (strcmp(pDesc->name, pConstr->pszName) == 0)
                  {
                     pTargetUnk = iter->second;
                     pTargetGUID = iter->first;
                     break;
                  }
               }
            }

            if (pTargetUnk != NULL)
            {
#ifndef NDEBUG
               cAutoIPtr<IGlobalObject> pTargetGlobalObj;
               Verify(SUCCEEDED(pTargetUnk->QueryInterface(IID_IGlobalObject, (void**)&pTargetGlobalObj)));
               const sGlobalObjDesc * pTargetDesc = pTargetGlobalObj->GetGlobalObjDesc();
               Assert(pTargetDesc != NULL);
#endif

               // add edge
               if (pConstr->when == kCW_Before)
               {
                  LocalMsg2("%s initialized before %s\n", pDesc->name, pTargetDesc->name);
                  initGraph.AddEdge(pIID, pTargetGUID);
               }
               else if (pConstr->when == kCW_After)
               {
                  LocalMsg2("%s initialized after %s\n", pDesc->name, pTargetDesc->name);
                  initGraph.AddEdge(pTargetGUID, pIID);
               }
            }
#ifndef NDEBUG
            else
            {
               char szGUID[kGuidStringLength];
               GUIDToString(*(pConstr->pGUID), szGUID, _countof(szGUID));

               DebugMsg3("%s initialized %s non-existent object %s\n",
                  pDesc->name,
                  (pConstr->when == kCW_Before) ? "before" : "after",
                  (pConstr->against == kCA_Guid) ? szGUID : pConstr->pszName);
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

      LocalMsg1("Initializing global object %s\n", pGlobalObj->GetGlobalObjDesc()->name);

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
         LocalMsg1("Releasing global object %s\n", p->GetGlobalObjDesc()->name);
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
