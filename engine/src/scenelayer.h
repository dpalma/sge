///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENELAYER_H
#define INCLUDED_SCENELAYER_H

#include "sceneapi.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneLayer
//

typedef std::list<ISceneEntity *> tSceneEntityList;

class cSceneLayer
{
public:
   cSceneLayer();
   ~cSceneLayer();

   tResult AddEntity(ISceneEntity * pEntity);
   tResult RemoveEntity(ISceneEntity * pEntity);
   tResult HasEntity(ISceneEntity * pEntity) const;

   tResult SetCamera(ISceneCamera * pCamera);
   tResult GetCamera(ISceneCamera * * ppCamera);

   void Clear();

   tResult Render(IRenderDevice * pRenderDevice);

   tResult Query(const cRay & ray, tSceneEntityList * pEntities);

private:
   tSceneEntityList m_entities;
   cAutoIPtr<ISceneCamera> m_pCamera;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENELAYER_H
