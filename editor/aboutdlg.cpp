/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aboutdlg.h"

#include <string>

#include "dbgalloc.h" // must be last header

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThirdPartyCredit
//

class cThirdPartyCredit
{
public:
   cThirdPartyCredit(const tChar * pszTitle, const tChar * pszAuthor, const tChar * pszUrl);
   cThirdPartyCredit(const cThirdPartyCredit & other);
   ~cThirdPartyCredit();
   const cThirdPartyCredit & operator =(const cThirdPartyCredit & other);
   const tChar * GetTitle() const;
   const tChar * GetAuthor() const;
   const tChar * GetUrl() const;
private:
   std::string m_title, m_author, m_url;
};

////////////////////////////////////////

cThirdPartyCredit::cThirdPartyCredit(const tChar * pszTitle, const tChar * pszAuthor, const tChar * pszUrl)
 : m_title((pszTitle != NULL) ? pszTitle : ""),
   m_author((pszAuthor != NULL) ? pszAuthor : ""),
   m_url((pszUrl != NULL) ? pszUrl : "")
{
}

////////////////////////////////////////

cThirdPartyCredit::cThirdPartyCredit(const cThirdPartyCredit & other)
{
   operator =(other);
}

////////////////////////////////////////

cThirdPartyCredit::~cThirdPartyCredit()
{
}

////////////////////////////////////////

const cThirdPartyCredit & cThirdPartyCredit::operator =(const cThirdPartyCredit & other)
{
   m_title = other.m_title;
   m_author = other.m_author;
   m_url = other.m_url;
   return *this;
}

////////////////////////////////////////

const tChar * cThirdPartyCredit::GetTitle() const
{
   return m_title.c_str();
}

////////////////////////////////////////

const tChar * cThirdPartyCredit::GetAuthor() const
{
   return m_author.c_str();
}

////////////////////////////////////////

const tChar * cThirdPartyCredit::GetUrl() const
{
   return m_url.c_str();
}

/////////////////////////////////////////////////////////////////////////////

static cThirdPartyCredit g_thirdPartyCredits[] =
{
   cThirdPartyCredit("CppUnit", "(various)", "http://cppunit.sourceforge.net/"),
   cThirdPartyCredit("TinyXml", "Lee Thomason", "http://www.grinninglizard.com/tinyxml/"),
   cThirdPartyCredit("Lua", "PUC-Rio", "http://www.lua.org/"),
   cThirdPartyCredit("Sizing Control Bars", "Cristi Posea", "http://www.datamekanix.com"),
   //cThirdPartyCredit("WTL Docking Windows", "Sergey Klimov", "http://www.codeproject.com/wtl/wtldockingwindows.asp"),
   cThirdPartyCredit("zlib", "Jean-loup Gailly and Mark Adler", "http://www.gzip.org/zlib/"),
};

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: CAboutDlg
//

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
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
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   CString creditString;
   for (int i = 0; i < _countof(g_thirdPartyCredits); i++)
   {
      const cThirdPartyCredit & c = g_thirdPartyCredits[i];

      CString s;
      s += c.GetTitle();
      s += ", ";
      s += c.GetAuthor();
      s += "\r\n";
      s += c.GetUrl();
      s += "\r\n\r\n";

      creditString += s;
   }

   SetDlgItemText(IDC_CREDITS, creditString);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
