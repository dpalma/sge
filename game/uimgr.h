///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_UIMGR_H
#define INCLUDED_UIMGR_H

#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IUIManager);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IUIManager
//

interface IUIManager : IUnknown
{
   virtual void ShowModalDialog(const char * pszXmlFile) = 0;

   virtual void Render() = 0;
};

IUIManager * UIManagerCreate();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_UIMGR_H
