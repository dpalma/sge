///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GROUNDTILED_H
#define INCLUDED_GROUNDTILED_H

#include "sceneapi.h"

#ifdef _MSC_VER
#pragma once
#endif

class cHeightMap;

template <typename T> class cVec2;
typedef class cVec2<float> tVec2;

class cTiledGround;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainNode
//

class cTerrainNode : public cComObject<IMPLEMENTS(ISceneEntity)>
{
   cTerrainNode(const cTerrainNode &);
   const cTerrainNode & operator =(const cTerrainNode &);

public:
   cTerrainNode(cHeightMap * pHeightMap, const char * pszTexture);
   virtual ~cTerrainNode();

   float GetElevation(float nx, float nz) const;

   tVec2 GetDimensions() const;

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
   virtual float GetBoundingRadius() const { return 9999999; }
//   virtual float GetBoundingRadius() const { return m_pSceneEntity->GetBoundingRadius(); }

private:
   cAutoIPtr<ISceneEntity> m_pSceneEntity;
   cHeightMap * m_pHeightMap;
   cTiledGround * m_pGround;
};

///////////////////////////////////////

cTerrainNode * TerrainNodeCreate(const char * pszHeightData, float heightScale, const char * pszTexture);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GROUNDTILED_H
