///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENE_H
#define INCLUDED_SCENE_H

#include "sceneapi.h"
#include "globalobj.h"

#include <list>

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

   tResult HasEntity(ISceneEntity * pEntity) const;

   virtual void Clear(eSceneLayer layer);
   virtual void Clear();

   virtual tResult Render(IRenderDevice * pRenderDevice);

   virtual tResult Query(const cRay & ray, tSceneEntityList * pEntities);

private:
   tSceneEntityList m_entities;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENE_H
