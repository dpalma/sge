/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "MapSettingsDlg.h"
#include "editorapi.h"

#include <algorithm>
#include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMapSettings
//

////////////////////////////////////////

cMapSettings::cMapSettings()
 : m_xDimension(0),
   m_zDimension(0),
   m_tileSet(""),
   m_heightData(kHeightData_None),
   m_heightMapFile("")
{
}

////////////////////////////////////////

cMapSettings::cMapSettings(uint xDimension,
                           uint zDimension,
                           const tChar * pszTileSet,
                           eHeightData heightData,
                           const tChar * pszHeightMapFile)
 : m_xDimension(xDimension),
   m_zDimension(zDimension),
   m_tileSet(pszTileSet != NULL ? pszTileSet : ""),
   m_heightData(heightData),
   m_heightMapFile(pszHeightMapFile != NULL ? pszHeightMapFile : "")
{
}

////////////////////////////////////////

cMapSettings::cMapSettings(const cMapSettings & mapSettings)
 : m_xDimension(mapSettings.m_xDimension),
   m_zDimension(mapSettings.m_zDimension),
   m_tileSet(mapSettings.m_tileSet),
   m_heightData(mapSettings.m_heightData),
   m_heightMapFile(mapSettings.m_heightMapFile)
{
}

////////////////////////////////////////

const cMapSettings & cMapSettings::operator=(const cMapSettings & mapSettings)
{
   m_xDimension = mapSettings.m_xDimension;
   m_zDimension = mapSettings.m_zDimension;
   m_tileSet = mapSettings.m_tileSet;
   m_heightData = mapSettings.m_heightData;
   m_heightMapFile = mapSettings.m_heightMapFile;
   return *this;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMapSettingsDlg
//

////////////////////////////////////////

cMapSettingsDlg::cMapSettingsDlg(const SIZE * pSizes, size_t nSizes, int sizeSelectIndex,
                                 const std::vector<cStr> & tileSets, int tileSetSelectIndex,
                                 eHeightData heightData,
                                 CWnd* pParent /*=NULL*/)
 : CDialog(cMapSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(cMapSettingsDlg)
	m_mapSizeIndex = -1;
	m_tileSetIndex = -1;
	m_heightData = -1;
	m_heightMapFile = _T("");
	//}}AFX_DATA_INIT

   if (pSizes != NULL && nSizes > 0)
   {
      Assert(AfxIsValidAddress(pSizes, nSizes * sizeof(SIZE), FALSE));
      m_mapSizes.resize(nSizes);
      for (uint i = 0; i < nSizes; i++)
      {
         m_mapSizes[i] = pSizes[i];
      }

      m_mapSizeIndex = sizeSelectIndex;
   }

   if (!tileSets.empty())
   {
      m_tileSets.resize(tileSets.size());
      std::copy(tileSets.begin(), tileSets.end(), m_tileSets.begin());
      m_tileSetIndex = tileSetSelectIndex;
   }

   m_heightData = heightData;
}

////////////////////////////////////////

void cMapSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cMapSettingsDlg)
	DDX_CBIndex(pDX, IDC_MAP_SIZE, m_mapSizeIndex);
	DDX_CBIndex(pDX, IDC_MAP_TILESET, m_tileSetIndex);
	DDX_Radio(pDX, IDC_HEIGHT_NONE, m_heightData);
	DDX_Text(pDX, IDC_HEIGHT_MAP_FILE, m_heightMapFile);
	DDV_MaxChars(pDX, m_heightMapFile, MAX_PATH);
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
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cMapSettingsDlg message handlers

////////////////////////////////////////

bool cMapSettingsDlg::GetSelectedSize(SIZE * pSize) const
{
   Assert(pSize != NULL);
   if (pSize == NULL)
   {
      return false;
   }

   if (m_mapSizeIndex >= 0 && m_mapSizeIndex < m_mapSizes.size())
   {
      *pSize = m_mapSizes[m_mapSizeIndex];
      return true;
   }

   return false;
}

////////////////////////////////////////

bool cMapSettingsDlg::GetSelectedTileSet(cStr * pTileSet) const
{
   Assert(pTileSet != NULL);
   if (pTileSet == NULL)
   {
      return false;
   }

   if (m_tileSetIndex >= 0 && m_tileSetIndex < m_tileSets.size())
   {
      *pTileSet = m_tileSets[m_tileSetIndex];
      return true;
   }

   return false;
}

////////////////////////////////////////

eHeightData cMapSettingsDlg::GetHeightData() const
{
   Assert(m_heightData == kHeightData_None ||
      m_heightData == kHeightData_HeightMap ||
      m_heightData == kHeightData_Noise);
   return (eHeightData)m_heightData;
}

////////////////////////////////////////

bool cMapSettingsDlg::GetHeightDataFile(cStr * pHeightData) const
{
   if (pHeightData != NULL)
   {
      *pHeightData = (LPCTSTR)m_heightMapFile;
      return true;
   }
   return false;
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
   ASSERT_VALID(GetDlgItem(IDC_MAP_SIZE));
   ASSERT_VALID(GetDlgItem(IDC_MAP_TILESET));
   GetDlgItem(IDC_MAP_SIZE)->ModifyStyle(CBS_SORT, 0);
   GetDlgItem(IDC_MAP_TILESET)->ModifyStyle(CBS_SORT, 0);

   if (!m_mapSizes.empty())
   {
      std::vector<SIZE>::iterator iter;
      for (iter = m_mapSizes.begin(); iter != m_mapSizes.end(); iter++)
      {
         CString str;
         str.Format("%d x %d\n", iter->cx, iter->cy);
         SendDlgItemMessage(IDC_MAP_SIZE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)str);
      }
   }

   if (!m_tileSets.empty())
   {
      std::vector<cStr>::iterator iter;
      for (iter = m_tileSets.begin(); iter != m_tileSets.end(); iter++)
      {
         SendDlgItemMessage(IDC_MAP_TILESET, CB_ADDSTRING, 0, (LPARAM)iter->c_str());
      }
   }

   // Update the dialog controls from the member variables again
   // after populating drop-lists, etc.
   UpdateData(FALSE);

   DoRadioButtonEnabling();
	
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

/////////////////////////////////////////////////////////////////////////////
