///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorapi.h"
#include "editorMessageLoop.h"

#include "techtime.h"
#include "globalobj.h"

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////
//
// CLASS cEditorMessageLoop
//

///////////////////////////////////////

int cEditorMessageLoop::Run()
{
   bool bQuit = false;
	bool bDoIdle = true;
	int nIdleCount = 0;

   double timeLastFrame = TimeGetSecs();
   double time = timeLastFrame;

   UseGlobal(EditorApp);

	while (!bQuit)
	{
      double timeEnterIdle = TimeGetSecs();
		while (bDoIdle && ((TimeGetSecs() - timeEnterIdle) < 0.05))
		{
			if (!OnIdle(nIdleCount++))
         {
				bDoIdle = false;
         }
		}

      // Pump messages while available
      while (::PeekMessage(&m_msg, NULL, NULL, NULL, PM_NOREMOVE))
      {
		   int result = ::GetMessage(&m_msg, NULL, 0, 0);

		   if (result == -1)
		   {
			   WarnMsg("::GetMessage returned -1 (error)\n");
			   continue; // error, don't process
		   }
		   else if (!result)
		   {
			   bQuit = true; // WM_QUIT, exit message loop
		   }

		   if (!PreTranslateMessage(&m_msg))
		   {
			   ::TranslateMessage(&m_msg);
			   ::DispatchMessage(&m_msg);
		   }

		   if (IsIdleMessage(&m_msg))
		   {
			   bDoIdle = true;
			   nIdleCount = 0;
		   }
      }

      double elapsed = time - timeLastFrame;

      pEditorApp->CallLoopClients(time, elapsed);

      timeLastFrame = time;
      time = TimeGetSecs();
	}

	return (int)m_msg.wParam;
}

/////////////////////////////////////////////////////////////////////////////
