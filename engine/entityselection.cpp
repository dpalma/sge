///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entityselection.h"

#include "tech/comenumutil.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#define BOOST_MEM_FN_ENABLE_STDCALL
#include <boost/mem_fn.hpp>

#include <algorithm>

#include "tech/dbgalloc.h" // must be last header

using namespace boost;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntitySelection
//

///////////////////////////////////////

cEntitySelection::cEntitySelection()
{
}

///////////////////////////////////////

cEntitySelection::~cEntitySelection()
{
   Clear();
}

///////////////////////////////////////

tResult cEntitySelection::Init()
{
   UseGlobal(EntityManager);
   pEntityManager->AddEntityManagerListener(static_cast<IEntityManagerListener*>(this));

   return S_OK;
}

///////////////////////////////////////

tResult cEntitySelection::Term()
{
   Clear();

   UseGlobal(EntityManager);
   pEntityManager->RemoveEntityManagerListener(static_cast<IEntityManagerListener*>(this));

   return S_OK;
}

///////////////////////////////////////

tResult cEntitySelection::AddEntitySelectionListener(IEntitySelectionListener * pListener)
{
   return Connect(pListener);
}

///////////////////////////////////////

tResult cEntitySelection::RemoveEntitySelectionListener(IEntitySelectionListener * pListener)
{
   return Disconnect(pListener);
}

///////////////////////////////////////

tResult cEntitySelection::Select(IEntity * pEntity)
{
   if (pEntity == NULL)
   {
      return E_POINTER;
   }

   if (Insert(pEntity))
   {
      ForEachConnection(mem_fun(&IEntitySelectionListener::OnEntitySelectionChange));
      return S_OK;
   }
   else
   {
      return S_FALSE;
   }
}

///////////////////////////////////////

tResult cEntitySelection::Deselect(IEntity * pEntity)
{
   tEntitySet::iterator f = m_selected.find(pEntity);
   if (f == m_selected.end())
   {
      return S_FALSE;
   }
   SafeRelease(*f);
   m_selected.erase(f);
   return S_OK;
}

///////////////////////////////////////

void cEntitySelection::ToggleSelect(IEntity * pEntity)
{
   tEntitySet::iterator f = m_selected.find(pEntity);
   if (f == m_selected.end())
   {
      Insert(pEntity);
   }
   else
   {
      SafeRelease(*f);
      m_selected.erase(f);
   }
}

///////////////////////////////////////

tResult cEntitySelection::IsSelected(IEntity * pEntity) const
{
   return (m_selected.find(pEntity) != m_selected.end()) ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cEntitySelection::DeselectAll()
{
   if (m_selected.empty())
   {
      return S_FALSE;
   }
   Clear();
   ForEachConnection(mem_fun(&IEntitySelectionListener::OnEntitySelectionChange));
   return S_OK;
}

///////////////////////////////////////

uint cEntitySelection::GetSelectedCount() const
{
   return m_selected.size();
}

///////////////////////////////////////

tResult cEntitySelection::SetSelected(IEnumEntities * pEnum)
{
   if (pEnum == NULL)
   {
      return E_POINTER;
   }

   Clear();

   ForEach<IEnumEntities, IEntity>(pEnum, bind1st(mem_fun(&cEntitySelection::Insert), this));

   ForEachConnection(mem_fun(&IEntitySelectionListener::OnEntitySelectionChange));

   return S_OK;
}

///////////////////////////////////////

tResult cEntitySelection::GetSelected(IEnumEntities * * ppEnum) const
{
   if (ppEnum == NULL)
   {
      return E_POINTER;
   }
   if (m_selected.empty())
   {
      return S_FALSE;
   }
   return tEntitySetEnum::Create(m_selected, ppEnum);
}

///////////////////////////////////////

void cEntitySelection::OnRemoveEntity(IEntity * pEntity)
{
   size_t nErasedFromSelected = m_selected.erase(pEntity);
   while (nErasedFromSelected-- > 0)
   {
      pEntity->Release();
   }
}

///////////////////////////////////////

bool cEntitySelection::Insert(IEntity * pEntity)
{
   pair<tEntitySet::iterator, bool> result = m_selected.insert(pEntity);
   if (result.second)
   {
      pEntity->AddRef();
   }
   return result.second;
}

///////////////////////////////////////

void cEntitySelection::Clear()
{
   for_each(m_selected.begin(), m_selected.end(), mem_fn(&IEntity::Release));
   m_selected.clear();
}

///////////////////////////////////////

tResult EntitySelectionCreate()
{
   cAutoIPtr<IEntitySelection> pEntitySelection(static_cast<IEntitySelection*>(new cEntitySelection));
   if (!pEntitySelection)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IEntitySelection, pEntitySelection);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

extern tResult TestEntityCreate(IEntity * * ppEntity);

TEST(SingleEntitySelection)
{
   cAutoIPtr<IEntitySelection> pEntitySelection(static_cast<IEntitySelection*>(new cEntitySelection));

   cAutoIPtr<IEntity> pEntity;
   CHECK_EQUAL(S_OK, TestEntityCreate(&pEntity));

   CHECK_EQUAL(S_OK, pEntitySelection->Select(pEntity));
   CHECK_EQUAL(S_FALSE, pEntitySelection->Select(pEntity));
   CHECK_EQUAL(1, pEntitySelection->GetSelectedCount());
   CHECK_EQUAL(S_OK, pEntitySelection->DeselectAll());
   CHECK_EQUAL(S_FALSE, pEntitySelection->DeselectAll());
   CHECK_EQUAL(0, pEntitySelection->GetSelectedCount());
   CHECK_EQUAL(S_FALSE, pEntitySelection->Deselect(pEntity));
   CHECK_EQUAL(S_OK, pEntitySelection->Select(pEntity));
   CHECK_EQUAL(1, pEntitySelection->GetSelectedCount());
   CHECK_EQUAL(S_OK, pEntitySelection->Deselect(pEntity));
   CHECK_EQUAL(S_FALSE, pEntitySelection->Deselect(pEntity));
}

TEST(ToggleEntitySelect)
{
   cAutoIPtr<IEntitySelection> pEntitySelection(static_cast<IEntitySelection*>(new cEntitySelection));

   cAutoIPtr<IEntity> pEntity;
   CHECK_EQUAL(S_OK, TestEntityCreate(&pEntity));

   CHECK_EQUAL(0, pEntitySelection->GetSelectedCount());
   pEntitySelection->ToggleSelect(pEntity);
   CHECK_EQUAL(1, pEntitySelection->GetSelectedCount());
   pEntitySelection->ToggleSelect(pEntity);
   CHECK_EQUAL(0, pEntitySelection->GetSelectedCount());
}

TEST(SameEntityPointerAddedTwice)
{
   cAutoIPtr<IEntitySelection> pEntitySelection(static_cast<IEntitySelection*>(new cEntitySelection));

   cAutoIPtr<IEntity> pEntity;
   CHECK_EQUAL(S_OK, TestEntityCreate(&pEntity));

   CHECK_EQUAL(S_OK, pEntitySelection->Select(pEntity));
   CHECK_EQUAL(S_FALSE, pEntitySelection->Select(pEntity));

   // TODO: Test via SetSelected
}

TEST(GetSetSelectedEntitiesByEnum)
{
   cAutoIPtr<IEntitySelection> pEntitySelection(static_cast<IEntitySelection*>(new cEntitySelection));

   static const int nTestEntities = 10;

   for (int i = 0; i < nTestEntities; ++i)
   {
      cAutoIPtr<IEntity> pEntity;
      CHECK_EQUAL(S_OK, TestEntityCreate(&pEntity));
      CHECK_EQUAL(S_OK, pEntitySelection->Select(pEntity));
   }

   CHECK_EQUAL(nTestEntities, pEntitySelection->GetSelectedCount());

   cAutoIPtr<IEnumEntities> pEnum;
   CHECK_EQUAL(S_OK, pEntitySelection->GetSelected(&pEnum));

   CHECK_EQUAL(S_OK, pEntitySelection->DeselectAll());
   CHECK_EQUAL(0, pEntitySelection->GetSelectedCount());

   CHECK_EQUAL(S_OK, pEntitySelection->SetSelected(pEnum));

   CHECK_EQUAL(nTestEntities, pEntitySelection->GetSelectedCount());
}

//TEST(SelectionIndicatorInstallation)
//{
//   cAutoIPtr<IEntitySelection> pEntitySelection(static_cast<IEntitySelection*>(new cEntitySelection));
//
//   cAutoIPtr<IEntity> pEntity;
//   CHECK_EQUAL(S_OK, TestEntityCreate(&pEntity));
//
//   cAutoIPtr<IEntityComponent> pComponent;
//   CHECK_EQUAL(S_FALSE, pEntity->GetComponent(IEntityBoxSelectionIndicatorComponent::CID, &pComponent));
//
//   CHECK_EQUAL(S_OK, pEntitySelection->Select(pEntity));
//
//   CHECK_EQUAL(S_OK, pEntity->GetComponent(IEntityBoxSelectionIndicatorComponent::CID, &pComponent));
//   SafeRelease(pComponent);
//
//   CHECK_EQUAL(S_OK, pEntitySelection->DeselectAll());
//
//   CHECK_EQUAL(S_FALSE, pEntity->GetComponent(IEntityBoxSelectionIndicatorComponent::CID, &pComponent));
//}

#endif

///////////////////////////////////////////////////////////////////////////////
