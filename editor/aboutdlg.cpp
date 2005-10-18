/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aboutdlg.h"

#ifndef _AFX_NO_OLE_SUPPORT
extern "C"
{
HIMAGELIST WINAPI ImageList_Read(LPSTREAM pstm);
BOOL       WINAPI ImageList_Write(HIMAGELIST himl, LPSTREAM pstm);
}
#endif
#include <atlctrls.h>
#include <atlctrlx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: CAboutDlg
//

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

CAboutDlg::~CAboutDlg()
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
   ON_WM_DESTROY()
END_MESSAGE_MAP()

static BOOL CALLBACK CreateHyperLinks(HWND hwnd, LPARAM lParam)
{
   std::vector<WTL::CHyperLink*> * pHyperLinks = (std::vector<WTL::CHyperLink*>*)lParam;

   TCHAR szClassName[30];
   if (GetClassName(hwnd, szClassName, _countof(szClassName)) > 0)
   {
      if (_tcsicmp(szClassName, _T("static")) == 0)
      {
         CString text;
         int length = GetWindowTextLength(hwnd);
         TCHAR * pszText = text.GetBuffer(length + 1);
         if (GetWindowText(hwnd, pszText, length) > 0)
         {
            if (_tcsstr(text, _T("http://")))
            {
               WTL::CHyperLink * pHyperLink = new WTL::CHyperLink;
               if (pHyperLink != NULL)
               {
                  Verify(pHyperLink->SubclassWindow(hwnd));
                  pHyperLinks->push_back(pHyperLink);
               }
            }
         }
      }
   }

   return TRUE;
}

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   EnumChildWindows(m_hWnd, CreateHyperLinks, (LPARAM)&m_hyperLinks);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAboutDlg::OnDestroy()
{
   CDialog::OnDestroy();

   std::vector<WTL::CHyperLink*>::iterator iter = m_hyperLinks.begin();
   for (; iter != m_hyperLinks.end(); iter++)
   {
      (*iter)->UnsubclassWindow();
      delete (*iter);
   }
   m_hyperLinks.clear();
}
