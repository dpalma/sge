/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORCTRLBARS_H)
#define INCLUDED_EDITORCTRLBARS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////

enum eControlBarPlacement
{
   kCBP_Top,
   kCBP_Left,
   kCBP_Right,
   kCBP_Bottom,
   kCBP_Float,
};

tResult RegisterControlBar(uint titleStringId,
                           CRuntimeClass * pRuntimeClass,
                           eControlBarPlacement placement);

void IterCtrlBarsBegin(HANDLE * phIter);
bool IterNextCtrlBar(HANDLE * phIter,
                     uint * pTitleStringId,
                     CRuntimeClass * * ppRuntimeClass,
                     eControlBarPlacement * pPlacement);
void IterCtrlBarsEnd(HANDLE hIter);

struct sAutoRegisterControlBar
{
   sAutoRegisterControlBar(uint titleStringId,
                           CRuntimeClass * pRuntimeClass,
                           eControlBarPlacement placement)
   {
      RegisterControlBar(titleStringId, pRuntimeClass, placement);
   }
};

#define AUTO_REGISTER_CONTROLBAR(titleStringId, pRuntimeClass, placement) \
   static sAutoRegisterControlBar MAKE_UNIQUE(g_autoReg##titleStringId##CtrlBar)( \
      titleStringId, pRuntimeClass, placement)

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORCTRLBARS_H)
