///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scene.h"
#include "ray.h"
#include "inputapi.h"

#include "render.h"

#include "matrix4.h"
#include "connptimpl.h"

// TODO: HACK
#include <windows.h>
#include <GL/gl.h>

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

   std::for_each(m_inputListeners.begin(), m_inputListeners.end(), CTInterfaceMethodRef(&IUnknown::Release));
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
   std::for_each(m_entities.begin(), m_entities.end(), CTInterfaceMethodRef(&::IUnknown::Release));
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

class cRenderEntity
{
public:
   cRenderEntity(IRenderDevice * pRenderDevice);

   void operator()(ISceneEntity * pEntity);

private:
   cAutoIPtr<IRenderDevice> m_pRenderDevice;
};

cRenderEntity::cRenderEntity(IRenderDevice * pRenderDevice)
 : m_pRenderDevice(pRenderDevice)
{
   Assert(m_pRenderDevice != NULL);
   if (m_pRenderDevice != NULL)
   {
      m_pRenderDevice->AddRef();
   }
}

void cRenderEntity::operator()(ISceneEntity * pEntity)
{
   glPushMatrix();
   glMultMatrixf(pEntity->GetWorldTransform().m);
   pEntity->Render(m_pRenderDevice);
   glPopMatrix();
}

///////////////////////////////////////

tResult cSceneLayer::Render(IRenderDevice * pRenderDevice)
{
   std::for_each(m_entities.begin(), m_entities.end(), cRenderEntity(pRenderDevice));
   return S_OK;
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

bool cSceneLayer::HandleMouseEvent(int x, int y, uint mouseState, double time)
{
   tInputListeners::iterator iter;
   for (iter = m_inputListeners.begin(); iter != m_inputListeners.end(); iter++)
   {
      if ((*iter)->OnMouseEvent(x, y, mouseState, time))
      {
         return true;
      }
   }

   return false;
}

///////////////////////////////////////

bool cSceneLayer::HandleKeyEvent(long key, bool down, double time)
{
   tInputListeners::iterator iter;
   for (iter = m_inputListeners.begin(); iter != m_inputListeners.end(); iter++)
   {
      if ((*iter)->OnKeyEvent(key, down, time))
      {
         return true;
      }
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////
