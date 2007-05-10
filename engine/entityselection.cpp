///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entityselection.h"

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
   DeselectAll();

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

   pair<tEntitySet::iterator, bool> result = m_selected.insert(pEntity);
   if (result.second)
   {
      pEntity->AddRef();
      ForEachConnection(mem_fun(&IEntitySelectionListener::OnEntitySelectionChange));
      return S_OK;
   }
   else
   {
      return S_FALSE;
   }
}

///////////////////////////////////////

tResult cEntitySelection::SelectBoxed(const tAxisAlignedBox & box)
{
   UseGlobal(EntityManager);

   cAutoIPtr<IEnumEntities> pEnum;
   if (pEntityManager->BoxCast(box, &pEnum) == S_OK)
   {
      SetSelected(pEnum);
   }

   ForEachConnection(mem_fun(&IEntitySelectionListener::OnEntitySelectionChange));

   return m_selected.empty() ? S_FALSE : S_OK;
}

///////////////////////////////////////

tResult cEntitySelection::DeselectAll()
{
   if (m_selected.empty())
   {
      return S_FALSE;
   }
   for_each(m_selected.begin(), m_selected.end(), mem_fn(&IEntity::Release));
   m_selected.clear();
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

   for_each(m_selected.begin(), m_selected.end(), mem_fn(&IEntity::Release));
   m_selected.clear();

   IEntity * pEntities[32];
   ulong count = 0;

   while (SUCCEEDED(pEnum->Next(_countof(pEntities), &pEntities[0], &count)) && (count > 0))
   {
      for (ulong i = 0; i < count; i++)
      {
         pair<tEntitySet::iterator, bool> result = m_selected.insert(pEntities[i]);
         if (result.second)
         {
            pEntities[i]->AddRef();
         }

         SafeRelease(pEntities[i]);
      }

      count = 0;
   }

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

tResult cEntitySelection::IsSelected(IEntity * pEntity) const
{
   return (m_selected.find(pEntity) != m_selected.end()) ? S_OK : S_FALSE;
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
