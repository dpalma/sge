///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENE_H
#define INCLUDED_SCENE_H

#include "sceneapi.h"
#include "scenelayer.h"
#include "globalobj.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cScene
//

class cScene : public cGlobalObject<IMPLEMENTS(IScene)>
{
public:
   cScene();
   ~cScene();

   virtual tResult AddEntity(eSceneLayer layer, ISceneEntity * pEntity);
   virtual tResult RemoveEntity(eSceneLayer layer, ISceneEntity * pEntity);

   virtual tResult SetCamera(eSceneLayer layer, ISceneCamera * pCamera);
   virtual tResult GetCamera(eSceneLayer layer, ISceneCamera * * ppCamera);

   virtual void Clear(eSceneLayer layer);
   virtual void Clear();

   virtual tResult Render(IRenderDevice * pRenderDevice);

   virtual tResult Query(const cRay & ray, tSceneEntityList * pEntities);
   virtual tResult Query(const cRay & ray, ISceneEntityEnum * * ppEnum);

private:
   tSceneEntityList m_entities;
   cSceneLayer m_layers[kMAXSCENELAYERS];
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENE_H
