///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENEENTITY_H
#define INCLUDED_SCENEENTITY_H

#include "sceneapi.h"

#include "vec3.h"
#include "quat.h"
#include "matrix4.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneEntity
//

class cSceneEntity : public cComObject<IMPLEMENTS(ISceneEntity)>
{
   cSceneEntity(const cSceneEntity &);
   const cSceneEntity & operator =(const cSceneEntity &);

public:
   cSceneEntity();
   virtual ~cSceneEntity();

   virtual ISceneEntity * AccessParent();
   virtual tResult SetParent(ISceneEntity * pEntity);
   virtual tResult IsChild(ISceneEntity * pEntity) const;
   virtual tResult AddChild(ISceneEntity * pEntity);
   virtual tResult RemoveChild(ISceneEntity * pEntity);

   virtual const tVec3 & GetLocalTranslation() const;
   virtual void SetLocalTranslation(const tVec3 & translation);
   virtual const tQuat & GetLocalRotation() const;
   virtual void SetLocalRotation(const tQuat & rotation);
   virtual const tMatrix4 & GetLocalTransform() const;

   virtual const tVec3 & GetWorldTranslation() const;
   virtual const tQuat & GetWorldRotation() const;
   virtual const tMatrix4 & GetWorldTransform() const;

   virtual void Render(IRenderDevice * pRenderDevice);
   virtual float GetBoundingRadius() const;

private:
   ISceneEntity * m_pParent;
   tSceneEntityList m_children;

   tVec3 m_localTranslation;
   tQuat m_localRotation;

   mutable bool m_bHaveLocalTransform;
   mutable tMatrix4 m_localTransform;

   mutable tVec3 m_worldTranslation;
   mutable bool m_bHaveWorldTranslation;

   mutable tQuat m_worldRotation;
   mutable bool m_bHaveWorldRotation;

   mutable bool m_bHaveWorldTransform;
   mutable tMatrix4 m_worldTransform;
};

///////////////////////////////////////

inline const tVec3 & cSceneEntity::GetLocalTranslation() const
{
   return m_localTranslation;
}

///////////////////////////////////////

inline void cSceneEntity::SetLocalTranslation(const tVec3 & translation)
{
   m_localTranslation = translation;
   m_bHaveLocalTransform = false;
   m_bHaveWorldTranslation = false;
   m_bHaveWorldTransform = false;
}

///////////////////////////////////////

inline const tQuat & cSceneEntity::GetLocalRotation() const
{
   return m_localRotation;
}

///////////////////////////////////////

inline void cSceneEntity::SetLocalRotation(const tQuat & rotation)
{
   m_localRotation = rotation;
   m_bHaveLocalTransform = false;
   m_bHaveWorldRotation = false;
   m_bHaveWorldTransform = false;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENEENTITY_H
