/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "msPlugInImpl.h"
#include "resource.h"

#include "msLib.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPlugIn
//

///////////////////////////////////////

cPlugIn::cPlugIn()
{
   VERIFY(m_title.LoadString(AFX_IDS_APP_TITLE));
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
   return m_title;
}

///////////////////////////////////////

int cPlugIn::Execute(msModel * pModel)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   if (pModel == NULL)
   {
      return -1;
   }

   int nMeshes = msModel_GetMeshCount(pModel);

   int nMaterials = msModel_GetMaterialCount(pModel);

   if (nMeshes == 0 && nMaterials == 0)
   {
      AfxMessageBox(IDS_ERR_NOTHINGTOEXPORT);
      return -1;
   }

   // TODO

   // Don't forget to destroy the model after an export
   msModel_Destroy (pModel);

   return 0;
}

///////////////////////////////////////

cMsPlugIn * CreatePlugIn()
{
   return new cPlugIn;
}

/////////////////////////////////////////////////////////////////////////////
