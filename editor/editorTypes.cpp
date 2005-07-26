/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTypes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorKeyEvent
//

cEditorKeyEvent::cEditorKeyEvent(WPARAM wParam, LPARAM lParam)
 : m_char(wParam), m_repeats(LOWORD(lParam)), m_flags(HIWORD(lParam))
{
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorMouseEvent
//

cEditorMouseEvent::cEditorMouseEvent(WPARAM wParam, LPARAM lParam)
 : m_flags(wParam), m_point(lParam)
{
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorMouseWheelEvent
//

cEditorMouseWheelEvent::cEditorMouseWheelEvent(WPARAM wParam, LPARAM lParam)
 : cEditorMouseEvent(LOWORD(wParam), lParam), m_zDelta(HIWORD(wParam))
{
}


/////////////////////////////////////////////////////////////////////////////
