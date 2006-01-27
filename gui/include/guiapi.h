///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIAPI_H
#define INCLUDED_GUIAPI_H

/// @file guiapi.h
/// Interface definitions for the GUI core system

#include "guidll.h"

#include "comtools.h"

#include "guitypes.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IGUIElement);
F_DECLARE_INTERFACE(IGUIStyle);
F_DECLARE_INTERFACE(IGUIFont);
F_DECLARE_INTERFACE(IGUIFontFactory);
F_DECLARE_INTERFACE(IGUIElementRenderer);
F_DECLARE_INTERFACE(IGUIElementEnum);
F_DECLARE_INTERFACE(IGUIEvent);
F_DECLARE_INTERFACE(IGUIEventSounds);
F_DECLARE_INTERFACE(IGUIEventRouter);
F_DECLARE_INTERFACE(IGUIEventListener);
F_DECLARE_INTERFACE(IGUIFactories);
F_DECLARE_INTERFACE(IGUIRenderDevice);
F_DECLARE_INTERFACE(IGUIRenderDeviceContext);
F_DECLARE_INTERFACE(IGUIContext);

class TiXmlElement;
class cGUIFontDesc;


///////////////////////////////////////////////////////////////////////////////

GUI_API void GUILayoutRegisterBuiltInTypes();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIFont
//

enum eGUIFontRenderTextFlags
{
   kRT_Center        = (1<<0),
   kRT_VCenter       = (1<<1),
   kRT_NoClip        = (1<<2),
   kRT_CalcRect      = (1<<3),
   kRT_SingleLine    = (1<<4),
   kRT_Bottom        = (1<<5),
   kRT_NoBlend       = (1<<6),
   kRT_DropShadow    = (1<<7),
};

interface IGUIFont : IUnknown
{
   virtual tResult RenderText(const char * pszText, int textLength, tRect * pRect,
                              uint flags, const cColor & color) const = 0;
   virtual tResult RenderText(const wchar_t * pszText, int textLength, tRect * pRect,
                              uint flags, const cColor & color) const = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIFontFactory
//

interface IGUIFontFactory : IUnknown
{
   virtual tResult CreateFontA(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont) = 0;
   virtual tResult CreateFontW(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont) = 0;
};

///////////////////////////////////////

#ifndef CreateFont
#ifdef _UNICODE
#define CreateFont CreateFontW
#else
#define CreateFont CreateFontA
#endif
#endif

///////////////////////////////////////

GUI_API void GUIFontFactoryCreate();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIElementRenderer
//

typedef tResult (* tGUIRendererFactoryFn)(void * pReserved,
                                          IGUIElementRenderer * * ppRenderer);

interface IGUIElementRenderer : IUnknown
{
   virtual tResult Render(IGUIElement * pElement, IGUIRenderDevice * pRenderDevice) = 0;

   virtual tResult GetPreferredSize(IGUIElement * pElement, tGUISize * pSize) = 0;

   virtual tResult ComputeClientArea(IGUIElement * pElement, tGUIRect * pClientArea) = 0;

   virtual tResult GetFont(IGUIElement * pElement, IGUIFont * * ppFont) = 0;
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
   kGUIEventDestroy,
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
   virtual tResult GetMousePosition(tGUIPoint * pMousePos) = 0;
   virtual tResult GetKeyCode(long * pKeyCode) = 0;
   virtual tResult GetSourceElement(IGUIElement * * ppElement) = 0;

   virtual bool IsCancellable() const = 0;

   virtual tResult GetCancelBubble() = 0;
   virtual tResult SetCancelBubble(bool bCancel) = 0;
};

///////////////////////////////////////

GUI_API tResult GUIEventCreate(tGUIEventCode eventCode, 
                               tGUIPoint mousePos, 
                               long keyCode, 
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
// INTERFACE: IGUIEventRouter
//

interface IGUIEventRouter : IUnknown
{
   virtual tResult AddEventListener(IGUIEventListener * pListener) = 0;
   virtual tResult RemoveEventListener(IGUIEventListener * pListener) = 0;

   virtual tResult GetFocus(IGUIElement * * ppElement) = 0;
   virtual tResult SetFocus(IGUIElement * pElement) = 0;
};


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
// INTERFACE: IGUIFactories
//

typedef tResult (* tGUIElementFactoryFn)(const TiXmlElement * pXmlElement,
                                         IGUIElement * pParent, IGUIElement * * ppElement);

///////////////////////////////////////

interface IGUIFactories : IUnknown
{
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

GUI_API tResult GUIFactoriesCreate();

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
// INTERFACE: IGUIRenderDevice
//
/// @interface IGUIRenderDevice
/// @brief Abstracts access to the platform 3D API (OpenGL, DirectX, etc.) for
/// the purpose of rendering GUI widgets
/// @see IGUIElementRenderer

interface IGUIRenderDevice : IUnknown
{
   virtual void PushScissorRect(const tGUIRect & rect) = 0;
   virtual void PopScissorRect() = 0;

   virtual void RenderSolidRect(const tGUIRect & rect, const tGUIColor & color) = 0;
   virtual void RenderBeveledRect(const tGUIRect & rect, int bevel,
                                  const tGUIColor & topLeft,
                                  const tGUIColor & bottomRight,
                                  const tGUIColor & face) = 0;

   virtual void FlushQueue() = 0;
};


////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIRenderDeviceContext
//
/// @interface IGUIRenderDeviceContext
/// @brief Provides fuller access to platform 3D API features than IGUIRenderDevice
/// @see IGUIRenderDevice
/// @see IGUIElementRenderer

interface IGUIRenderDeviceContext : IGUIRenderDevice
{
   virtual void Begin2D() = 0;
   virtual void End2D() = 0;

   virtual tResult GetViewportSize(uint * pWidth, uint * pHeight) = 0;
};

///////////////////////////////////////

GUI_API tResult GUIRenderDeviceCreateGL(IGUIRenderDeviceContext * * ppRenderDevice);
GUI_API tResult GUIRenderDeviceCreateD3D(IGUIRenderDeviceContext * * ppRenderDevice);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIContext
//

interface IGUIContext : IGUIEventRouter
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

   virtual tResult GetElementById(const tGUIChar * pszId, IGUIElement * * ppElement) = 0;

   virtual tResult GetElementsOfType(REFGUID iid, IGUIElementEnum * * ppEnum) const = 0;

   virtual tResult RenderGUI() = 0;

   virtual tResult GetRenderDeviceContext(IGUIRenderDeviceContext * * ppRenderDeviceContext) = 0;
   virtual tResult SetRenderDeviceContext(IGUIRenderDeviceContext * pRenderDeviceContext) = 0;

   virtual tResult GetDefaultFont(IGUIFont * * ppFont) = 0;

   virtual tResult ShowDebugInfo(const tGUIPoint & placement, IGUIStyle * pStyle) = 0;
   virtual tResult HideDebugInfo() = 0;
};

///////////////////////////////////////

GUI_API tResult GUIContextCreate(const tGUIChar * pszScriptName = NULL);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIAPI_H
