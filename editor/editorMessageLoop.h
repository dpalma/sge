/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORMESSAGELOOP_H)
#define INCLUDED_EDITORMESSAGELOOP_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorMessageLoop
//
// The CMessageLoop::Run() method over-ridden here is not virtual, but
// it is only called once and it is called through a cEditorMessageLoop
// object (not polymorphically).

class cEditorMessageLoop : public CMessageLoop
{
public:
   int Run();
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORMESSAGELOOP_H)
