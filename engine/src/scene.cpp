///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scene.h"
#include "ray.h"

#include "render.h"

#include "matrix4.h"

// TODO: HACK
#include <windows.h>
#include <GL/gl.h>

#include <algorithm>
#include <functional>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cScene
//

///////////////////////////////////////

cScene::cScene()
{
}

///////////////////////////////////////

cScene::~cScene()
{
   Clear();
}

///////////////////////////////////////

tResult cScene::AddEntity(eSceneLayer layer, ISceneEntity * pEntity)
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

tResult cScene::RemoveEntity(eSceneLayer layer, ISceneEntity * pEntity)
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

tResult cScene::HasEntity(ISceneEntity * pEntity) const
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

void cScene::Clear(eSceneLayer layer)
{
   std::for_each(m_entities.begin(), m_entities.end(), CTInterfaceMethodRef(&::IUnknown::Release));
   m_entities.clear();
}

///////////////////////////////////////

void cScene::Clear()
{
   std::for_each(m_entities.begin(), m_entities.end(), CTInterfaceMethodRef(&::IUnknown::Release));
   m_entities.clear();
}

///////////////////////////////////////

void DoRender(ISceneEntity * pEntity)
{
   glPushMatrix();
   glMultMatrixf(pEntity->GetWorldTransform().m);
   pEntity->Render();
   glPopMatrix();
}

///////////////////////////////////////

tResult cScene::Render(IRenderDevice * pRenderDevice)
{
   std::for_each(m_entities.begin(), m_entities.end(), DoRender);
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

tResult cScene::Query(const cRay & ray, tSceneEntityList * pEntities)
{
   Assert(pEntities != NULL);
   std::for_each(m_entities.begin(), m_entities.end(), cRayTest(ray, pEntities));
   return pEntities->empty() ? S_FALSE : S_OK;
}

///////////////////////////////////////

void SceneCreate()
{
   cAutoIPtr<IScene>(new cScene);
}

///////////////////////////////////////////////////////////////////////////////
