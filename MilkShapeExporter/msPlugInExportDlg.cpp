/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "resource.h"
#include "msPlugInExportDlg.h"
#include "ModelTreeInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMsPlugInExportDlg
//

////////////////////////////////////////

cMsPlugInExportDlg::cMsPlugInExportDlg(cModelTreeInfo * pModelTreeInfo, CWnd* pParent /*=NULL*/)
 : CDialog(cMsPlugInExportDlg::IDD, pParent),
   m_pModelTreeInfo(pModelTreeInfo)
{
	//{{AFX_DATA_INIT(cMsPlugInExportDlg)
	m_bExportAnimations = FALSE;
	m_bExportMaterials = FALSE;
	m_exportFileName = _T("");
	m_skeletonFileName = _T("");
	//}}AFX_DATA_INIT
}

////////////////////////////////////////

void cMsPlugInExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cMsPlugInExportDlg)
	DDX_Control(pDX, IDC_MODEL_INFO_TREE, m_modelInfo);
	DDX_Control(pDX, IDC_ANIMATIONS, m_animations);
	DDX_Check(pDX, IDC_EXPORTANIMATIONS, m_bExportAnimations);
	DDX_Check(pDX, IDC_EXPORTMATERIALS, m_bExportMaterials);
	DDX_Text(pDX, IDC_FILENAME, m_exportFileName);
	DDX_Text(pDX, IDC_SKELETONFILENAME, m_skeletonFileName);
	//}}AFX_DATA_MAP
}

////////////////////////////////////////

BEGIN_MESSAGE_MAP(cMsPlugInExportDlg, CDialog)
	//{{AFX_MSG_MAP(cMsPlugInExportDlg)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_BROWSESKELETON, OnBrowseSkeleton)
	ON_BN_CLICKED(IDC_ADDANIMATION, OnAddAnimation)
	ON_BN_CLICKED(IDC_REMOVEANIMATION, OnRemoveAnimation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cMsPlugInExportDlg message handlers

BOOL ChooseExportFileName(CString * pFileName)
{
   Assert(pFileName != NULL);

   CString defaultExt;
   VERIFY(defaultExt.LoadString(IDS_EXPORT_DEFAULT_EXTENSION));

   CString filter;
   VERIFY(filter.LoadString(IDS_EXPORT_FILTER));

	CFileDialog dlg(FALSE, defaultExt, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter);
   if (dlg.DoModal() == IDOK)
   {
      *pFileName = dlg.GetPathName();
      return TRUE;
   }

   return FALSE;
}

void cMsPlugInExportDlg::OnBrowse() 
{
   if (ChooseExportFileName(&m_exportFileName))
   {
      UpdateData(FALSE);
   }
}

void cMsPlugInExportDlg::OnBrowseSkeleton() 
{
	CFileDialog dlg(FALSE);
   if (dlg.DoModal() == IDOK)
   {
      m_skeletonFileName = dlg.GetPathName();
      UpdateData(FALSE);
   }
}

void cMsPlugInExportDlg::OnAddAnimation() 
{
	// TODO: Add your control notification handler code here
	
}

void cMsPlugInExportDlg::OnRemoveAnimation() 
{
	// TODO: Add your control notification handler code here
	
}

BOOL cMsPlugInExportDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
   m_addAnimationButton.AutoLoad(IDC_ADDANIMATION, this);
   m_removeAnimationButton.AutoLoad(IDC_REMOVEANIMATION, this);

   CRect rect;
   m_animations.GetClientRect(rect);
   VERIFY(m_animations.InsertColumn(0, "Name", LVCFMT_LEFT, rect.Width() / 2) == 0);
   VERIFY(m_animations.InsertColumn(1, "Start", LVCFMT_LEFT, rect.Width() / 4) == 1);
   VERIFY(m_animations.InsertColumn(2, "End", LVCFMT_LEFT, rect.Width() / 4) == 2);

   if (m_pModelTreeInfo != NULL)
   {
      m_pModelTreeInfo->DisplayModelInfo(&m_modelInfo);
   }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
