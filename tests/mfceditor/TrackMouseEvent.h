/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TRACKMOUSEEVENT_H)
#define INCLUDED_TRACKMOUSEEVENT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cTrackMouseEvent
//

template <class T>
class cTrackMouseEvent
{
public:
   cTrackMouseEvent() : m_bTrackingMouseLeave(false)
   {
   }

   ~cTrackMouseEvent()
   {
   }

   BEGIN_MSG_MAP(cTrackMouseEvent)
      MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
      MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
   END_MSG_MAP()

   LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
	{
      T * pT = static_cast<T*>(this);
      if (!m_bTrackingMouseLeave)
      {
         TRACKMOUSEEVENT tme = {0};
         tme.cbSize = sizeof(TRACKMOUSEEVENT);
         tme.dwFlags = TME_LEAVE;
         tme.hwndTrack = pT->m_hWnd;
         if (_TrackMouseEvent(&tme))
         {
            m_bTrackingMouseLeave = true;
         }
      }
      bHandled = FALSE;
      return 0;
   }

   LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
	{
      m_bTrackingMouseLeave = false;
      bHandled = FALSE;
      return 0;
   }

private:
   bool m_bTrackingMouseLeave;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_TRACKMOUSEEVENT_H)
