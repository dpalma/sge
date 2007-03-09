///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIAPI_H
#define INCLUDED_GUIAPI_H

/// @file guiapi.h
/// Interface definitions for the GUI core system

#include "guidll.h"

#include "tech/comtools.h"

#include "guitypes.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE_GUID(IGUIElementRenderer, "3722802E-DFF2-468f-973F-3A7572827D72");
F_DECLARE_INTERFACE_GUID(IGUIElementEnum, "9B7A0950-4A35-496b-8BBB-C8090610A07F");
F_DECLARE_INTERFACE_GUID(IGUIEvent, "C9C4FA5D-F358-4e59-90C9-9424A27EA2AA");
F_DECLARE_INTERFACE_GUID(IGUIEventSounds, "B9C4F872-B530-4e59-B284-28C9867DF637");
F_DECLARE_INTERFACE_GUID(IGUIEventListener, "6F774FE2-FAB7-4097-9A9C-831BC8F0B2DE");
F_DECLARE_INTERFACE_GUID(IGUIDraggable, "35EA0C5D-D1C0-4492-812C-2E040F4A6642");
F_DECLARE_INTERFACE_GUID(IGUIFactory, "1E25154F-32DC-4e76-8F5B-1BF4E2F7150D");
F_DECLARE_INTERFACE_GUID(IGUIFactoryListener, "EBE8F62F-F5E6-4ba2-9DAE-6C7C20124478");
F_DECLARE_INTERFACE_GUID(IGUIContext, "EED2B148-43A2-4af0-B441-E187B335A8CD");

F_DECLARE_INTERFACE(IGUIElement);
F_DECLARE_INTERFACE(IGUIStyle);

F_DECLARE_INTERFACE(IRenderFont);

class TiXmlElement;


///////////////////////////////////////////////////////////////////////////////

GUI_API void GUILayoutRegisterBuiltInTypes();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIElementRenderer
//

typedef tResult (* tGUIRendererFactoryFn)(void * pReserved,
                                          IGUIElementRenderer * * ppRenderer);

enum eGUIElementRenderState
{
   kGUIElementRenderStateDefault    = 0,
   kGUIElementRenderStateMouseOver  = (1<<0),
   kGUIElementRenderStateArmed      = (1<<1),
   kGUIElementRenderStateFocus      = (1<<2),
};

interface IGUIElementRenderer : IUnknown
{
   virtual tResult Render(IGUIElement * pElement, const tGUIPoint & position, uint state) = 0;

   virtual tResult GetPreferredSize(IGUIElement * pElement, const tGUISize & parentSize, tGUISize * pSize) = 0;

   virtual tResult AllocateBorderSpace(IGUIElement * pElement, tGUIRect * pRect) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIElementEnum
//

interface IGUIElementEnum : IUnknown
{
   virtual tResult Next(ulong count, IGUIElement * * ppElements, ulong * pnElements) = 0;
   virtual tResult Skip(ulong count) = 0;
   virtual tResult Reset() = 0;
   virtual tResult Clone(IGUIElementEnum * * ppEnum) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIEvent
//

enum eGUIEventCode
{
   kGUIEventNone,
   kGUIEventFocus,
   kGUIEventBlur,
   kGUIEventMouseMove,
   kGUIEventMouseEnter,
   kGUIEventMouseLeave,
   kGUIEventMouseUp,
   kGUIEventMouseDown,
   kGUIEventMouseWheelUp,
   kGUIEventMouseWheelDown,
   kGUIEventKeyUp,
   kGUIEventKeyDown,
   kGUIEventClick,
   kGUIEventHover,
   kGUIEventDragStart,
   kGUIEventDragEnd,
   kGUIEventDragMove,
   kGUIEventDragOver,
   kGUIEventDrop,
};

typedef enum eGUIEventCode tGUIEventCode;

interface IGUIEvent : IUnknown
{
   virtual tResult GetEventCode(tGUIEventCode * pEventCode) = 0;
   virtual tResult GetMousePosition(tScreenPoint * pMousePos) = 0;
   virtual tResult GetKeyCode(long * pKeyCode) = 0;
   virtual tResult GetSourceElement(IGUIElement * * ppElement) = 0;

   virtual bool IsCancellable() const = 0;

   virtual tResult GetCancelBubble() = 0;
   virtual tResult SetCancelBubble(bool bCancel) = 0;

   virtual bool IsCtrlKeyDown() const = 0;
   virtual bool IsAltKeyDown() const = 0;
   virtual bool IsShiftKeyDown() const = 0;
};

///////////////////////////////////////

GUI_API tResult GUIEventCreate(tGUIEventCode eventCode,
                               tScreenPoint mousePos,
                               long keyCode,
                               int modifierKeys,
                               IGUIElement * pSource,
                               bool bCancellable,
                               IGUIEvent * * ppEvent);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIEventSounds
//

interface IGUIEventSounds : IUnknown
{
   virtual tResult SetEventSound(tGUIEventCode eventCode, const tGUIChar * pszSound) = 0;
   virtual tResult GetEventSound(tGUIEventCode eventCode, tGUIString * pSound) const = 0;
   virtual tResult ClearEventSound(tGUIEventCode eventCode) = 0;
   virtual void ClearAll() = 0;
};

///////////////////////////////////////

GUI_API tResult GUIEventSoundsCreate(const tGUIChar * pszScriptName = NULL);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIEventListener
//

interface IGUIEventListener : IUnknown
{
   virtual tResult OnEvent(IGUIEvent * pEvent) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIDraggable
//

interface IGUIDraggable : IUnknown
{
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIFactory
//

typedef tResult (* tGUIElementFactoryFn)(const TiXmlElement * pXmlElement,
                                         IGUIElement * pParent, IGUIElement * * ppElement);

///////////////////////////////////////

interface IGUIFactory : IUnknown
{
   virtual tResult AddFactoryListener(IGUIFactoryListener * pListener) = 0;
   virtual tResult RemoveFactoryListener(IGUIFactoryListener * pListener) = 0;

   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * pParent,
                                 IGUIElement * * ppElement) = 0;

   virtual tResult CreateRenderer(const tGUIChar * pszRendererClass,
                                  IGUIElementRenderer * * ppRenderer) = 0;

   virtual tResult RegisterElementFactory(const tGUIChar * pszType,
                                          tGUIElementFactoryFn pFactoryFn) = 0;
   virtual tResult RevokeElementFactory(const tGUIChar * pszType) = 0;

   virtual tResult RegisterRendererFactory(const tGUIChar * pszRenderer,
                                           tGUIRendererFactoryFn pFactoryFn) = 0;
   virtual tResult RevokeRendererFactory(const tGUIChar * pszRenderer) = 0;
};

///////////////////////////////////////

GUI_API tResult GUIFactoryCreate();

///////////////////////////////////////

GUI_API tResult GUIRegisterElementFactory(const tGUIChar * pszType,
                                          tGUIElementFactoryFn pFactoryFn);
GUI_API tResult GUIRegisterRendererFactory(const tGUIChar * pszRenderer,
                                           tGUIRendererFactoryFn pFactoryFn);

///////////////////////////////////////

#define REFERENCE_GUIFACTORY(type) \
   extern void * type##FactoryRefSym(); \
   void * MAKE_UNIQUE(g_pRefSym##type) = type##FactoryRefSym()

#define AUTOREGISTER_GUIFACTORYFN(type, factoryFn, registerFn) \
   void * type##FactoryRefSym() { return (void*)(&(factoryFn)); } \
   static tResult MAKE_UNIQUE(g_##type##AutoRegResult) = (*registerFn)(#type, factoryFn)

#define AUTOREGISTER_GUIELEMENTFACTORYFN(type, factoryFn) \
   AUTOREGISTER_GUIFACTORYFN(type, factoryFn, GUIRegisterElementFactory)

#define AUTOREGISTER_GUIRENDERERFACTORYFN(type, factoryFn) \
   AUTOREGISTER_GUIFACTORYFN(type, factoryFn, GUIRegisterRendererFactory)


////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIFactoryListener
//

interface IGUIFactoryListener : IUnknown
{
   virtual tResult PreCreateElement(const TiXmlElement * pXmlElement, IGUIElement * pParent) = 0;
   virtual void OnCreateElement(const TiXmlElement * pXmlElement, IGUIElement * pParent, IGUIElement * pElement) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIContext
//

enum eGUILayoutOptions
{
   kGUILayoutDefault       = 0,
   kGUILayoutNoMove        = (1 << 0),
   kGUILayoutNoSize        = (1 << 1),
};

interface IGUIContext : IUnknown
{
   /// @brief Show a modal dialog box
   /// @param pszDialog specifies an XML fragment or file name
   /// @return TODO
   virtual tResult ShowModalDialog(const tGUIChar * pszDialog) = 0;

   /// @brief Create GUI elements from XML specifications
   /// @param pszPage specifies either an XML fragment or file name
   /// @return S_OK, S_FALSE if no error and no elements loaded, or an E_xxx code
   virtual tResult PushPage(const tGUIChar * pszPage) = 0;
   virtual tResult PopPage() = 0;

   virtual tResult AddOverlayPage(const tGUIChar * pszPage) = 0;

   virtual tResult GetElementById(const tGUIChar * pszId, IGUIElement * * ppElement) = 0;

   virtual tResult GetOverlayElement(const tGUIChar * pszId, IGUIElement * * ppElement) = 0;

   virtual tResult RequestLayout(IGUIElement * pRequester, uint options) = 0;

   inline tResult RequestLayout(IGUIElement * pRequester)
   {
      return RequestLayout(pRequester, kGUILayoutDefault);
   }

   virtual tResult RenderGUI(uint width, uint height) = 0;

   virtual tResult ShowDebugInfo(const tGUIPoint & placement, IGUIStyle * pStyle) = 0;
   virtual tResult HideDebugInfo() = 0;

   virtual tResult GetDefaultFont(IRenderFont * * ppFont) = 0;

   virtual tResult AddEventListener(IGUIEventListener * pListener) = 0;
   virtual tResult RemoveEventListener(IGUIEventListener * pListener) = 0;
};

///////////////////////////////////////

GUI_API tResult GUIContextCreate(const tGUIChar * pszScriptName = NULL);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIAPI_H
