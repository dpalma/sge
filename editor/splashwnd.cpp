///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "splashwnd.h"
#include "BitmapUtils.h"

#include "configapi.h"
#include "techstring.h"
#include "thread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// REFERENCES
// http://www.codeproject.com/dialog/splasher.asp

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplashWnd
//

///////////////////////////////////////

BEGIN_MESSAGE_MAP(cSplashWnd, CWnd)
   //{{AFX_MSG_MAP(cSplashWnd)
   ON_WM_CREATE()
   ON_WM_PAINT()
   ON_WM_CLOSE()
   //}}AFX_MSG_MAP
END_MESSAGE_MAP()

///////////////////////////////////////

cSplashWnd::cSplashWnd()
 : m_bOKToClose(false),
   m_height(0),
   m_width(0)
{
}

///////////////////////////////////////

cSplashWnd::~cSplashWnd()
{
   if (m_wndOwner.GetSafeHwnd() != NULL)
   {
      m_wndOwner.DestroyWindow();
   }
}

///////////////////////////////////////

HBITMAP cSplashWnd::SetBitmap(HBITMAP hBitmap)
{
   HBITMAP hFormer = (HBITMAP)m_Bitmap.Detach();
   if (hBitmap != NULL)
   {
      m_Bitmap.Attach(hBitmap);

      BITMAP bm = {0};
      Verify(m_Bitmap.GetBitmap(&bm));
      m_width = bm.bmWidth;
      m_height = bm.bmHeight;
   }
   return hFormer;
}

///////////////////////////////////////

bool cSplashWnd::Create()
{
   // The owner window of the splash screen is an invisible WS_POPUP to
   // prevent the splash screen window from appearing on the task bar.
   if (!m_wndOwner.CreateEx(0,
                            AfxRegisterWndClass(0),
                            _T(""),
                            WS_POPUP,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            NULL,
                            0))
   {
      DebugMsg("Unable to create internal splash screen window\n");
      return false;
   }

   if (!CWnd::CreateEx(0,
                       AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
                       _T(""),
                       WS_POPUP | WS_VISIBLE,
                       0,
                       0,
                       m_width,
                       m_height,
                       m_wndOwner.GetSafeHwnd(),
                       NULL))
   {
      DebugMsg("Unable to create splash screen window\n");
      return false;
   }

   return true;
}

///////////////////////////////////////

int cSplashWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   if (CWnd::OnCreate(lpCreateStruct) == -1)
      return -1;

   CenterWindow();

   return 0;
}

///////////////////////////////////////

void cSplashWnd::OnPaint()
{
   CPaintDC dc(this);

   if (m_Bitmap.GetSafeHandle() != NULL)
   {
      CDC memDC;
      if (memDC.CreateCompatibleDC(&dc))
      {
         CBitmap * pOldBitmap = memDC.SelectObject(&m_Bitmap);
         dc.BitBlt(0, 0, m_width, m_height, &memDC, 0, 0, SRCCOPY);
         memDC.SelectObject(pOldBitmap);
      }
   }
#ifdef _DEBUG
   else
   {
      CRect rect;
      GetClientRect(rect);
      dc.FillSolidRect(rect, RGB(255,0,255));
   }
#endif
}

///////////////////////////////////////

void cSplashWnd::OnClose() 
{
   if (m_bOKToClose) 
   {
      CWnd::OnClose();
   }
}

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplashThread
//

///////////////////////////////////////

BEGIN_MESSAGE_MAP(cSplashThread, CWinThread)
	//{{AFX_MSG_MAP(cSplashThread)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///////////////////////////////////////

IMPLEMENT_DYNCREATE_EX(cSplashThread, CWinThread)

///////////////////////////////////////

cSplashThread::cSplashThread()
{
}

///////////////////////////////////////

cSplashThread::~cSplashThread()
{
}

///////////////////////////////////////

HBITMAP cSplashThread::SetBitmap(HBITMAP hBitmap)
{
   return m_splashWnd.SetBitmap(hBitmap);
}

///////////////////////////////////////

BOOL cSplashThread::InitInstance()
{
   ASSERT_VALID(AfxGetApp());

   // Attach this thread's UI state to the main one. This will ensure that 
   // the activation state is managed consistenly across the two threads.
   if (!AttachThreadInput(m_nThreadID, AfxGetApp()->m_nThreadID, TRUE))
   {
      DebugMsg1("AttachThreadInput failed, last error is %d\n", GetLastError());
   }

   if (!m_splashWnd.Create())
   {
      return false;
   }

   m_pMainWnd = &m_splashWnd;

   return TRUE;
}

///////////////////////////////////////

void cSplashThread::HideSplash()
{
   // Wait until the splash window has been created before trying to close it
   while (!m_splashWnd.GetSafeHwnd())
   {
   }

   m_splashWnd.SetOKToClose();
   m_splashWnd.SendMessage(WM_CLOSE);
}

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorSplashScreen
//

///////////////////////////////////////

cEditorSplashScreen::cEditorSplashScreen()
 : m_pSplashThread(NULL)
{
}

///////////////////////////////////////

cEditorSplashScreen::~cEditorSplashScreen()
{
   if (m_pSplashThread != NULL)
   {
      m_pSplashThread->HideSplash();
   }
}

///////////////////////////////////////

tResult cEditorSplashScreen::Create(const tChar * pszBitmap, uint delay)
{
   Assert(pszBitmap != NULL);
   Assert(m_pSplashThread == NULL);

   HBITMAP hSplashBitmap = NULL;
   if (::LoadBitmap(pszBitmap, &hSplashBitmap))
   {
      m_pSplashThread = DYNAMIC_DOWNCAST(cSplashThread, AfxBeginThread(
         RUNTIME_CLASS(cSplashThread), THREAD_PRIORITY_NORMAL, CREATE_SUSPENDED));

      if (m_pSplashThread)
      {
         ASSERT_VALID(m_pSplashThread);

         m_pSplashThread->SetBitmap(hSplashBitmap);
         m_pSplashThread->ResumeThread();

         if (delay > 0)
         {
            ThreadSleep(delay);
         }

         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult EditorSplashScreenCreate(IEditorSplashScreen * * ppEditorSplashScreen)
{
   if (ppEditorSplashScreen == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<cEditorSplashScreen> p(new cEditorSplashScreen);
   if (!p)
   {
      return E_OUTOFMEMORY;
   }

   cStr splashImage;
   if (ConfigGet("splash_image", &splashImage) == S_OK)
   {
      int splashDelay = 0;
      ConfigGet("splash_delay_ms", &splashDelay);

      if (FAILED(p->Create(splashImage.c_str(), splashDelay)))
      {
         return E_FAIL;
      }

      *ppEditorSplashScreen = CTAddRef(static_cast<IEditorSplashScreen*>(p));
      return S_OK;
   }

   return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
