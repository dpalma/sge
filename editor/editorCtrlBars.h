/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORCTRLBARS_H)
#define INCLUDED_EDITORCTRLBARS_H

#include <DockMisc.h>
#include <DockingWindow.h>
#include <ExtDockingWindow.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////

class cDockingWindow : 
	public dockwins::CTitleDockingWindowImpl<cDockingWindow, CWindow, dockwins::CVC6LikeTitleDockingWindowTraits>
{
	typedef dockwins::CTitleDockingWindowImpl<cDockingWindow, CWindow, dockwins::CVC6LikeTitleDockingWindowTraits> tBaseClass;

public:
   DECLARE_WND_CLASS(_T("cDockingWindow"))

   BEGIN_MSG_MAP(thisClass)
      CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()
};

typedef tResult (* tDockingWindowFactoryFn)(cDockingWindow * *);

enum eControlBarPlacement
{
   kCBP_Top,
   kCBP_Left,
   kCBP_Right,
   kCBP_Bottom,
   kCBP_Float,
};

tResult RegisterDockingWindow(uint titleStringId,
                              tDockingWindowFactoryFn pFactoryFn,
                              eControlBarPlacement placement);

void IterCtrlBarsBegin(HANDLE * phIter);
bool IterNextCtrlBar(HANDLE * phIter,
                     uint * pTitleStringId,
                     tDockingWindowFactoryFn * ppFactoryFn,
                     eControlBarPlacement * pPlacement);
void IterCtrlBarsEnd(HANDLE hIter);

struct sAutoRegisterDockingWindow
{
   sAutoRegisterDockingWindow(uint titleStringId,
                           tDockingWindowFactoryFn pFactoryFn,
                           eControlBarPlacement placement)
   {
      RegisterDockingWindow(titleStringId, pFactoryFn, placement);
   }
};

#define AUTO_REGISTER_CONTROLBAR(titleStringId, factoryFn, placement) \
   static sAutoRegisterDockingWindow MAKE_UNIQUE(g_autoReg##titleStringId##DockWin)( \
      titleStringId, factoryFn, placement)

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORCTRLBARS_H)
