/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "MapPropertiesDlg.h"

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


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMapPropertiesDlg
//

////////////////////////////////////////

cMapPropertiesDlg::cMapPropertiesDlg(CWnd* pParent /*=NULL*/)
 : CDialog(cMapPropertiesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(cMapPropertiesDlg)
	m_numPlayers = 1;
	//}}AFX_DATA_INIT
}

void cMapPropertiesDlg::SetTitle(LPCTSTR pszTitle)
{
   if (pszTitle != NULL)
   {
      m_title = pszTitle;
   }
   else
   {
      m_title.Empty();
   }
}

void cMapPropertiesDlg::GetTitle(CString * pTitle) const
{
   if (pTitle != NULL)
   {
      *pTitle = m_title;
   }
}

void cMapPropertiesDlg::SetAuthor(LPCTSTR pszAuthor)
{
   if (pszAuthor != NULL)
   {
      m_author = pszAuthor;
   }
   else
   {
      m_author.Empty();
   }
}

void cMapPropertiesDlg::GetAuthor(CString * pAuthor) const
{
   if (pAuthor != NULL)
   {
      *pAuthor = m_author;
   }
}

void cMapPropertiesDlg::SetNumPlayers(int numPlayers)
{
   m_numPlayers = numPlayers;
}

int cMapPropertiesDlg::GetNumPlayers() const
{
   return m_numPlayers;
}

void cMapPropertiesDlg::SetDescription(LPCTSTR pszDesc)
{
   if (pszDesc != NULL)
   {
      m_description = pszDesc;
   }
   else
   {
      m_description.Empty();
   }
}

void cMapPropertiesDlg::GetDescription(CString * pDesc) const
{
   if (pDesc != NULL)
   {
      *pDesc = m_description;
   }
}

////////////////////////////////////////

#define kNumPlayersMin 1
#define kNumPlayersMax 8

void cMapPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cMapPropertiesDlg)
	DDX_Text(pDX, IDC_TITLE, m_title);
	DDX_Text(pDX, IDC_AUTHOR, m_author);
	DDX_Text(pDX, IDC_NUMPLAYERS, m_numPlayers);
	DDX_Control(pDX, IDC_SPIN_NUMPLAYERS, m_numPlayersSpin);
	DDX_Text(pDX, IDC_DESCRIPTION, m_description);
	DDV_MinMaxInt(pDX, m_numPlayers, kNumPlayersMin, kNumPlayersMax);
	//}}AFX_DATA_MAP

   if (pDX->m_bSaveAndValidate)
   {
   }
   else
   {
      m_numPlayersSpin.SetRange32(kNumPlayersMin, kNumPlayersMax);
      UpdateDialogControls(this, FALSE);
   }
}

////////////////////////////////////////

BEGIN_MESSAGE_MAP(cMapPropertiesDlg, CDialog)
	//{{AFX_MSG_MAP(cMapPropertiesDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cMapPropertiesDlg message handlers

////////////////////////////////////////

BOOL cMapPropertiesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
