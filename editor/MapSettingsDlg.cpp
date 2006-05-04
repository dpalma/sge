/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "MapSettingsDlg.h"

#include "editorapi.h"
#include "terrainapi.h"
#include "BitmapUtils.h"

#include "globalobj.h"
#include "resourceapi.h"

#include <algorithm>
#include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void GetTileSets(std::vector<cStr> * pTileSets);

//////////////////////////////////////////////////////////////////////////////

static const SIZE g_mapSizes[] =
{
   { 64, 64 },
   { 128, 128 },
   { 192, 192 },
   { 256, 256 },
};

static const uint kDefaultMapSizeIndex = 0;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMapSettingsDlg
//

////////////////////////////////////////

cMapSettingsDlg::cMapSettingsDlg(const cTerrainSettings & terrainSettings, CWnd* pParent /*=NULL*/)
 : CDialog(cMapSettingsDlg::IDD, pParent),
   m_initialTile(-1)
{
	//{{AFX_DATA_INIT(cMapSettingsDlg)
	m_tileSet = terrainSettings.GetTileSet();
	m_heightData = static_cast<int>(terrainSettings.GetHeightData());
	m_heightMapFile = terrainSettings.GetHeightMap();
	m_mapHeightIndex = -1;
	m_mapWidthIndex = -1;
	m_heightScale = 1;
	//}}AFX_DATA_INIT

   for (int i = 0; i < _countof(g_mapSizes); i++)
   {
      if (g_mapSizes[i].cx == terrainSettings.GetTileCountX())
      {
	      m_mapWidthIndex = i;
      }
      if (g_mapSizes[i].cy == terrainSettings.GetTileCountZ())
      {
	      m_mapHeightIndex = i;
      }
   }
}

////////////////////////////////////////

static const int kHeightScaleMin = 1;
static const int kHeightScaleMax = 256;

void cMapSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cMapSettingsDlg)
	DDX_Control(pDX, IDC_HEIGHT_SCALE_SPIN, m_heightScaleSpin);
	DDX_Control(pDX, IDC_HEIGHT_SCALE_LABEL, m_heightScaleLabel);
	DDX_Control(pDX, IDC_MAP_INITIAL_TILE, m_initialTileComboBox);
	DDX_Radio(pDX, IDC_HEIGHT_NONE, m_heightData);
	DDX_Text(pDX, IDC_HEIGHT_MAP_FILE, m_heightMapFile);
	DDV_MaxChars(pDX, m_heightMapFile, MAX_PATH);
	DDX_CBIndex(pDX, IDC_MAP_HEIGHT, m_mapHeightIndex);
	DDX_CBIndex(pDX, IDC_MAP_WIDTH, m_mapWidthIndex);
	DDX_Text(pDX, IDC_HEIGHT_SCALE, m_heightScale);
	DDV_MinMaxInt(pDX, m_heightScale, kHeightScaleMin, kHeightScaleMax);
	//}}AFX_DATA_MAP

   if (pDX->m_bSaveAndValidate)
   {
      {
         int tileSetSel = SendDlgItemMessage(IDC_MAP_TILESET, CB_GETCURSEL);
         if (tileSetSel != CB_ERR)
         {
            int index = SendDlgItemMessage(IDC_MAP_TILESET, CB_GETITEMDATA, tileSetSel);
            if (index != CB_ERR)
            {
               m_tileSet = m_tileSets[index].c_str();
            }
         }
      }

      {
         int initialTileSel = m_initialTileComboBox.GetCurSel();
         if (initialTileSel != CB_ERR)
         {
            m_initialTile = m_initialTileComboBox.GetItemData(initialTileSel);
         }
         else
         {
            m_initialTile = -1;
         }
      }
   }
   else
   {
      m_heightScaleSpin.SetRange32(kHeightScaleMin, kHeightScaleMax);

      UpdateDialogControls(this, FALSE);
   }
}

////////////////////////////////////////

BEGIN_MESSAGE_MAP(cMapSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(cMapSettingsDlg)
	ON_BN_CLICKED(IDC_BROWSE_HEIGHT_MAP, OnBrowseHeightMap)
	//}}AFX_MSG_MAP
   ON_BN_CLICKED(IDC_HEIGHT_NONE, DoRadioButtonEnabling)
   ON_BN_CLICKED(IDC_HEIGHT_NOISE, DoRadioButtonEnabling)
   ON_BN_CLICKED(IDC_IMPORT_HEIGHT_MAP, DoRadioButtonEnabling)
   ON_CBN_SELCHANGE(IDC_MAP_TILESET, OnSelectTileSet)
   ON_UPDATE_COMMAND_UI(IDC_BROWSE_HEIGHT_MAP, OnUpdateHeightImportControl)
   ON_UPDATE_COMMAND_UI(IDC_HEIGHT_MAP_FILE, OnUpdateHeightImportControl)
   ON_UPDATE_COMMAND_UI(IDC_HEIGHT_SCALE, OnUpdateHeightImportControl)
   ON_UPDATE_COMMAND_UI(IDC_HEIGHT_SCALE_SPIN, OnUpdateHeightImportControl)
   ON_UPDATE_COMMAND_UI(IDC_HEIGHT_SCALE_LABEL, OnUpdateHeightImportControl)
END_MESSAGE_MAP()

////////////////////////////////////////

tResult cMapSettingsDlg::GetTerrainSettings(cTerrainSettings * pTS) const
{
   if (pTS == NULL)
   {
      return E_POINTER;
   }

   //pTS->SetTileSize(); // Use default
   pTS->SetTileCountX(g_mapSizes[m_mapWidthIndex].cx);
   pTS->SetTileCountZ(g_mapSizes[m_mapHeightIndex].cy);
   pTS->SetTileSet(m_tileSet);
   pTS->SetInitialTile(m_initialTile);
   pTS->SetHeightData(static_cast<eTerrainHeightData>(m_heightData));
   pTS->SetHeightMap(m_heightMapFile);
   pTS->SetHeightMapScale(static_cast<float>(m_heightScale));

   return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// cMapSettingsDlg message handlers

////////////////////////////////////////

void cMapSettingsDlg::OnCancel()
{
   // Don't call the base class method so that the dialog
   // can never be cancelled (no Cancel button, ESC key, etc.)
}

////////////////////////////////////////

BOOL cMapSettingsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
   Assert((GetDlgItem(IDC_MAP_WIDTH)->GetStyle() & CBS_SORT) == 0);
   Assert((GetDlgItem(IDC_MAP_HEIGHT)->GetStyle() & CBS_SORT) == 0);

   CString temp;
   for (uint i = 0; i < _countof(g_mapSizes); i++)
   {
      temp.Format("%d", g_mapSizes[i].cx);
      SendDlgItemMessage(IDC_MAP_WIDTH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)temp);
      temp.Format("%d", g_mapSizes[i].cy);
      SendDlgItemMessage(IDC_MAP_HEIGHT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)temp);
   }

   if (m_tileSet.IsEmpty())
   {
      cStr temp;
      UseGlobal(EditorApp);
      pEditorApp->GetDefaultTileSet(&temp);
      m_tileSet = temp.c_str();
   }

   PopulateTileSetComboBox();

   // Update the dialog controls from the member variables again
   // after populating drop-lists, etc.
   UpdateData(FALSE);

   DoRadioButtonEnabling();
   PopulateInitialTileComboBox();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

////////////////////////////////////////

void cMapSettingsDlg::OnBrowseHeightMap() 
{
   // Force the m_heightData member variable to match the state of the radio buttons
   UpdateData(TRUE);

   CString filter;
   Verify(filter.LoadString(IDS_HEIGHT_MAP_FILTER));
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter);
   if (dlg.DoModal() == IDOK)
   {
      m_heightMapFile = dlg.GetPathName();
      UpdateData(FALSE);
   }
}

////////////////////////////////////////

void cMapSettingsDlg::OnUpdateHeightImportControl(CCmdUI *pCmdUI)
{
   pCmdUI->Enable(GetCheckedRadioButton(IDC_HEIGHT_NONE, IDC_IMPORT_HEIGHT_MAP) == IDC_IMPORT_HEIGHT_MAP);
}

////////////////////////////////////////

void cMapSettingsDlg::DoRadioButtonEnabling()
{
   UpdateDialogControls(this, FALSE);
}

////////////////////////////////////////

void cMapSettingsDlg::OnSelectTileSet()
{
   PopulateInitialTileComboBox();
}

////////////////////////////////////////

void cMapSettingsDlg::PopulateTileSetComboBox()
{
   SendDlgItemMessage(IDC_MAP_TILESET, CB_RESETCONTENT);
   m_tileSets.clear();

   int iSelection = CB_ERR;

   m_tileSets.clear();
   GetTileSets(&m_tileSets);

   UseGlobal(ResourceManager);
   std::vector<cStr>::const_iterator iter = m_tileSets.begin();
   for (; iter != m_tileSets.end(); iter++)
   {
      cStr tileSetName;
      ITerrainTileSet * pTileSet = NULL;
      if (pResourceManager->Load(iter->c_str(), kRT_TerrainTileSet, NULL, (void**)&pTileSet) != S_OK
         || pTileSet->GetName(&tileSetName) != S_OK)
      {
         continue;
      }

      int index = SendDlgItemMessage(IDC_MAP_TILESET, CB_ADDSTRING, 0, (LPARAM)tileSetName.c_str());
      SendDlgItemMessage(IDC_MAP_TILESET, CB_SETITEMDATA, index, (LPARAM)(m_tileSets.size() - 1));
      if (m_tileSet.Compare(iter->c_str()) == 0)
      {
         iSelection = index;
      }
   }

   SendDlgItemMessage(IDC_MAP_TILESET, CB_SETCURSEL, iSelection);
}

////////////////////////////////////////

void cMapSettingsDlg::PopulateInitialTileComboBox()
{
   int sel = SendDlgItemMessage(IDC_MAP_TILESET, CB_GETCURSEL);
   if (sel != CB_ERR)
   {
      m_initialTileComboBox.ResetContent();

      ITerrainTileSet * pTileSet = NULL;
      UseGlobal(ResourceManager);
      if (pResourceManager->Load(m_tileSet, kRT_TerrainTileSet, NULL, (void**)&pTileSet) == S_OK)
      {
         int imageSize = m_initialTileComboBox.GetItemHeight(0);

         HIMAGELIST hImageList = NULL;
         if (TerrainTileSetCreateImageList(pTileSet, imageSize, &hImageList) == S_OK)
         {
            m_initialTileComboBox.SetImageList(CImageList::FromHandle(hImageList));
         }

         uint nTiles = 0;
         if (pTileSet->GetTileCount(&nTiles) == S_OK)
         {
            for (uint i = 0; i < nTiles; i++)
            {
               cStr tileName;
               if (pTileSet->GetTileName(i, &tileName) == S_OK)
               {
                  COMBOBOXEXITEM item = {0};
                  item.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_LPARAM;
                  item.iItem = i;
                  item.pszText = const_cast<char *>(tileName.c_str());
                  item.cchTextMax = tileName.length();
                  item.iImage = i;
                  item.iSelectedImage = i;
                  item.lParam = i;
                  Verify(m_initialTileComboBox.InsertItem(&item) != CB_ERR);
               }
            }

            m_initialTileComboBox.SetCurSel(0);
         }
      }
   }
}

/////////////////////////////////////////////////////////////////////////////
