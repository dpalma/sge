///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CONFIGSTORE_H
#define INCLUDED_CONFIGSTORE_H

#include "configapi.h"
#include "filespec.h"
#include "str.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTextConfigStore
//

class cTextConfigStore : public cComObject<IMPLEMENTS(IConfigStore)>
{
public:
   cTextConfigStore(const cFileSpec & file);

   virtual tResult Load(IConfig * pConfig);
   virtual tResult Save(IConfig * pConfig);
   virtual tResult MergeSave(IConfig * pConfig);

private:
   cFileSpec m_file;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cIniSectionConfigStore
//

class cIniSectionConfigStore : public cComObject<IMPLEMENTS(IConfigStore)>
{
public:
   cIniSectionConfigStore(const cFileSpec & file, const char * pszSection);

   virtual tResult Load(IConfig * pConfig);
   virtual tResult Save(IConfig * pConfig);
   virtual tResult MergeSave(IConfig * pConfig);

private:
   cFileSpec m_file;
   cStr m_section;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CONFIGSTORE_H
