///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIAPI_H
#define INCLUDED_GUIAPI_H

#include "comtools.h"

#include "guitypes.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IGUIElement);
F_DECLARE_INTERFACE(IGUIElementFactory);
F_DECLARE_INTERFACE(IGUIContainerElement);
F_DECLARE_INTERFACE(IGUIPanelElement);
F_DECLARE_INTERFACE(IGUIButtonElement);
F_DECLARE_INTERFACE(IGUILabelElement);
F_DECLARE_INTERFACE(IGUIEventRouter);
F_DECLARE_INTERFACE(IGUIEventListener);
F_DECLARE_INTERFACE(IGUIFactory);
F_DECLARE_INTERFACE(IGUIContext);

F_DECLARE_INTERFACE(IRenderDevice);
class TiXmlElement;

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIElement
//

enum eGUIDimensionType
{
   kAbsolute,
   kRelative,
};

interface IGUIElement : IUnknown
{
   virtual const char * GetId() const = 0;
   virtual void SetId(const char * pszId) = 0;

   virtual bool HasFocus() const = 0;
   virtual void SetFocus(bool bFocus) = 0;

   virtual bool IsVisible() const = 0;
   virtual void SetVisible(bool bVisible) = 0;

   virtual bool IsEnabled() const = 0;
   virtual void SetEnabled(bool bEnabled) = 0;

   virtual tResult GetParent(IGUIElement * * ppParent) = 0;
   virtual tResult SetParent(IGUIElement * pParent) = 0;

   virtual tGUIPoint GetPosition() const = 0;
   virtual void SetPosition(const tGUIPoint & point) = 0;

   virtual tGUISize GetSize() const = 0;
   virtual void SetSize(const tGUISize & size) = 0;

   virtual bool Contains(const tGUIPoint & point) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIElementFactory
//

interface IGUIElementFactory : IUnknown
{
   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIContainerElement
//

interface IGUIContainerElement : IGUIElement
{
   virtual tResult AddElement(IGUIElement * pElement) = 0;
   virtual tResult RemoveElement(IGUIElement * pElement) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIPanelElement
//

interface IGUIPanelElement : IGUIContainerElement
{
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIButtonElement
//

interface IGUIButtonElement : IGUIElement
{
   virtual bool IsArmed() const = 0;
   virtual void SetArmed(bool bArmed) = 0;

   virtual bool IsMouseOver() const = 0;
   virtual void SetMouseOver(bool bMouseOver) = 0;

   virtual const char * GetText() const = 0;
   virtual void SetText(const char * pszText) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUILabelElement
//

interface IGUILabelElement : IGUIElement
{
   virtual const char * GetText() const = 0;
   virtual void SetText(const char * pszText) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIEventRouter
//

interface IGUIEventRouter : IUnknown
{
   virtual tResult AddEventListener(IGUIEventListener * pListener) = 0;
   virtual tResult RemoveEventListener(IGUIEventListener * pListener) = 0;

   virtual tResult GetFocus(IGUIElement * * ppElement) = 0;
   virtual tResult SetFocus(IGUIElement * pElement) = 0;

   virtual tResult GetCapture(IGUIElement * * ppElement) = 0;
   virtual tResult SetCapture(IGUIElement * pElement) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIEventListener
//

interface IGUIEventListener : IUnknown
{
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIFactory
//

interface IGUIFactory : IUnknown
{
   virtual tResult CreateElement(const char * pszType, const TiXmlElement * pXmlElement, IGUIElement * * ppElement) = 0;

   virtual tResult RegisterElementFactory(const char * pszType, IGUIElementFactory * pFactory) = 0;
   virtual tResult RevokeElementFactory(const char * pszType) = 0;
};

///////////////////////////////////////

void GUIFactoryCreate();

///////////////////////////////////////

tResult RegisterGUIElementFactory(const char * pszType, IGUIElementFactory * pFactory);

struct sAutoRegisterGUIElementFactory
{
   sAutoRegisterGUIElementFactory(const char * pszType, IGUIElementFactory * pFactory)
   {
      RegisterGUIElementFactory(pszType, pFactory);
      SafeRelease(pFactory);
   }
};

#define AUTOREGISTER_GUIELEMENTFACTORY(type, factoryClass) \
   static sAutoRegisterGUIElementFactory g_auto##type(#type, static_cast<IGUIElementFactory *>(new (factoryClass)))

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIContext
//

interface IGUIContext : IGUIEventRouter
{
   virtual tResult LoadFromResource(const char * psz) = 0;
   virtual tResult LoadFromString(const char * psz) = 0;
};

///////////////////////////////////////

void GUIContextCreate();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIAPI_H
