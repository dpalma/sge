///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorCtrlBars.h"

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////

struct sControlBarDesc
{
   uint titleStringId;
   CRuntimeClass * pRuntimeClass;
   eControlBarPlacement placement;
   struct sControlBarDesc * pNext;
};

static bool operator ==(const struct sControlBarDesc & cb1,
                        const struct sControlBarDesc & cb2)
{
   return
      (cb1.titleStringId == cb2.titleStringId) &&
      (cb1.pRuntimeClass == cb2.pRuntimeClass) &&
      (cb1.placement == cb2.placement);
}

struct sControlBarDesc * g_pCtrlBars = NULL;

struct sAutoDeleteCtrlBars
{
   ~sAutoDeleteCtrlBars();
};

sAutoDeleteCtrlBars::~sAutoDeleteCtrlBars()
{
   while (g_pCtrlBars != NULL)
   {
      struct sControlBarDesc * p = g_pCtrlBars;
      g_pCtrlBars = g_pCtrlBars->pNext;
      delete p;
   }
}

sAutoDeleteCtrlBars g_autoDeleteCtrlBars;

tResult RegisterControlBar(uint titleStringId,
                           CRuntimeClass * pRuntimeClass,
                           eControlBarPlacement placement)
{
   struct sControlBarDesc * pcb = new struct sControlBarDesc;
   pcb->titleStringId = titleStringId;
   pcb->pRuntimeClass = pRuntimeClass;
   pcb->placement = placement;
   pcb->pNext = g_pCtrlBars;
   g_pCtrlBars = pcb;
   return S_OK;
}

void IterCtrlBarsBegin(HANDLE * phIter)
{
   Assert(phIter != NULL);
   *phIter = (HANDLE)g_pCtrlBars;
}

bool IterNextCtrlBar(HANDLE * phIter,
                     uint * pTitleStringId,
                     CRuntimeClass * * ppRuntimeClass,
                     eControlBarPlacement * pPlacement)
{
   if (phIter != NULL && *phIter != NULL)
   {
      struct sControlBarDesc * & pCtrlBar = (struct sControlBarDesc * &)*phIter;

      if (pTitleStringId != NULL)
      {
         *pTitleStringId = pCtrlBar->titleStringId;
      }
      if (ppRuntimeClass != NULL)
      {
         *ppRuntimeClass = pCtrlBar->pRuntimeClass;
      }
      if (pPlacement != NULL)
      {
         *pPlacement = pCtrlBar->placement;
      }

      pCtrlBar = pCtrlBar->pNext;

      return true;
   }

   return false;
}

void IterCtrlBarsEnd(HANDLE hIter)
{
}

/////////////////////////////////////////////////////////////////////////////
