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

typedef dockwins::CDockingWindowTraits<dockwins::CVC6LikeCaption,
								WS_OVERLAPPEDWINDOW | WS_POPUP | /*WS_VISIBLE |*/
								WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_TOOLWINDOW> 
							 cDockingWindowTraits;

class cDockingWindow : 
	public dockwins::CTitleDockingWindowImpl<cDockingWindow, CWindow, cDockingWindowTraits>
{
	typedef dockwins::CTitleDockingWindowImpl<cDockingWindow, CWindow, cDockingWindowTraits> tBaseClass;

public:
   DECLARE_WND_CLASS(_T("DockingWindow"))

   BEGIN_MSG_MAP(cDockingWindow)
      CHAIN_MSG_MAP(tBaseClass)
	END_MSG_MAP()
};

typedef tResult (* tDockingWindowFactoryFn)(cDockingWindow * *);

enum eDockingWindowPlacement
{
   kDWP_Top,
   kDWP_Left,
   kDWP_Right,
   kDWP_Bottom,
   kDWP_Float,
};

tResult RegisterDockingWindow(uint titleStringId,
                              tDockingWindowFactoryFn pFactoryFn,
                              eDockingWindowPlacement placement);

void IterCtrlBarsBegin(HANDLE * phIter);
bool IterNextCtrlBar(HANDLE * phIter,
                     uint * pTitleStringId,
                     tDockingWindowFactoryFn * ppFactoryFn,
                     eDockingWindowPlacement * pPlacement);
void IterCtrlBarsEnd(HANDLE hIter);

struct sAutoRegisterDockingWindow
{
   sAutoRegisterDockingWindow(uint titleStringId,
                              tDockingWindowFactoryFn pFactoryFn,
                              eDockingWindowPlacement placement)
   {
      RegisterDockingWindow(titleStringId, pFactoryFn, placement);
   }
};

#define AUTO_REGISTER_DOCKINGWINDOW(titleStringId, factoryFn, placement) \
   static sAutoRegisterDockingWindow MAKE_UNIQUE(g_autoReg##titleStringId##DockWin)( \
      titleStringId, factoryFn, placement)

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORCTRLBARS_H)
