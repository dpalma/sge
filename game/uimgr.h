///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_UIMGR_H
#define INCLUDED_UIMGR_H

#include "ui.h"
#include "uiparse.h"
#include "sceneapi.h"
#include "inputapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIManager
//

class cUIManager : private cUIContainer, public cComObject<IMPLEMENTS(ISceneEntity)>
{
   typedef cUIContainer tContainerBase;

public:
   cUIManager();
   virtual ~cUIManager();

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

   ////////////////////////////////////

   void ShowModalDialog(const char * pszXmlFile);

   ////////////////////////////////////
   // cUIContainer/cUIComponent over-rides

   virtual cUISize GetSize() const;
   virtual bool OnEvent(const cUIEvent * pEvent, tUIResult * pResult);

private:
   void PreBubbleEvent(cUIComponent * pEventTarget, cUIEvent * pEvent);

   class cDialogParseHook : public cUIParseHook
   {
   public:
      cDialogParseHook();
      virtual ~cDialogParseHook();

      virtual eSkipResult SkipElement(const char * pszElement);

   private:
      int m_nDlgsSeen;
   };

   class cInputListener : public cComObject<cDefaultInputListener, &IID_IInputListener>
   {
      void operator delete(void *) { Assert(!"Should never be called"); }
   public:
      virtual bool OnKeyEvent(long key, bool down, double time);
      virtual bool OnInputEvent(const sInputEvent * pEvent);
   };

   friend class cInputListener;
   cInputListener m_inputListener;

   cUIComponent * m_pLastMouseOver;

   cAutoIPtr<ISceneEntity> m_pSceneEntity;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_UIMGR_H
