/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "msPlugInImpl.h"
#include "Exporter.h"
#include "resource.h"

#include "comtools.h"

#include "msLib.h"

#include <windows.h>

#include "dbgalloc.h" // must be last header

extern bool ShowExportPreviewDlg(cExporter * pExporter);

/////////////////////////////////////////////////////////////////////////////

static tResult LoadString(HINSTANCE hInstance, uint stringId, std::string * pString)
{
   if (pString == NULL)
   {
      return E_POINTER;
   }

   HRSRC hRsrc = FindResource(hInstance, MAKEINTRESOURCE(((stringId >> 4) + 1)), RT_STRING);
   if (hRsrc == NULL)
   {
      return E_FAIL;
   }

   DWORD size = SizeofResource(hInstance, hRsrc);
   if (size == 0)
   {
      pString->clear();
      return S_OK;
   }

   if (size > 4096)
   {
      ErrorMsg1("Sanity check failed loading resource string (size = %d)\n", size);
      return E_FAIL;
   }

   size += 1; // allow for null-terminator

   char * pszTemp = static_cast<char*>(_alloca(size));
   if (LoadStringA(hInstance, stringId, pszTemp, size) > 0)
   {
      pString->assign(pszTemp);
      return S_OK;
   }

   return E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////

static tResult ChooseExportFileName(std::string * pFileName)
{
   std::string defaultExt;
   Verify(LoadString(g_hInstance, IDS_EXPORT_DEFAULT_EXTENSION, &defaultExt) == S_OK);

   std::string filter;
   Verify(LoadString(g_hInstance, IDS_EXPORT_FILTER, &filter) == S_OK);

   size_t pos = 0;
   while ((pos = filter.find('|', pos)) != std::string::npos)
   {
      filter[pos] = '\0';
      pos += 1;
   }

   tChar szFile[_MAX_PATH];
   ZeroMemory(szFile, sizeof(szFile));

   OPENFILENAME ofn;
   ZeroMemory(&ofn, sizeof(ofn));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.lpstrFilter = filter.c_str();
   ofn.lpstrFile = szFile;
   ofn.nMaxFile = _countof(szFile);
   ofn.lpstrTitle; // TODO
   ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

   if (!GetSaveFileName(&ofn))
   {
      return S_FALSE;
   }

   pFileName->assign(ofn.lpstrFile);
   return S_OK;
}

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
   if (m_title.empty())
   {
      Verify(LoadString(g_hInstance, IDS_TITLE, &m_title) == S_OK);
   }
   return m_title.c_str();
}

///////////////////////////////////////

int cPlugIn::Execute(msModel * pModel)
{
   if (pModel == NULL)
   {
      return -1;
   }

   int result = -1;

   cExporter exporter(pModel);

   exporter.PreProcess();

   if (ShowExportPreviewDlg(&exporter))
   {
      std::string fileName;
      if (ChooseExportFileName(&fileName) == S_OK)
      {
         result = SUCCEEDED(exporter.ExportMesh(fileName.c_str())) ? 0 : -1;
      }
   }

   return result;
}

/////////////////////////////////////////////////////////////////////////////

cMsPlugIn * CreatePlugIn()
{
   return new cPlugIn;
}

/////////////////////////////////////////////////////////////////////////////
