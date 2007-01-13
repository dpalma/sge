/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ToolPaletteBar.h"

#include "editorapi.h"
#include "editorCtrlBars.h"
#include "editorTools.h"
#include "BitmapUtils.h"

#include "engine/engineapi.h"
#include "engine/terrainapi.h"

#include "tech/configapi.h"
#include "tech/fileenum.h"
#include "tech/filepath.h"
#include "tech/filespec.h"
#include "tech/globalobj.h"
#include "tech/resourceapi.h"

#include "resource.h"       // main symbols

#include <tinyxml.h>

#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int kToolPaletteId = IDS_TOOL_PALETTE_BAR_TITLE;


std::vector<cStr> g_tileSets;

void GetTileSets(std::vector<cStr> * pTileSets)
{
   if (pTileSets != NULL)
   {
      pTileSets->resize(g_tileSets.size());
      std::copy(g_tileSets.begin(), g_tileSets.end(), pTileSets->begin());
   }
}

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
      Sprintf(&m_name, "STRING%d", nameStringId);
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

static HTOOLGROUP CreateTerrainToolGroup(cToolPalette * pToolPalette, const tChar * pszTerrainTileSet)
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
      HTOOLGROUP hGroup = pToolPalette->AddGroup(tileSetName.c_str(), hTileSetImages);
      if (hGroup != NULL)
      {
         for (uint i = 0; i < nTiles; i++)
         {
            cStr tileName;
            if (pTileSet->GetTileName(i, &tileName) != S_OK)
            {
               continue;
            }

            cTerrainTileTool * pTerrainTool = new cTerrainTileTool(i);
            if (pTerrainTool != NULL)
            {
               pToolPalette->AddTool(hGroup, tileName.c_str(), i, pTerrainTool);
            }
         }

         return hGroup;
      }
   }

   return NULL;
}

/////////////////////////////////////////////////////////////////////////////

static HTOOLGROUP CreateEntityToolGroup(cToolPalette * pToolPalette, const std::vector<cStr> & entities)
{
   HTOOLGROUP hUnitGroup = pToolPalette->AddGroup("Entities", NULL);
   if (hUnitGroup != NULL)
   {
      UseGlobal(ResourceManager);

      std::vector<cStr>::const_iterator iter = entities.begin();
      for (; iter != entities.end(); iter++)
      {
         const TiXmlDocument * pXmlDoc = NULL;
         if (pResourceManager->Load(iter->c_str(), kRT_TiXml, NULL, (void**)&pXmlDoc) != S_OK)
         {
            continue;
         }

         const TiXmlElement * pRoot = pXmlDoc->RootElement();
         if ((pRoot != NULL) && (_stricmp(pRoot->Value(), "entity") == 0))
         {
            cPlaceEntityTool * pPlaceEntityTool = new cPlaceEntityTool(*iter);
            if (pPlaceEntityTool != NULL)
            {
               const char * pszName = pRoot->Attribute("name");
               if (pszName == NULL)
               {
                  pszName = iter->c_str();
               }
               pToolPalette->AddTool(hUnitGroup, pszName, -1, pPlaceEntityTool);
            }
         }
      }
   }

   return hUnitGroup;
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

static void ListFiles(const tChar * pszDir, const tChar * pszExt, std::vector<cFileSpec> * pFiles)
{
   Assert(pszDir && pszExt && pFiles);

   cFileSpec wildcard(_T("*"));
   wildcard.SetPath(cFilePath(pszDir));

   cAutoIPtr<IEnumFiles> pEnum;
   if (EnumFiles(wildcard, &pEnum) == S_OK)
   {
      cFileSpec files[10];
      uint attribs[10];
      ulong nFiles = 0;
      while (SUCCEEDED(pEnum->Next(_countof(files), &files[0], &attribs[0], &nFiles)))
      {
         for (uint i = 0; i < nFiles; i++)
         {
            if (_tcsicmp(files[i].GetFileExt(), pszExt) == 0)
            {
               pFiles->push_back(files[i]);
            }
            else if ((attribs[i] & kFA_Directory) == kFA_Directory)
            {
               ListFiles(files[i].CStr(), pszExt, pFiles);
            }
         }
      }
   }
}

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

   std::vector<cStr> entities;

   cStr data;
   if (ConfigGet(_T("data"), &data) == S_OK)
   {
      std::vector<cFileSpec> xmlFiles;
      ListFiles(data.c_str(), _T("xml"), &xmlFiles);

      UseGlobal(ResourceManager);

      std::vector<cFileSpec>::const_iterator iter = xmlFiles.begin();
      for (; iter != xmlFiles.end(); iter++)
      {
         const TiXmlDocument * pXmlDoc = NULL;
         if (pResourceManager->Load(iter->CStr(), kRT_TiXml, NULL, (void**)&pXmlDoc) != S_OK)
         {
            continue;
         }

         const TiXmlElement * pRoot = pXmlDoc->RootElement();
         if (pRoot != NULL)
         {
            if (_stricmp(pRoot->Value(), "tileset") == 0)
            {
               //TRACE1("TileSet %s\n", iter->GetFileName());
               g_tileSets.push_back(iter->GetFileName());
            }
            else if (_stricmp(pRoot->Value(), "entity") == 0)
            {
               //TRACE1("Entity %s\n", iter->GetFileName());
               entities.push_back(iter->GetFileName());
            }
         }
      }
   }

   UseGlobal(ResourceManager);
   {
      std::vector<cStr>::const_iterator iter = g_tileSets.begin();
      for (; iter != g_tileSets.end(); iter++)
      {
         HTOOLGROUP hToolGroup = CreateTerrainToolGroup(&m_toolPalette, iter->c_str());
         if ((hToolGroup != NULL) && (iter == g_tileSets.begin()))
         {
            UseGlobal(EditorApp);
            pEditorApp->SetDefaultTileSet(iter->c_str());
         }
      }
   }

   CreateEntityToolGroup(&m_toolPalette, entities);

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
            UseGlobal(EditorToolState);
            pEditorToolState->SetActiveTool(pEditorTool);
         }
      }

      if (!pEditorTool)
      {
         UseGlobal(EditorToolState);
         pEditorToolState->SetActiveTool(NULL);
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

   static const cStandardToolDef<cSelectTool> selectTool(IDS_SELECT_TOOL, 0);
   static const cStandardToolDef<cMoveCameraTool> moveCameraTool(IDS_MOVE_CAMERA_TOOL, 3);
   static const cStandardToolDef<cTerrainElevationTool> terrainElevationTool(IDS_ELEVATION_TOOL, 1);
   static const cStandardToolDef<cTerrainPlateauTool> terrainPlateauTool(IDS_PLATEAU_TOOL, 2);

   static const cToolFactory * stdTools[] =
   {
      &selectTool,
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
