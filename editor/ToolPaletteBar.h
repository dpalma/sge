/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TOOLPALETTEBAR_H
#define INCLUDED_TOOLPALETTEBAR_H

#include "editorapi.h"
#include "editorTools.h"
#include "editorCtrlBars.h"

#include <vector>

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cButtonPanel
//

class cButtonPanel
{
public:
   cButtonPanel();
   ~cButtonPanel();

   void AddButton(CButton * pButton);

   void Clear();

   void Reposition(LPCRECT pRect, BOOL bRepaint = TRUE);

   void HandleClick(uint buttonId);

   void SetMargins(LPCRECT pMargins);

private:
   typedef std::vector<CButton *> tButtons;
   tButtons m_buttons;

   CRect m_margins;

   cAutoIPtr<cTerrainTileTool> m_pTool;
};

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPaletteBar
//

class cToolPaletteBar : public cDockingWindow,
                        public CMessageFilter,
                        public cComObject<IMPLEMENTS(IEditorTileManagerListener)>
{
   enum
   {
      kButtonIdFirst = 1000,
      kButtonIdLast = 1100,
   };

public:
   static tResult Factory(cDockingWindow * * ppDockingWindow);

   cToolPaletteBar();
   virtual ~cToolPaletteBar();

   virtual void OnDefaultTileSetChange(IEditorTileSet * pTileSet);

   BEGIN_MSG_MAP(cToolPaletteBar)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      MESSAGE_HANDLER(WM_SIZE, OnSize)
      COMMAND_RANGE_HANDLER(kButtonIdFirst, kButtonIdLast, OnButtonClicked)
      CHAIN_MSG_MAP(cDockingWindow)
   END_MSG_MAP()

   LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
   LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
   LRESULT OnButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);

	virtual BOOL PreTranslateMessage(MSG * pMsg);

   void ClearButtons();
   void RepositionButtons(BOOL bRepaint = TRUE);

private:
   cButtonPanel m_buttonPanel;

   CToolTipCtrl m_tooltip;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !INCLUDED_TOOLPALETTEBAR_H
