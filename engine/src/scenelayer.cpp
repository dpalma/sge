///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scenelayer.h"
#include "ray.h"
#include "frustum.h"
#include "inputapi.h"

#include "render.h"

#include "matrix4.h"
#include "connptimpl.h"

#include <algorithm>
#include <functional>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneLayer
//

///////////////////////////////////////

cSceneLayer::cSceneLayer()
{
}

///////////////////////////////////////

cSceneLayer::~cSceneLayer()
{
   Clear();

   std::for_each(m_inputListeners.begin(), m_inputListeners.end(), CTInterfaceMethod(&IUnknown::Release));
   m_inputListeners.clear();
}

///////////////////////////////////////

tResult cSceneLayer::AddEntity(ISceneEntity * pEntity)
{
   if (pEntity != NULL)
   {
      Assert(HasEntity(pEntity) == S_FALSE);
      m_entities.push_back(pEntity);
      pEntity->AddRef();
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cSceneLayer::RemoveEntity(ISceneEntity * pEntity)
{
   Assert(pEntity != NULL);
   tSceneEntityList::iterator iter;
   for (iter = m_entities.begin(); iter != m_entities.end(); iter++)
   {
      if (CTIsSameObject(*iter, pEntity))
      {
         (*iter)->Release();
         m_entities.erase(iter);
         return S_OK;
      }
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cSceneLayer::HasEntity(ISceneEntity * pEntity) const
{
   // TODO: Use some sort of lookup to do this, instead of a linear search
   Assert(pEntity != NULL);
   tSceneEntityList::const_iterator iter;
   for (iter = m_entities.begin(); iter != m_entities.end(); iter++)
   {
      if (CTIsSameObject(*iter, pEntity))
      {
         return S_OK;
      }
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cSceneLayer::SetCamera(ISceneCamera * pCamera)
{
   SafeRelease(m_pCamera);
   m_pCamera = pCamera;
   if (pCamera != NULL)
      pCamera->AddRef();
   return S_OK;
}

///////////////////////////////////////

tResult cSceneLayer::GetCamera(ISceneCamera * * ppCamera)
{
   if (ppCamera == NULL)
      return E_FAIL;
   if (!m_pCamera)
      return S_FALSE;
   *ppCamera = m_pCamera;
   m_pCamera->AddRef();
   return S_OK;
}

///////////////////////////////////////

void cSceneLayer::Clear()
{
   std::for_each(m_entities.begin(), m_entities.end(), CTInterfaceMethod(&::IUnknown::Release));
   m_entities.clear();
}

///////////////////////////////////////

tResult cSceneLayer::AddInputListener(IInputListener * pListener)
{
   return add_interface(m_inputListeners, pListener) ? S_OK : E_FAIL;
}

///////////////////////////////////////

tResult cSceneLayer::RemoveInputListener(IInputListener * pListener)
{
   return remove_interface(m_inputListeners, pListener) ? S_OK : E_FAIL;
}

///////////////////////////////////////

class cRayTest
{
   void operator delete(void *);
   const cRayTest & operator =(const cRayTest &);

public:
   cRayTest(const cRay & ray, tSceneEntityList * pEntities);

   void operator()(ISceneEntity * pEntity);

private:
   // this member can be a reference because this object is meant to
   // be used as a temporary within the scope of a function invocation
   const cRay & m_ray; 
   tSceneEntityList * m_pEntities;
};

///////////////////////////////////////

cRayTest::cRayTest(const cRay & ray, tSceneEntityList * pEntities)
 : m_ray(ray),
   m_pEntities(pEntities)
{
   Assert(pEntities != NULL);
}

///////////////////////////////////////

void cRayTest::operator()(ISceneEntity * pEntity)
{
   if (m_ray.IntersectsSphere(pEntity->GetWorldTranslation(), pEntity->GetBoundingRadius()))
   {
      pEntity->AddRef();
      m_pEntities->push_back(pEntity);
   }
}

///////////////////////////////////////

tResult cSceneLayer::Query(const cRay & ray, tSceneEntityList * pEntities)
{
   Assert(pEntities != NULL);
   std::for_each(m_entities.begin(), m_entities.end(), cRayTest(ray, pEntities));
   return pEntities->empty() ? S_FALSE : S_OK;
}

///////////////////////////////////////

class cFrustumCull
{
   void operator delete(void *);
   const cFrustumCull & operator =(const cFrustumCull &);

public:
   cFrustumCull(const cFrustum & frustum, tSceneEntityList * pEntities);

   void operator()(ISceneEntity * pEntity);

private:
   // this member can be a reference because this object is meant to
   // be used as a temporary within the scope of a function invocation
   const cFrustum & m_frustum; 
   tSceneEntityList * m_pEntities;
};

///////////////////////////////////////

cFrustumCull::cFrustumCull(const cFrustum & frustum, tSceneEntityList * pEntities)
 : m_frustum(frustum),
   m_pEntities(pEntities)
{
   Assert(pEntities != NULL);
}

///////////////////////////////////////

void cFrustumCull::operator()(ISceneEntity * pEntity)
{
   if (m_frustum.SphereInFrustum(pEntity->GetWorldTranslation(), pEntity->GetBoundingRadius()))
   {
      pEntity->AddRef();
      m_pEntities->push_back(pEntity);
   }
}

///////////////////////////////////////

void cSceneLayer::Cull(const cFrustum & frustum, tSceneEntityList * pEntities)
{
   Assert(pEntities != NULL);
   std::for_each(m_entities.begin(), m_entities.end(), cFrustumCull(frustum, pEntities));
}

///////////////////////////////////////

void cSceneLayer::GetAll(tSceneEntityList * pEntities)
{
   Assert(pEntities != NULL);
   pEntities->resize(m_entities.size());
   std::copy(m_entities.begin(), m_entities.end(), pEntities->begin());
   std::for_each(pEntities->begin(), pEntities->end(), CTInterfaceMethod(&::IUnknown::AddRef));
}

///////////////////////////////////////

bool cSceneLayer::HandleInputEvent(const sInputEvent * pEvent)
{
   tInputListeners::reverse_iterator iter;
   for (iter = m_inputListeners.rbegin(); iter != m_inputListeners.rend(); iter++)
   {
      if ((*iter)->OnInputEvent(pEvent))
      {
         return true;
      }
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////
