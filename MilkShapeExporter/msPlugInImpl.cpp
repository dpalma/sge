/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "msPlugInImpl.h"
#include "msPlugInExportDlg.h"

#include "msLib.h"

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPlugIn
//

///////////////////////////////////////

cPlugIn::cPlugIn()
{
}

///////////////////////////////////////

cPlugIn::~cPlugIn()
{
}

///////////////////////////////////////

int cPlugIn::GetType()
{
   return cMsPlugIn::eTypeExport;
}

///////////////////////////////////////

const char * cPlugIn::GetTitle()
{
   if (m_title.IsEmpty())
   {
      Verify(m_title.LoadString(IDS_TITLE));
   }
   return m_title;
}

///////////////////////////////////////

int cPlugIn::Execute(msModel * pModel)
{
   if (pModel == NULL)
   {
      return -1;
   }

   cMsPlugInExportDlg dlg(pModel);

   int result = dlg.DoModal();

   msModel_Destroy (pModel);

   return (result == IDOK) ? 0 : -1;
}

///////////////////////////////////////

cMsPlugIn * CreatePlugIn()
{
   return new cPlugIn;
}

/////////////////////////////////////////////////////////////////////////////
