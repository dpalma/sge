/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ToolPaletteBar.h"

#include "editorapi.h"
#include "editorCtrlBars.h"
#include "editorTools.h"
#include "BitmapUtils.h"

#include "terrainapi.h"

#include "globalobj.h"
#include "resourceapi.h"

#include "resource.h"       // main symbols

#include <vector>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int kToolPaletteId = 1234;


/////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cStandardToolDef
//

class cToolFactory
{
public:
   virtual ~cToolFactory()
   {
   }

   virtual const tChar * GetName() const = 0;
   virtual int GetImageIndex() const = 0;
   virtual tResult CreateTool(IEditorTool * * ppTool) const = 0;
};

template <class TOOL>
class cStandardToolDef : public cToolFactory
{
   cStandardToolDef();

public:
   cStandardToolDef(const tChar * pszName, int imageIndex);
   cStandardToolDef(uint nameStringId, int imageIndex);

   const tChar * GetName() const { return m_name.c_str(); }
   int GetImageIndex() const { return m_imageIndex; }

   tResult CreateTool(IEditorTool * * ppTool) const;

private:
   cStr m_name;
   int m_imageIndex;
};

////////////////////////////////////////

template <class TOOL>
cStandardToolDef<TOOL>::cStandardToolDef()
 : m_imageIndex(-1)
{
}

////////////////////////////////////////

template <class TOOL>
cStandardToolDef<TOOL>::cStandardToolDef(const tChar * pszName, int imageIndex)
 : m_name(pszName)
 , m_imageIndex(imageIndex)
{
}

////////////////////////////////////////

template <class TOOL>
cStandardToolDef<TOOL>::cStandardToolDef(uint nameStringId, int imageIndex)
 : m_imageIndex(imageIndex)
{
   CString temp;
   if (temp.LoadString(nameStringId))
   {
      m_name.assign(temp);
   }
   else
   {
      ErrorMsg1("Error loading string resource, id = %d\n", nameStringId);
      m_name.Format("STRING%d", nameStringId);
   }
}

////////////////////////////////////////

template <class TOOL>
tResult cStandardToolDef<TOOL>::CreateTool(IEditorTool * * ppTool) const
{
   if (ppTool == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IEditorTool> pTool(static_cast<IEditorTool*>(new TOOL));
   if (!pTool)
   {
      return E_OUTOFMEMORY;
   }

   *ppTool = CTAddRef(pTool);
   return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPaletteBar
//

AUTO_REGISTER_CONTROLBAR(IDS_TOOL_PALETTE_BAR_TITLE, RUNTIME_CLASS(cToolPaletteBar), kCBP_Right);

IMPLEMENT_DYNCREATE(cToolPaletteBar, cEditorControlBar);

cToolPaletteBar::cToolPaletteBar()
{
}

cToolPaletteBar::~cToolPaletteBar()
{
}

BEGIN_MESSAGE_MAP(cToolPaletteBar, cEditorControlBar)
   //{{AFX_MSG_MAP(cToolPaletteBar)
   ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
   ON_NOTIFY(kTPN_ItemCheck, kToolPaletteId, OnToolPaletteItemCheck)
   ON_NOTIFY(kTPN_ItemDestroy, kToolPaletteId, OnToolPaletteItemDestroy)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cToolPaletteBar message handlers

int cToolPaletteBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
   if (cEditorControlBar::OnCreate(lpCreateStruct) == -1)
      return -1;

   if (!m_tooltip.Create(this))
   {
      DebugMsg1("Unable to create tooltip control (error %d)\n", GetLastError());
      return -1;
   }

   if (!m_toolPalette.Create(m_hWnd, CWindow::rcDefault, "", 0, 0, kToolPaletteId))
   {
      ErrorMsg1("Unable to create tool palette control (error %d)\n", GetLastError());
      return -1;
   }

   CreateStandardToolGroup();

   std::vector<cStr> terrainTileSets;
   UseGlobal(ResourceManager);
   if (pResourceManager->ListResources(kRT_TerrainTileSet, &terrainTileSets) == S_OK)
   {
      bool bSetDefaultTileSet = false;
      std::vector<cStr>::const_iterator iter = terrainTileSets.begin();
      for (; iter != terrainTileSets.end(); iter++)
      {
         HTOOLGROUP hToolGroup = CreateTerrainToolGroup(iter->c_str());
         if (!bSetDefaultTileSet && (hToolGroup != NULL))
         {
            UseGlobal(EditorApp);
            pEditorApp->SetDefaultTileSet(iter->c_str());
            bSetDefaultTileSet = true;
         }
      }
   }

   return 0;
}

void cToolPaletteBar::OnDestroy() 
{
   cEditorControlBar::OnDestroy();

   m_toolPalette.Clear();
}

void cToolPaletteBar::OnSize(UINT nType, int cx, int cy) 
{
	cEditorControlBar::OnSize(nType, cx, cy);
   if (m_toolPalette.IsWindow())
   {
      CRect rect;
      GetClientRect(rect);
      m_toolPalette.MoveWindow(rect);
   }
}

void cToolPaletteBar::OnToolPaletteItemCheck(NMHDR * pnmh, LRESULT * pResult)
{
   Assert(pnmh != NULL);

   sToolPaletteItem tpi;
   if (m_toolPalette.GetTool(((sNMToolPaletteItemClick *)pnmh)->hTool, &tpi))
   {
      cAutoIPtr<IEditorTool> pEditorTool;
      if (tpi.pUserData != NULL)
      {
         IUnknown * pUnk = reinterpret_cast<IUnknown *>(tpi.pUserData);
         if (pUnk->QueryInterface(&pEditorTool) == S_OK)
         {
            UseGlobal(EditorApp);
            pEditorApp->SetActiveTool(pEditorTool);
         }
      }

      if (!pEditorTool)
      {
         UseGlobal(EditorApp);
         pEditorApp->SetActiveTool(NULL);
      }
   }
}

void cToolPaletteBar::OnToolPaletteItemDestroy(NMHDR * pnmh, LRESULT * pResult)
{
   Assert(pnmh != NULL);

   sToolPaletteItem tpi;
   if (m_toolPalette.GetTool(((sNMToolPaletteItemClick *)pnmh)->hTool, &tpi))
   {
      if (tpi.pUserData != NULL)
      {
         IUnknown * pUnk = reinterpret_cast<IUnknown *>(tpi.pUserData);
         pUnk->Release();
      }
   }
}

BOOL cToolPaletteBar::PreTranslateMessage(MSG* pMsg) 
{
   if (IsWindow(m_tooltip.GetSafeHwnd()))
   {
      if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONUP || pMsg->message == WM_MOUSEMOVE)
      {
         m_tooltip.RelayEvent(pMsg);
      }
   }

   return cEditorControlBar::PreTranslateMessage(pMsg);
}

HTOOLGROUP cToolPaletteBar::CreateStandardToolGroup()
{
   HIMAGELIST hStdImages = ImageList_LoadImage(_Module.GetResourceInstance(),
      MAKEINTRESOURCE(IDB_STD_TOOLS), 16, 0, CLR_DEFAULT, IMAGE_BITMAP, 0);
   if (hStdImages == NULL)
   {
      ErrorMsg("Error loading image list for standard tools\n");
      return NULL;
   }

   static const cStandardToolDef<cMoveCameraTool> moveCameraTool(IDS_SELECT_TOOL, 0);
   static const cStandardToolDef<cTerrainElevationTool> terrainElevationTool(IDS_ELEVATION_TOOL, 1);
   static const cStandardToolDef<cTerrainPlateauTool> terrainPlateauTool(IDS_PLATEAU_TOOL, 2);

   static const cToolFactory * stdTools[] =
   {
      &moveCameraTool,
      &terrainElevationTool,
      &terrainPlateauTool,
   };

   HTOOLGROUP hStdGroup = m_toolPalette.AddGroup("", hStdImages);
   if (hStdGroup != NULL)
   {
      for (int i = 0; i < _countof(stdTools); i++)
      {
         cAutoIPtr<IEditorTool> pTool;
         if (stdTools[i]->CreateTool(&pTool) != S_OK)
         {
            ErrorMsg1("Error creating \"%s\" tool\n", stdTools[i]->GetName());
            continue;
         }
         HTOOLITEM hTool = m_toolPalette.AddTool(hStdGroup, stdTools[i]->GetName(),
            stdTools[i]->GetImageIndex(), CTAddRef(pTool));
         if (hTool == NULL)
         {
            pTool->Release(); // Release once for the CTAddRef above
            ErrorMsg1("Error adding \"%s\" to toolbar\n", stdTools[i]->GetName());
         }
      }
   }

   return hStdGroup;
}

HTOOLGROUP cToolPaletteBar::CreateTerrainToolGroup(const tChar * pszTerrainTileSet)
{
   UseGlobal(ResourceManager);

   ITerrainTileSet * pTileSet = NULL;
   if (pResourceManager->Load(pszTerrainTileSet, kRT_TerrainTileSet, NULL, (void**)&pTileSet) != S_OK)
   {
      return NULL;
   }

   uint nTiles = 0;
   if ((pTileSet->GetTileCount(&nTiles) != S_OK) || (nTiles == 0))
   {
      return NULL;
   }

   cStr tileSetName;
   HIMAGELIST hTileSetImages = NULL;
   if (pTileSet->GetName(&tileSetName) == S_OK
      && TerrainTileSetCreateImageList(pTileSet, 16, &hTileSetImages) == S_OK)
   {
      HTOOLGROUP hGroup = m_toolPalette.AddGroup(tileSetName.c_str(), hTileSetImages);
      if (hGroup != NULL)
      {
         for (uint i = 0; i < nTiles; i++)
         {
            cStr tileName;
            if (pTileSet->GetTileName(i, &tileName) != S_OK)
            {
               continue;
            }

            cTerrainTileTool * pTerrainTool = new cTerrainTileTool;
            if (pTerrainTool != NULL)
            {
               pTerrainTool->SetTile(i);
               m_toolPalette.AddTool(hGroup, tileName.c_str(), i, pTerrainTool);
            }
         }

         return hGroup;
      }
   }

   return NULL;
}
