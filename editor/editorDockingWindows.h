/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORDOCKINGWINDOWS_H)
#define INCLUDED_EDITORDOCKINGWINDOWS_H

#include <DockMisc.h>
#include <DockingWindow.h>
#include <ExtDockingWindow.h>

#include <vector>

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

/////////////////////////////////////////////////////////////////////////////

typedef tResult (* tDockingWindowFactoryFn)(cDockingWindow * *);

enum eDockingWindowPlacement
{
   kDWP_Top,
   kDWP_Left,
   kDWP_Right,
   kDWP_Bottom,
   kDWP_Float,
};

struct sDockingWindowDesc
{
   uint titleStringId;
   tDockingWindowFactoryFn pFactoryFn;
   eDockingWindowPlacement placement;
   SIZE defaultSize;
};

tResult RegisterDockingWindow(const sDockingWindowDesc & dwd);

struct sAutoRegisterDockingWindow
{
   sAutoRegisterDockingWindow(const sDockingWindowDesc & dwd)
   {
      RegisterDockingWindow(dwd);
   }
};

#define AUTO_REGISTER_DOCKINGWINDOW(titleStringId, factoryFn, placement) \
   AUTO_REGISTER_DOCKINGWINDOW_SIZED(titleStringId, factoryFn, placement, 100, 100)

#define AUTO_REGISTER_DOCKINGWINDOW_SIZED(titleStringId, factoryFn, placement, width, height) \
   static sDockingWindowDesc MAKE_UNIQUE(g_##titleStringId##DWDesc) = { titleStringId, factoryFn, placement, { width, height } }; \
   static sAutoRegisterDockingWindow MAKE_UNIQUE(g_##titleStringId##DWAuto)( \
      MAKE_UNIQUE(g_##titleStringId##DWDesc))

void GetDockingWindowDescs(std::vector<sDockingWindowDesc> * pDWDescs);

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORDOCKINGWINDOWS_H)
