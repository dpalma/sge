///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENEMESH_H
#define INCLUDED_SCENEMESH_H

#include "sceneapi.h"
#include "sim.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IMesh);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneMesh
//

class cSceneMesh : public cComObject<IMPLEMENTS(ISceneEntity)>
{
   cSceneMesh(const cSceneMesh &);
   const cSceneMesh & operator =(const cSceneMesh &);

public:
   cSceneMesh();
   virtual ~cSceneMesh();

   virtual ISceneEntity * AccessParent() { return m_pSceneEntity->AccessParent(); }
   virtual tResult SetParent(ISceneEntity * pEntity) { return m_pSceneEntity->SetParent(pEntity); }
   virtual tResult IsChild(ISceneEntity * pEntity) const { return m_pSceneEntity->IsChild(pEntity); }
   virtual tResult AddChild(ISceneEntity * pEntity) { return m_pSceneEntity->AddChild(pEntity); }
   virtual tResult RemoveChild(ISceneEntity * pEntity) { return m_pSceneEntity->RemoveChild(pEntity); }

   virtual const tVec3 & GetLocalTranslation() const { return m_pSceneEntity->GetLocalTranslation(); }
   virtual void SetLocalTranslation(const tVec3 & translation) { m_pSceneEntity->SetLocalTranslation(translation); }
   virtual const tQuat & GetLocalRotation() const { return m_pSceneEntity->GetLocalRotation(); }
   virtual void SetLocalRotation(const tQuat & rotation) { m_pSceneEntity->SetLocalRotation(rotation); }
   virtual const tMatrix4 & GetLocalTransform() const { return m_pSceneEntity->GetLocalTransform(); }

   virtual const tVec3 & GetWorldTranslation() const { return m_pSceneEntity->GetWorldTranslation(); }
   virtual const tQuat & GetWorldRotation() const { return m_pSceneEntity->GetWorldRotation(); }
   virtual const tMatrix4 & GetWorldTransform() const { return m_pSceneEntity->GetWorldTransform(); }

   virtual void Render();
   virtual float GetBoundingRadius() const;

   bool SetMesh(const char * pszMesh);
   IMesh * AccessMesh();

private:
   tResult PostRead();

   void Animate(double elapsedTime);

   class cSimClient : public cComObject<IMPLEMENTS(ISimClient)>
   {
      void CDECL operator delete(void *) { Assert(!"Should never be called"); }
   public:
      virtual void DeleteThis() { /* do not delete */ }
      virtual void OnFrame(double elapsedTime);
   };

   friend class cSimClient;
   cSimClient m_simClient;

   cAutoIPtr<ISceneEntity> m_pSceneEntity;

   cAutoIPtr<IMesh> m_pMesh;
   float m_boundingSphereRadius;

   std::vector<tMatrix4> m_boneMatrices;
   float m_animationTime;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENEMESH_H
