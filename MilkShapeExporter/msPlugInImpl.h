/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MSPLUGINIMPL_H
#define INCLUDED_MSPLUGINIMPL_H

#include "msPlugIn.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPlugIn

class cPlugIn : public cMsPlugIn
{
public:
   cPlugIn();
   virtual ~cPlugIn();

   virtual int GetType();
   virtual const char * GetTitle();
   virtual int Execute(msModel * pModel);

private:
   CString m_title;
};

/////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_MSPLUGINIMPL_H
