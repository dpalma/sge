///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FILEITER_H
#define INCLUDED_FILEITER_H

#include "techdll.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// ENUM: eFileAttribute
//

enum eFileAttribute
{
   kFA_ReadOnly                  = (1 << 0),
   kFA_Hidden                    = (1 << 1),
   kFA_System                    = (1 << 2),
   kFA_Directory                 = (1 << 4),
   kFA_Archive                   = (1 << 5),
   kFA_Normal                    = (1 << 7),
   kFA_Temporary                 = (1 << 8),
   kFA_Compressed                = (1 << 11),
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFileIter
//

class cFileIter
{
public:
   virtual ~cFileIter() = 0;
   virtual void IterBegin(const char * pszSpec) = 0;
   virtual BOOL IterNext(char * pszResult, uint maxResult, uint * pAttribs) = 0;
   virtual void IterEnd() = 0;
};

///////////////////////////////////////

TECH_API cFileIter * FileIterCreate();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FILEITER_H
