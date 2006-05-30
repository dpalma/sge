///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIELEMENTAPI_H
#define INCLUDED_GUIELEMENTAPI_H

/// @file guielementapi.h
/// Interface definitions for graphical user interface elements

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
F_DECLARE_INTERFACE(IGUIElementRenderer);
F_DECLARE_INTERFACE(IGUIElementEnum);
F_DECLARE_INTERFACE(IGUIEvent);
F_DECLARE_INTERFACE(IGUIContainerElement);
F_DECLARE_INTERFACE(IGUIPanelElement);
F_DECLARE_INTERFACE(IGUIDialogElement);
F_DECLARE_INTERFACE(IGUITitleBarElement);
F_DECLARE_INTERFACE(IGUIButtonElement);
F_DECLARE_INTERFACE(IGUILabelElement);
F_DECLARE_INTERFACE(IGUITextEditElement);
F_DECLARE_INTERFACE(IGUIScrollBarElement);
F_DECLARE_INTERFACE(IGUIListBoxElement);
F_DECLARE_INTERFACE(IGUIScriptElement);
F_DECLARE_INTERFACE(IGUILayoutManager);
F_DECLARE_INTERFACE(IGUIGridLayout);
F_DECLARE_INTERFACE(IGUIFlowLayout);

class TiXmlElement;


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIElement
//
/// @interface IGUIElement
/// @brief The base GUI element interface provides access to properties and 
/// methods common to all GUI widgets.

interface IGUIElement : IUnknown
{
   /// @return The semi-unique identifier assigned to this element
   virtual tResult GetId(tGUIString * pId) const = 0;
   virtual void SetId(const tGUIChar * pszId) = 0;

   /// @return A boolean value representing whether the element presently has input focus
   virtual bool HasFocus() const = 0;
   /// @brief Called by the GUI system to tell the element that it has input 
   /// focus and can render an indicator of that if desired
   /// @internal
   /// @remarks Calling this function will not actually obtain input focus.
   virtual void SetFocus(bool bFocus) = 0;

   /// @return A boolean value representing whether the mouse is presently over the element
   /// @remarks This method is intended for use by rendering code to display a highlight
   /// effect when the mouse is over the element.
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

   virtual tResult OnEvent(IGUIEvent * pEvent) = 0;

   virtual tResult GetRenderer(IGUIElementRenderer * * ppRenderer) = 0;
   virtual tResult SetRenderer(IGUIElementRenderer * pRenderer) = 0;

   virtual tResult GetStyle(IGUIStyle * * ppStyle) = 0;
   virtual tResult SetStyle(IGUIStyle * pStyle) = 0;

   virtual tResult EnumChildren(IGUIElementEnum * * ppChildren) = 0;

   virtual tResult GetClientArea(tGUIRect * pClientArea) = 0;
   virtual tResult SetClientArea(const tGUIRect & clientArea) = 0;

   virtual tResult ComputeClientArea(IGUIElementRenderer * pRenderer, tGUIRect * pClientArea) = 0;
};


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
// INTERFACE: IGUIContainerElement
//

interface IGUIContainerElement : IGUIElement
{
   virtual tResult AddElement(IGUIElement * pElement) = 0;
   virtual tResult RemoveElement(IGUIElement * pElement) = 0;
   virtual tResult RemoveAll() = 0;
   virtual tResult HasElement(IGUIElement * pElement) const = 0;

   virtual tResult GetLayout(IGUILayoutManager * * ppLayout) = 0;
   virtual tResult SetLayout(IGUILayoutManager * pLayout) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIPanelElement
//

interface IGUIPanelElement : IGUIContainerElement
{
};

////////////////////////////////////////

GUI_API tResult GUIPanelCreate(IGUIPanelElement * * ppPanelElement);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIDialogElement
//

interface IGUIDialogElement : IGUIContainerElement
{
   virtual tResult GetTitle(tGUIString * pTitle) = 0;
   virtual tResult SetTitle(const tGUIChar * pszTitle) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUITitleBarElement
//

interface IGUITitleBarElement : IGUIElement
{
   virtual tResult GetTitle(tGUIString * pTitle) = 0;
   virtual tResult SetTitle(const tGUIChar * pszTitle) = 0;
};

////////////////////////////////////////

GUI_API tResult GUITitleBarCreate(IGUITitleBarElement * * ppTitleBarElement);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIButtonElement
//

interface IGUIButtonElement : IGUIElement
{
   virtual bool IsArmed() const = 0;
   virtual void SetArmed(bool bArmed) = 0;

   virtual const tGUIChar * GetText() const = 0;
   virtual tResult GetText(tGUIString * pText) const = 0;
   virtual tResult SetText(const tGUIChar * pszText) = 0;

   virtual tResult GetOnClick(tGUIString * pOnClick) const = 0;
   virtual tResult SetOnClick(const tGUIChar * pszOnClick) = 0;
};

////////////////////////////////////////

GUI_API tResult GUIButtonCreate(IGUIButtonElement * * ppButtonElement);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUILabelElement
//

interface IGUILabelElement : IGUIElement
{
   virtual const tGUIChar * GetText() const = 0;
   virtual tResult GetText(tGUIString * pText) = 0;
   virtual tResult SetText(const tGUIChar * pszText) = 0;
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

   virtual const tGUIChar * GetText() const = 0;
   virtual tResult GetText(tGUIString * pText) = 0;
   virtual tResult SetText(const tGUIChar * pszText) = 0;

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

tResult GUIScrollBarElementCreate(eGUIScrollBarType scrollBarType, IGUIScrollBarElement * * ppScrollBar);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIListBoxElement
//

interface IGUIListBoxElement : IGUIElement
{
   virtual tResult AddItem(const tGUIChar * pszString, uint_ptr extra) = 0;
   virtual tResult RemoveItem(uint index) = 0;

   virtual tResult GetItemCount(uint * pItemCount) const = 0;
   virtual tResult GetItem(uint index, tGUIString * pString,
                           uint_ptr * pExtra, bool * pbIsSelected) const = 0;

   virtual const tGUIChar * GetItemText(uint index) const = 0;
   virtual bool IsItemSelected(uint index) const = 0;

   virtual tResult Clear() = 0;

   virtual tResult FindItem(const tGUIChar * pszString, uint * pIndex) const = 0;

   virtual tResult Select(uint startIndex, uint endIndex) = 0;
   virtual tResult SelectAll() = 0;

   virtual tResult Deselect(uint startIndex, uint endIndex) = 0;
   virtual tResult DeselectAll() = 0;

   virtual tResult GetSelectedCount(uint * pSelectedCount) const = 0;
   virtual tResult GetSelected(uint * pIndices, uint nMaxIndices) = 0;

   virtual tResult GetRowCount(uint * pRowCount) const = 0;
   virtual tResult SetRowCount(uint rowCount) = 0;

   virtual tResult GetVerticalScrollBar(IGUIScrollBarElement * * ppScrollBar) = 0;

   virtual tResult GetItemHeight(uint * pItemHeight) const = 0;
   virtual tResult SetItemHeight(uint itemHeight) = 0;

   virtual tResult GetOnSelChange(tGUIString * pOnSelChange) const = 0;
   virtual tResult SetOnSelChange(const tGUIChar * pszOnSelChange) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIScriptElement
//

interface IGUIScriptElement : IGUIElement
{
   virtual tResult GetScript(tGUIString * pScript) = 0;
   virtual tResult SetScript(const tGUIChar * pszScript) = 0;
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

GUI_API tResult GUILayoutManagerCreate(const TiXmlElement * pXmlElement,
                                       IGUILayoutManager * * ppLayout);

///////////////////////////////////////

typedef tResult (* tGUILayoutFactoryFn)(const TiXmlElement * pXmlElement,
                                        IGUILayoutManager * * ppLayout);
GUI_API tResult GUILayoutRegister(const tGUIChar * pszName, tGUILayoutFactoryFn pfn);


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
GUI_API tResult GUIGridLayoutCreate(uint columns, uint rows, uint hGap, uint vGap,
                                    IGUIGridLayout * * ppLayout);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIFlowLayout
//

interface IGUIFlowLayout : IGUILayoutManager
{
};

GUI_API IGUILayoutManager * GUIFlowLayoutCreate();


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIELEMENTAPI_H
