/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aboutdlg.h"

#include <string>

#include "dbgalloc.h" // must be last header

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
//
// CLASS: cAboutDlg
//

////////////////////////////////////////

LRESULT cAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
   CenterWindow(GetParent());

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

////////////////////////////////////////

LRESULT cAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
   EndDialog(wID);
   return 0;
}

/////////////////////////////////////////////////////////////////////////////
