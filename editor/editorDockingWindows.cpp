///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDockingWindows.h"

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////

struct sDockingWindowDescNode : public sDockingWindowDesc
{
   struct sDockingWindowDescNode * pNext;
};

static bool operator ==(const struct sDockingWindowDesc & cb1,
                        const struct sDockingWindowDesc & cb2)
{
   return
      (cb1.titleStringId == cb2.titleStringId) &&
      (cb1.pFactoryFn == cb2.pFactoryFn) &&
      (cb1.placement == cb2.placement) &&
      (cb1.defaultSize.cx == cb2.defaultSize.cx) &&
      (cb1.defaultSize.cy == cb2.defaultSize.cy);
}

/////////////////////////////////////////////////////////////////////////////

struct sDockingWindowDescNode * g_pDockingWindowDescs = NULL;

static class cAutoDeleteDockingWindowDescs
{
public:
   ~cAutoDeleteDockingWindowDescs()
   {
      while (g_pDockingWindowDescs != NULL)
      {
         struct sDockingWindowDescNode * p = g_pDockingWindowDescs;
         g_pDockingWindowDescs = g_pDockingWindowDescs->pNext;
         delete p;
      }
   }
} g_autoDeleteDockingWindowDescs;

/////////////////////////////////////////////////////////////////////////////

tResult RegisterDockingWindow(const sDockingWindowDesc & dwd)
{
   struct sDockingWindowDescNode * pDWDN = new struct sDockingWindowDescNode;
   if (pDWDN == NULL)
   {
      return E_OUTOFMEMORY;
   }
   static_cast<struct sDockingWindowDesc &>(*pDWDN) = dwd;
   pDWDN->pNext = g_pDockingWindowDescs;
   g_pDockingWindowDescs = pDWDN;
   return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

void GetDockingWindowDescs(std::vector<sDockingWindowDesc> * pDWDescs)
{
   if (pDWDescs != NULL)
   {
      pDWDescs->clear();

      struct sDockingWindowDescNode * pDWIter = g_pDockingWindowDescs;
      while (pDWIter != NULL)
      {
         pDWDescs->push_back(static_cast<sDockingWindowDesc>(*pDWIter));
         pDWIter = pDWIter->pNext;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////
