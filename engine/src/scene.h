///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENE_H
#define INCLUDED_SCENE_H

#include "sceneapi.h"
#include "scenelayer.h"
#include "inputapi.h"
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

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult AddEntity(eSceneLayer layer, ISceneEntity * pEntity);
   virtual tResult RemoveEntity(eSceneLayer layer, ISceneEntity * pEntity);

   virtual tResult SetCamera(eSceneLayer layer, ISceneCamera * pCamera);
   virtual tResult GetCamera(eSceneLayer layer, ISceneCamera * * ppCamera);

   virtual void Clear(eSceneLayer layer);
   virtual void Clear();

   virtual tResult AddInputListener(eSceneLayer layer, IInputListener * pListener);
   virtual tResult RemoveInputListener(eSceneLayer layer, IInputListener * pListener);

   virtual tResult Render(IRenderDevice * pRenderDevice);

   virtual tResult Query(const cRay & ray, tSceneEntityList * pEntities);
   virtual tResult Query(const cRay & ray, ISceneEntityEnum * * ppEnum);

private:
   class cInputListener : public cComObject<cDefaultInputListener, &IID_IInputListener>
   {
      void CDECL operator delete(void *) { Assert(!"Should never be called"); }
   public:
      virtual bool OnMouseEvent(int x, int y, uint mouseState, double time);
      virtual bool OnKeyEvent(long key, bool down, double time);
      virtual bool OnInputEvent(const sInputEvent * pEvent);
   };

   friend class cInputListener;
   cInputListener m_inputListener;

   tSceneEntityList m_entities;
   cSceneLayer m_layers[kMAXSCENELAYERS];
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENE_H
