///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "sceneentity.h"

#include <algorithm>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneEntity
//

ISceneEntity * SceneEntityCreate()
{
   return static_cast<ISceneEntity *>(new cSceneEntity);
}

///////////////////////////////////////

cSceneEntity::cSceneEntity()
 : m_pParent(NULL),
   m_localTranslation(0,0,0),
   m_localRotation(0,0,0,1),
   m_bHaveLocalTransform(false),
   m_bHaveWorldTranslation(false),
   m_bHaveWorldRotation(false),
   m_bHaveWorldTransform(false)
{
   m_localTransform.Identity();
}

///////////////////////////////////////

cSceneEntity::~cSceneEntity()
{
   std::for_each(m_children.begin(), m_children.end(), CTInterfaceMethodRef(&IUnknown::Release));
   m_children.clear();
}

///////////////////////////////////////

ISceneEntity * cSceneEntity::AccessParent()
{
   return m_pParent;
}

///////////////////////////////////////

tResult cSceneEntity::SetParent(ISceneEntity * pEntity)
{
   if (pEntity != NULL)
   {
      Assert(pEntity->IsChild(this) == S_OK);
   }
   m_pParent = pEntity;
   m_bHaveWorldTranslation = false;
   m_bHaveWorldRotation = false;
   m_bHaveWorldTransform = false;
   return S_OK;
}

///////////////////////////////////////

tResult cSceneEntity::IsChild(ISceneEntity * pEntity) const
{
   // TODO: Use some sort of lookup to do this, instead of a linear search
   Assert(pEntity != NULL);
   tSceneEntityList::const_iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      if (CTIsSameObject(*iter, pEntity))
      {
         return S_OK;
      }
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cSceneEntity::AddChild(ISceneEntity * pEntity)
{
   if (pEntity != NULL)
   {
      Assert(IsChild(pEntity) == S_FALSE);
      m_children.push_back(pEntity);
      pEntity->AddRef();
      pEntity->SetParent(this);
      return S_OK;
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cSceneEntity::RemoveChild(ISceneEntity * pEntity)
{
   Assert(pEntity != NULL);
   tSceneEntityList::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      if (CTIsSameObject(*iter, pEntity))
      {
         Assert(CTIsSameObject(pEntity->AccessParent(), this));
         pEntity->SetParent(NULL);
         m_children.erase(iter);
         return S_OK;
      }
   }
   return S_FALSE;
}

///////////////////////////////////////

const tMatrix4 & cSceneEntity::GetLocalTransform() const
{
   if (!m_bHaveLocalTransform)
   {
      const tVec3 & t = GetLocalTranslation();
      const tQuat & r = GetLocalRotation();

      tMatrix4 mt, mr;
      MatrixTranslate(t.x, t.y, t.z, &mt);
      r.ToMatrix(&mr);

      m_localTransform = mt * mr;

      m_bHaveLocalTransform = true;
   }

   return m_localTransform;
}

///////////////////////////////////////

const tVec3 & cSceneEntity::GetWorldTranslation() const
{
   if (!m_bHaveWorldTranslation)
   {
      if (m_pParent != NULL)
      {
         m_worldTranslation = GetLocalTranslation() + m_pParent->GetWorldTranslation();
      }
      else
      {
         m_worldTranslation = GetLocalTranslation();
      }

      m_bHaveWorldTranslation = true;
   }

   return m_worldTranslation;
}

///////////////////////////////////////

const tQuat & cSceneEntity::GetWorldRotation() const
{
   if (!m_bHaveWorldRotation)
   {
      if (m_pParent != NULL)
      {
         m_worldRotation = GetLocalRotation() * m_pParent->GetWorldRotation();
      }
      else
      {
         m_worldRotation = GetLocalRotation();
      }

      m_bHaveWorldRotation = true;
   }

   return m_worldRotation;
}

///////////////////////////////////////

const tMatrix4 & cSceneEntity::GetWorldTransform() const
{
   if (!m_bHaveWorldTransform)
   {
      const tVec3 & t = GetWorldTranslation();
      const tQuat & r = GetWorldRotation();

      tMatrix4 mt, mr;
      MatrixTranslate(t.x, t.y, t.z, &mt);
      r.ToMatrix(&mr);

      m_worldTransform = mt * mr;

      m_bHaveWorldTransform = true;
   }

   return m_worldTransform;
}

///////////////////////////////////////

void cSceneEntity::Render(IRenderDevice * pRenderDevice)
{
}

///////////////////////////////////////

float cSceneEntity::GetBoundingRadius() const
{
   return 0;
}

///////////////////////////////////////////////////////////////////////////////
