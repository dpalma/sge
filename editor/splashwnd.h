///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SPLASHWND_H
#define INCLUDED_SPLASHWND_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplashWnd
//

class cSplashWnd : public CWnd
{
public:
   cSplashWnd();
   ~cSplashWnd();

// Operations
   HBITMAP SetBitmap(HBITMAP hBitmap);
   bool Create();
   void SetOKToClose() { m_bOKToClose = true; };  

protected:
   //{{AFX_VIRTUAL(cSplashWnd)
   //}}AFX_VIRTUAL

   //{{AFX_MSG(cSplashWnd)
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnPaint();
   afx_msg void OnClose();
   //}}AFX_MSG

   DECLARE_MESSAGE_MAP()

private:
   bool m_bOKToClose;
   CBitmap m_Bitmap;
   int m_height;
   int m_width;
   CWnd m_wndOwner;
};

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplashThread
//

class cSplashThread : public CWinThread
{
public:
   void HideSplash();
   HBITMAP SetBitmap(HBITMAP hBitmap);

protected:
   cSplashThread();
   virtual ~cSplashThread();

   DECLARE_DYNCREATE(cSplashThread)

   //{{AFX_VIRTUAL(cSplashThread)
   virtual BOOL InitInstance();
   //}}AFX_VIRTUAL

   //{{AFX_MSG(cSplashThread)
   //}}AFX_MSG

   DECLARE_MESSAGE_MAP()

private:
   cSplashWnd m_splashWnd;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SPLASHWND_H
