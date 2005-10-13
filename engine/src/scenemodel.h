///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENEMODEL_H
#define INCLUDED_SCENEMODEL_H

#include "sceneapi.h"
#include "simapi.h"
#include "model.h"

#include "techstring.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

class cModel;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneModel
//

class cSceneModel : public cComObject<IMPLEMENTS(ISceneEntity)>
{
   cSceneModel(const cSceneModel &);
   const cSceneModel & operator =(const cSceneModel &);

public:
   cSceneModel(const tChar * pszModel);
   virtual ~cSceneModel();

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

   virtual void Render(IRenderDevice * pRenderDevice);
   virtual float GetBoundingRadius() const { return m_pSceneEntity->GetBoundingRadius(); }

   virtual tResult Intersects(const cRay & ray) { return m_pSceneEntity->Intersects(ray); }

private:
   void Animate(double elapsedTime);

   class cSimClient : public cComObject<IMPLEMENTS(ISimClient)>
   {
      cSceneModel * m_pOuter;
   public:
      cSimClient(cSceneModel * pOuter);
      virtual void OnSimFrame(double elapsedTime);
   };
   friend class cSimClient;
   cSimClient m_simClient;

   cAutoIPtr<ISceneEntity> m_pSceneEntity;

   cStr m_model;
   cModel * m_pModel;
   double m_animationTime, m_animationLength;
   tBlendedVertices m_blendedVerts;
   std::vector<tMatrix4> m_blendMatrices;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENEMODEL_H
