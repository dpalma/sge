/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "MapSettingsDlg.h"
#include "editorapi.h"
#include "editorTypes.h"

#include "globalobj.h"

#include <algorithm>
#include <functional>

#include "dbgalloc.h" // must be last header

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

cMapSettingsDlg::cMapSettingsDlg(eHeightData heightData, CWnd* pParent /*=NULL*/)
 : CDialog(cMapSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(cMapSettingsDlg)
	m_heightData = -1;
	m_heightMapFile = _T("");
	m_mapHeightIndex = -1;
	m_mapWidthIndex = -1;
	//}}AFX_DATA_INIT

   cStr tileSet;
   UseGlobal(EditorTileManager);
   pEditorTileManager->GetDefaultTileSet(&tileSet);
   m_tileSet = tileSet.c_str();

	m_mapHeightIndex = kDefaultMapSizeIndex;
	m_mapWidthIndex = kDefaultMapSizeIndex;

   m_heightData = heightData;
}

////////////////////////////////////////

void cMapSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cMapSettingsDlg)
	DDX_Control(pDX, IDC_MAP_INITIAL_TILE, m_initialTileComboBox);
	DDX_CBStringExact(pDX, IDC_MAP_TILESET, m_tileSet);
	DDX_Radio(pDX, IDC_HEIGHT_NONE, m_heightData);
	DDX_Text(pDX, IDC_HEIGHT_MAP_FILE, m_heightMapFile);
	DDV_MaxChars(pDX, m_heightMapFile, MAX_PATH);
	DDX_CBIndex(pDX, IDC_MAP_HEIGHT, m_mapHeightIndex);
	DDX_CBIndex(pDX, IDC_MAP_WIDTH, m_mapWidthIndex);
	//}}AFX_DATA_MAP
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
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cMapSettingsDlg message handlers

////////////////////////////////////////

tResult cMapSettingsDlg::GetMapSettings(cMapSettings * pMapSettings) const
{
   if (pMapSettings == NULL)
   {
      return E_POINTER;
   }

   *pMapSettings = cMapSettings(
      g_mapSizes[m_mapWidthIndex].cx,
      g_mapSizes[m_mapHeightIndex].cy,
      m_tileSet,
      (eHeightData)m_heightData,
      m_heightMapFile);

   return S_OK;
}

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
	
   // Don't sort the drop-lists because selection is defined and
   // returned as indices into the lists given in the constructor
   ASSERT_VALID(GetDlgItem(IDC_MAP_WIDTH));
   ASSERT_VALID(GetDlgItem(IDC_MAP_HEIGHT));
   ASSERT_VALID(GetDlgItem(IDC_MAP_TILESET));
   GetDlgItem(IDC_MAP_WIDTH)->ModifyStyle(CBS_SORT, 0);
   GetDlgItem(IDC_MAP_HEIGHT)->ModifyStyle(CBS_SORT, 0);
   GetDlgItem(IDC_MAP_TILESET)->ModifyStyle(CBS_SORT, 0);

   if (!m_mapSizes.empty())
   {
      std::vector<SIZE>::iterator iter;
      for (iter = m_mapSizes.begin(); iter != m_mapSizes.end(); iter++)
      {
         CString str;
         str.Format("%d", iter->cx);
         SendDlgItemMessage(IDC_MAP_WIDTH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)str);
         str.Format("%d", iter->cy);
         SendDlgItemMessage(IDC_MAP_HEIGHT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)str);
      }
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

void cMapSettingsDlg::DoRadioButtonEnabling()
{
   ASSERT_VALID(GetDlgItem(IDC_IMPORT_HEIGHT_MAP));
   ASSERT_VALID(GetDlgItem(IDC_BROWSE_HEIGHT_MAP));
   ASSERT_VALID(GetDlgItem(IDC_HEIGHT_MAP_FILE));

   if (IsDlgButtonChecked(IDC_IMPORT_HEIGHT_MAP) == BST_CHECKED)
   {
      GetDlgItem(IDC_BROWSE_HEIGHT_MAP)->EnableWindow(TRUE);
      GetDlgItem(IDC_HEIGHT_MAP_FILE)->EnableWindow(TRUE);
   }
   else
   {
      GetDlgItem(IDC_BROWSE_HEIGHT_MAP)->EnableWindow(FALSE);
      GetDlgItem(IDC_HEIGHT_MAP_FILE)->EnableWindow(FALSE);
   }
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

   uint nTileSets = 0;
   UseGlobal(EditorTileManager);
   if (pEditorTileManager->GetTileSetCount(&nTileSets) == S_OK && nTileSets > 0)
   {
      for (uint i = 0; i < nTileSets; i++)
      {
         cStr name;
         cAutoIPtr<IEditorTileSet> pTileSet;
         if (pEditorTileManager->GetTileSet(i, &pTileSet) == S_OK
            && pTileSet->GetName(&name) == S_OK)
         {
            SendDlgItemMessage(IDC_MAP_TILESET, CB_ADDSTRING, 0, (LPARAM)name.c_str());
         }
         else
         {
            WarnMsg1("Error getting tile set %d\n", i);
         }
      }
   }
}

////////////////////////////////////////

void cMapSettingsDlg::PopulateInitialTileComboBox()
{
   int sel = SendDlgItemMessage(IDC_MAP_TILESET, CB_GETCURSEL);
   if (sel != CB_ERR)
   {
      m_initialTileComboBox.ResetContent();

      cAutoIPtr<IEditorTileSet> pTileSet;
      UseGlobal(EditorTileManager);
      if (pEditorTileManager->GetTileSet(m_tileSet, &pTileSet) == S_OK)
      {
         int imageSize = m_initialTileComboBox.GetItemHeight(0);

         HIMAGELIST hImageList = NULL;
         if (pTileSet->GetImageList(imageSize, &hImageList) == S_OK)
         {
            m_initialTileComboBox.SetImageList(CImageList::FromHandle(hImageList));
         }

         uint nTiles = 0;
         if (pTileSet->GetTileCount(&nTiles) == S_OK)
         {
            for (uint i = 0; i < nTiles; i++)
            {
               cAutoIPtr<IEditorTile> pTile;
               if (pTileSet->GetTile(i, &pTile) == S_OK)
               {
                  cStr tileName;
                  Verify(pTile->GetName(&tileName) == S_OK);

                  COMBOBOXEXITEM item;
                  item.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
                  item.iItem = i;
                  item.pszText = const_cast<char *>(tileName.c_str());
                  item.cchTextMax = tileName.length();
                  item.iImage = i;
                  item.iSelectedImage = i;

                  Verify(m_initialTileComboBox.InsertItem(&item) != CB_ERR);
               }
            }

            m_initialTileComboBox.SetCurSel(0);
         }
      }
   }
}

/////////////////////////////////////////////////////////////////////////////
