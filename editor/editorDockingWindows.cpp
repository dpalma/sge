///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorCtrlBars.h"

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////

struct sDockingWindowDesc
{
   uint titleStringId;
   tDockingWindowFactoryFn pFactoryFn;
   eDockingWindowPlacement placement;
   struct sDockingWindowDesc * pNext;
};

static bool operator ==(const struct sDockingWindowDesc & cb1,
                        const struct sDockingWindowDesc & cb2)
{
   return
      (cb1.titleStringId == cb2.titleStringId) &&
      (cb1.pFactoryFn == cb2.pFactoryFn) &&
      (cb1.placement == cb2.placement);
}

/////////////////////////////////////////////////////////////////////////////

struct sDockingWindowDesc * g_pDockingWindowDescs = NULL;

struct sAutoDeleteDockingWindowDescs
{
   ~sAutoDeleteDockingWindowDescs();
};

sAutoDeleteDockingWindowDescs::~sAutoDeleteDockingWindowDescs()
{
   while (g_pDockingWindowDescs != NULL)
   {
      struct sDockingWindowDesc * p = g_pDockingWindowDescs;
      g_pDockingWindowDescs = g_pDockingWindowDescs->pNext;
      delete p;
   }
}

sAutoDeleteDockingWindowDescs g_autoDeleteDockingWindowDescs;

/////////////////////////////////////////////////////////////////////////////

tResult RegisterDockingWindow(uint titleStringId,
                              tDockingWindowFactoryFn pFactoryFn,
                              eDockingWindowPlacement placement)
{
   struct sDockingWindowDesc * pcb = new struct sDockingWindowDesc;
   pcb->titleStringId = titleStringId;
   pcb->pFactoryFn = pFactoryFn;
   pcb->placement = placement;
   pcb->pNext = g_pDockingWindowDescs;
   g_pDockingWindowDescs = pcb;
   return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

void IterCtrlBarsBegin(HANDLE * phIter)
{
   Assert(phIter != NULL);
   *phIter = (HANDLE)g_pDockingWindowDescs;
}

bool IterNextCtrlBar(HANDLE * phIter,
                     uint * pTitleStringId,
                     tDockingWindowFactoryFn * ppFactoryFn,
                     eDockingWindowPlacement * pPlacement)
{
   if (phIter != NULL && *phIter != NULL)
   {
      struct sDockingWindowDesc * & pCtrlBar = (struct sDockingWindowDesc * &)*phIter;

      if (pTitleStringId != NULL)
      {
         *pTitleStringId = pCtrlBar->titleStringId;
      }
      if (ppFactoryFn != NULL)
      {
         *ppFactoryFn = pCtrlBar->pFactoryFn;
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
