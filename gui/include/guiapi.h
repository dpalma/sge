///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIAPI_H
#define INCLUDED_GUIAPI_H

/// @file guiapi.h
/// Interface definitions for graphical user interface elements plus the 
/// core system itself

#include "guidll.h"

#include "comtools.h"

#include "guitypes.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IGUIElement);
F_DECLARE_INTERFACE(IGUIStyle);
F_DECLARE_INTERFACE(IGUIStyleSheet);
F_DECLARE_INTERFACE(IGUIStyleElement);
F_DECLARE_INTERFACE(IGUIFont);
F_DECLARE_INTERFACE(IGUIFontFactory);
F_DECLARE_INTERFACE(IGUIElementRenderer);
F_DECLARE_INTERFACE(IGUIElementEnum);
F_DECLARE_INTERFACE(IGUIEvent);
F_DECLARE_INTERFACE(IGUIContainerElement);
F_DECLARE_INTERFACE(IGUILayoutManager);
F_DECLARE_INTERFACE(IGUIGridLayout);
F_DECLARE_INTERFACE(IGUIFlowLayout);
F_DECLARE_INTERFACE(IGUIPanelElement);
F_DECLARE_INTERFACE(IGUIDialogElement);
F_DECLARE_INTERFACE(IGUIButtonElement);
F_DECLARE_INTERFACE(IGUILabelElement);
F_DECLARE_INTERFACE(IGUITextEditElement);
F_DECLARE_INTERFACE(IGUIScrollBarElement);
F_DECLARE_INTERFACE(IGUIListBoxElement);
F_DECLARE_INTERFACE(IGUIScriptElement);
F_DECLARE_INTERFACE(IGUIEventRouter);
F_DECLARE_INTERFACE(IGUIEventListener);
F_DECLARE_INTERFACE(IGUIFactories);
F_DECLARE_INTERFACE(IGUIRenderDevice);
F_DECLARE_INTERFACE(IGUIRenderDeviceContext);
F_DECLARE_INTERFACE(IGUIContext);

class TiXmlElement;
class cGUIFontDesc;

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIElement
//
/// @interface IGUIElement
/// @brief The base GUI element interface provides access to properties and 
/// methods common to all GUI widgets.

typedef tResult (* tGUIElementFactoryFn)(const TiXmlElement * pXmlElement,
                                         IGUIElement * pParent, IGUIElement * * ppElement);

interface IGUIElement : IUnknown
{
   /// @return The semi-unique identifier assigned to this element
   virtual tResult GetId(tGUIString * pId) const = 0;
   virtual void SetId(const tChar * pszId) = 0;

   /// @return A boolean value representing whether the element presently has input focus
   virtual bool HasFocus() const = 0;
   /// @brief Called by the GUI system to tell the element that it has input 
   /// focus and can render an indicator of that if desired
   /// @internal
   /// @remarks Calling this function will not actually obtain input focus.
   virtual void SetFocus(bool bFocus) = 0;

   /// @return A boolean value representing whether the mouse is presently over the element
   virtual bool IsMouseOver() const = 0;
   /// @brief Called by the GUI system to tell the element that the mouse is 
   /// presently over it
   /// @internal
   virtual void SetMouseOver(bool bMouseOver) = 0;

   virtual bool IsVisible() const = 0;
   virtual void SetVisible(bool bVisible) = 0;

   virtual bool IsEnabled() const = 0;
   virtual void SetEnabled(bool bEnabled) = 0;

   virtual tResult GetParent(IGUIElement * * ppParent) = 0;
   virtual tResult SetParent(IGUIElement * pParent) = 0;

   /// @return The element's position relative to it's parent element
   /// @remarks Use GUIElementAbsolutePosition to calculate the element's 
   /// position in screen coordinates.
   /// @see GUIElementAbsolutePosition
   virtual tGUIPoint GetPosition() const = 0;
   /// @internal
   /// @remarks Element positions are set by the GUI system using parameters 
   /// from the style along with context information from the parent 
   /// element, if any. Use caution when calling SetPosition directly.
   virtual void SetPosition(const tGUIPoint & point) = 0;

   virtual tGUISize GetSize() const = 0;
   virtual void SetSize(const tGUISize & size) = 0;

   virtual bool Contains(const tGUIPoint & point) const = 0;

   virtual tResult OnEvent(IGUIEvent * pEvent) = 0;

   /// @brief The renderer "class" names the type of GUI element renderer object
   /// that will be instantiated for this element by default.
   /// @return The name of the default renderer for this type of GUI element
   /// @remarks Override the default renderer by instantiating an alternative
   /// one and using SetRenderer to replace the default.
   /// @see IGUIElementRenderer
   virtual tResult GetRendererClass(tGUIString * pRendererClass) = 0;
   virtual tResult SetRendererClass(const tChar * pszRendererClass) = 0;

   virtual tResult GetRenderer(IGUIElementRenderer * * ppRenderer) = 0;
   virtual tResult SetRenderer(IGUIElementRenderer * pRenderer) = 0;

   virtual tResult GetStyle(IGUIStyle * * ppStyle) = 0;
   virtual tResult SetStyle(IGUIStyle * pStyle) = 0;

   virtual tResult EnumChildren(IGUIElementEnum * * ppChildren) = 0;

   virtual tResult GetClientArea(tGUIRect * pClientArea) = 0;
   virtual tResult SetClientArea(const tGUIRect & clientArea) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIStyle
//
/// @interface IGUIStyle
/// @brief Contains the visual attributes, configured in XML, used to render an element.

enum eGUIAlignment
{
   kGUIAlignLeft,
   kGUIAlignRight,
   kGUIAlignCenter,
};

enum eGUIVerticalAlignment
{
   kGUIVertAlignTop,
   kGUIVertAlignBottom,
   kGUIVertAlignCenter,
};

enum eGUIDimensionSpec
{
   kGUIDimensionPixels,
   kGUIDimensionPercent,
};

interface IGUIStyle : IUnknown
{
   virtual tResult GetAttribute(const tChar * pszAttribute, tGUIString * pValue) = 0;
   virtual tResult GetAttribute(const tChar * pszAttribute, int * pValue) = 0;
   virtual tResult GetAttribute(const tChar * pszAttribute, tGUIColor * pValue) = 0;
   virtual tResult SetAttribute(const tChar * pszAttribute, const tChar * pszValue) = 0;
   virtual tResult SetAttribute(const tChar * pszAttribute, int value) = 0;

   virtual tResult GetAlignment(uint * pAlignment) = 0;
   virtual tResult SetAlignment(uint alignment) = 0;

   virtual tResult GetVerticalAlignment(uint * pAlignment) = 0;
   virtual tResult SetVerticalAlignment(uint alignment) = 0;

   virtual tResult GetBackgroundColor(tGUIColor * pColor) = 0;
   virtual tResult SetBackgroundColor(const tGUIColor & color) = 0;

   virtual tResult GetForegroundColor(tGUIColor * pColor) = 0;
   virtual tResult SetForegroundColor(const tGUIColor & color) = 0;

   virtual tResult GetTextAlignment(uint * pAlignment) = 0;
   virtual tResult SetTextAlignment(uint alignment) = 0;

   virtual tResult GetTextVerticalAlignment(uint * pAlignment) = 0;
   virtual tResult SetTextVerticalAlignment(uint alignment) = 0;

   virtual tResult GetFontName(tGUIString * pFontName) = 0;
   virtual tResult SetFontName(const tChar * pszFontName) = 0;

   virtual tResult GetFontPointSize(uint * pFontPointSize) = 0;
   virtual tResult SetFontPointSize(uint fontPointSize) = 0;

   virtual tResult GetFontBold(bool * pB) = 0;
   virtual tResult SetFontBold(bool b) = 0;

   virtual tResult GetFontItalic(bool * pB) = 0;
   virtual tResult SetFontItalic(bool b) = 0;

   virtual tResult GetFontShadow(bool * pB) = 0;
   virtual tResult SetFontShadow(bool b) = 0;

   virtual tResult GetFontOutline(bool * pB) = 0;
   virtual tResult SetFontOutline(bool b) = 0;

   virtual tResult GetFontDesc(cGUIFontDesc * pFontDesc) = 0;

   virtual tResult GetWidth(int * pWidth, uint * pSpec) = 0;
   virtual tResult SetWidth(int width, uint spec) = 0;

   virtual tResult GetHeight(int * pHeight, uint * pSpec) = 0;
   virtual tResult SetHeight(int height, uint spec) = 0;

   virtual tResult Clone(IGUIStyle * * ppStyle) = 0;
};

///////////////////////////////////////
/// Attempt to parse a color value from the given string. Valid strings are
/// of the form "(R,G,B,A)". The alpha component is optional. Color components 
/// can be in the range [0,255] or [0,1]. Certain standard colors are 
/// supported, too. For example, "red", "green", "blue", etc.

GUI_API tResult GUIParseColor(const tChar * pszColor, tGUIColor * pColor);

///////////////////////////////////////
/// Parse a CSS-like string to produce a GUI style object.

GUI_API tResult GUIStyleParse(const tChar * pszStyle, long length, IGUIStyle * * ppStyle);

GUI_API tResult GUIStyleParseInline(const tChar * pszStyle, long length, IGUIStyle * pClassStyle, IGUIStyle * * ppStyle);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIStyleSheet
//

interface IGUIStyleSheet : IUnknown
{
   virtual tResult AddRule(const tChar * pszSelector, IGUIStyle * pStyle) = 0;

   virtual tResult GetStyle(const tChar * pszType, const tChar * pszClass, IGUIStyle * * ppStyle) const = 0;
};

///////////////////////////////////////

GUI_API tResult GUIStyleSheetParse(const tChar * pszStyleSheet, IGUIStyleSheet * * ppStyleSheet);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIStyleElement
//

interface IGUIStyleElement : IGUIElement
{
   virtual tResult GetStyleSheet(IGUIStyleSheet * * ppStyleSheet) const = 0;
};


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

typedef tResult (* tGUIRendererFactoryFn)(void * pReserved, IGUIElementRenderer * * ppRenderer);

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
// INTERFACE: IGUIContainerElement
//

interface IGUIContainerElement : IGUIElement
{
   virtual tResult AddElement(IGUIElement * pElement) = 0;
   virtual tResult RemoveElement(IGUIElement * pElement) = 0;
   virtual tResult HasElement(IGUIElement * pElement) const = 0;
   virtual tResult GetElement(const tChar * pszId, IGUIElement * * ppElement) const = 0;

   virtual tResult GetLayout(IGUILayoutManager * * ppLayout) = 0;
   virtual tResult SetLayout(IGUILayoutManager * pLayout) = 0;

   virtual tResult GetInsets(tGUIInsets * pInsets) = 0;
   virtual tResult SetInsets(const tGUIInsets & insets) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUILayoutManager
//

interface IGUILayoutManager : IUnknown
{
   virtual tResult Layout(IGUIElement * pParent) = 0;
   virtual tResult GetPreferredSize(IGUIElement * pParent, tGUISize * pSize) = 0;

   virtual tResult GetHGap(uint * pHGap) = 0;
   virtual tResult SetHGap(uint hGap) = 0;

   virtual tResult GetVGap(uint * pVGap) = 0;
   virtual tResult SetVGap(uint vGap) = 0;
};

///////////////////////////////////////
// Create a layout manager from an XML fragment

GUI_API tResult GUILayoutManagerCreate(const TiXmlElement * pXmlElement, IGUILayoutManager * * ppLayout);

///////////////////////////////////////

typedef tResult (* tGUILayoutFactoryFn)(const TiXmlElement * pXmlElement, IGUILayoutManager * * ppLayout);
GUI_API tResult GUILayoutRegister(const tChar * pszName, tGUILayoutFactoryFn pfn);

GUI_API void GUILayoutRegisterBuiltInTypes();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIGridLayout
//

interface IGUIGridLayout : IGUILayoutManager
{
   virtual tResult GetColumns(uint * pColumns) = 0;
   virtual tResult SetColumns(uint columns) = 0;

   virtual tResult GetRows(uint * pRows) = 0;
   virtual tResult SetRows(uint rows) = 0;
};

///////////////////////////////////////

GUI_API tResult GUIGridLayoutCreate(IGUIGridLayout * * ppLayout);
GUI_API tResult GUIGridLayoutCreate(uint columns, uint rows, IGUIGridLayout * * ppLayout);
GUI_API tResult GUIGridLayoutCreate(uint columns, uint rows, uint hGap, uint vGap, IGUIGridLayout * * ppLayout);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIFlowLayout
//

interface IGUIFlowLayout : IGUILayoutManager
{
};

GUI_API IGUILayoutManager * GUIFlowLayoutCreate();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIPanelElement
//

interface IGUIPanelElement : IGUIContainerElement
{
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIDialogElement
//

interface IGUIDialogElement : IGUIContainerElement
{
   virtual tResult GetTitle(tGUIString * pTitle) = 0;
   virtual tResult SetTitle(const char * pszTitle) = 0;

   virtual tResult GetCaptionHeight(uint * pHeight) = 0;
   virtual tResult SetCaptionHeight(uint height) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIButtonElement
//

interface IGUIButtonElement : IGUIElement
{
   virtual bool IsArmed() const = 0;
   virtual void SetArmed(bool bArmed) = 0;

   virtual tResult GetText(tGUIString * pText) const = 0;
   virtual tResult SetText(const char * pszText) = 0;

   virtual tResult GetOnClick(tGUIString * pOnClick) const = 0;
   virtual tResult SetOnClick(const char * pszOnClick) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUILabelElement
//

interface IGUILabelElement : IGUIElement
{
   virtual tResult GetText(tGUIString * pText) = 0;
   virtual tResult SetText(const char * pszText) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUITextEditElement
//

interface IGUITextEditElement : IGUIElement
{
   // The number of characters that the element will be sized to accomodate
   virtual tResult GetEditSize(uint * pEditSize) = 0;
   virtual tResult SetEditSize(uint editSize) = 0;

   virtual tResult GetSelection(uint * pStart, uint * pEnd) = 0;
   virtual tResult SetSelection(uint start, uint end) = 0;

   virtual tResult GetText(tGUIString * pText) = 0;
   virtual tResult SetText(const char * pszText) = 0;

   virtual void UpdateBlinkingCursor() = 0;
   virtual bool ShowBlinkingCursor() const = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIScrollBarElement
//

enum eGUIScrollBarType
{
   kGUIScrollBarHorizontal,
   kGUIScrollBarVertical,
};

enum eGUIScrollBarPart
{
   kGUIScrollBarPartNone,
   kGUIScrollBarPartButton1,
   kGUIScrollBarPartButton2,
   kGUIScrollBarPartTrack1,
   kGUIScrollBarPartTrack2,
   kGUIScrollBarPartThumb,
};

interface IGUIScrollBarElement : IGUIElement
{
   virtual eGUIScrollBarType GetScrollBarType() = 0;

   virtual eGUIScrollBarPart GetArmedPart() = 0;
   virtual eGUIScrollBarPart GetMouseOverPart() = 0;

   virtual tResult GetPartRect(eGUIScrollBarPart part, tGUIRect * pRect) = 0;

   virtual tResult GetRange(int * pRangeMin, int * pRangeMax) = 0;
   virtual tResult SetRange(int rangeMin, int rangeMax) = 0;

   virtual tResult GetScrollPos(int * pScrollPos) = 0;
   virtual tResult SetScrollPos(int scrollPos) = 0;

   virtual tResult GetLineSize(int * pLineSize) = 0;
   virtual tResult SetLineSize(int lineSize) = 0;

   virtual tResult GetPageSize(int * pPageSize) = 0;
   virtual tResult SetPageSize(int pageSize) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIListBoxElement
//

interface IGUIListBoxElement : IGUIElement
{
   virtual tResult AddItem(const tChar * pszString, uint_ptr extra) = 0;
   virtual tResult RemoveItem(uint index) = 0;

   virtual tResult GetItemCount(uint * pItemCount) const = 0;
   virtual tResult GetItem(uint index, cStr * pString, uint_ptr * pExtra, bool * pbIsSelected) const = 0;

   virtual tResult Sort() = 0;

   virtual tResult Clear() = 0;

   virtual tResult FindItem(const tChar * pszString, uint * pIndex) const = 0;

   virtual tResult Select(uint startIndex, uint endIndex) = 0;
   virtual tResult SelectAll() = 0;

   virtual tResult Deselect(uint startIndex, uint endIndex) = 0;
   virtual tResult DeselectAll() = 0;

   virtual tResult GetSelectedCount(uint * pSelectedCount) const = 0;
   virtual tResult GetSelected(uint * pIndices, uint nMaxIndices) = 0;

   virtual tResult GetRowCount(uint * pRowCount) const = 0;
   virtual tResult SetRowCount(uint rowCount) = 0;

   virtual tResult GetScrollBar(eGUIScrollBarType scrollBarType, IGUIScrollBarElement * * ppScrollBar) = 0;

   virtual tResult GetItemHeight(uint * pItemHeight) const = 0;
   virtual tResult SetItemHeight(uint itemHeight) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIScriptElement
//

interface IGUIScriptElement : IGUIElement
{
   virtual tResult GetScript(tGUIString * pScript) = 0;
   virtual tResult SetScript(const tChar * pszScript) = 0;
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

interface IGUIFactories : IUnknown
{
   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * pParent, IGUIElement * * ppElement) = 0;

   virtual tResult CreateRenderer(const tChar * pszRendererClass, IGUIElementRenderer * * ppRenderer) = 0;

   virtual tResult RegisterElementFactory(const tChar * pszType, tGUIElementFactoryFn pFactoryFn) = 0;
   virtual tResult RevokeElementFactory(const tChar * pszType) = 0;

   virtual tResult RegisterRendererFactory(const tChar * pszRenderer, tGUIRendererFactoryFn pFactoryFn) = 0;
   virtual tResult RevokeRendererFactory(const tChar * pszRenderer) = 0;
};

///////////////////////////////////////

GUI_API tResult GUIFactoriesCreate();

///////////////////////////////////////

GUI_API tResult GUIRegisterElementFactory(const tChar * pszType, tGUIElementFactoryFn pFactoryFn);
GUI_API tResult GUIRegisterRendererFactory(const tChar * pszRenderer, tGUIRendererFactoryFn pFactoryFn);

///////////////////////////////////////

#define REFERENCE_GUIFACTORY(type) \
   extern void * type##FactoryRefSym(); \
   void * MAKE_UNIQUE(g_pRefSym##type) = type##FactoryRefSym()

#define AUTOREGISTER_GUIFACTORYFN(type, factoryFn, registerFn) \
   void * type##FactoryRefSym() { return factoryFn; } \
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
   virtual void RenderBeveledRect(const tGUIRect & rect, int bevel, const tGUIColor & topLeft,
                                  const tGUIColor & bottomRight, const tGUIColor & face) = 0;

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
   virtual tResult ShowModalDialog(const tChar * pszDialog) = 0;

   /// @brief Create GUI elements from XML specifications
   /// @param pszPage specifies either an XML fragment or file name
   /// @return S_OK, S_FALSE if no error and no elements loaded, or an E_xxx code
   virtual tResult PushPage(const tChar * pszPage) = 0;
   virtual tResult PopPage() = 0;

   virtual tResult GetElementById(const tChar * pszId, IGUIElement * * ppElement) = 0;

   virtual tResult GetElementsOfType(REFGUID iid, IGUIElementEnum * * ppEnum) const = 0;

   virtual tResult RenderGUI() = 0;

   virtual tResult GetRenderDeviceContext(IGUIRenderDeviceContext * * ppRenderDeviceContext) = 0;
   virtual tResult SetRenderDeviceContext(IGUIRenderDeviceContext * pRenderDeviceContext) = 0;

   virtual tResult GetDefaultFont(IGUIFont * * ppFont) = 0;

   virtual tResult ShowDebugInfo(const tGUIPoint & placement, IGUIStyle * pStyle) = 0;
   virtual tResult HideDebugInfo() = 0;
};

///////////////////////////////////////

GUI_API tResult GUIContextCreate(const tChar * pszScriptName = NULL);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIAPI_H
