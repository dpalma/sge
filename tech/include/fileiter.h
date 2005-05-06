///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FILEITER_H
#define INCLUDED_FILEITER_H

#include "techdll.h"

#ifdef _MSC_VER
#pragma once
#endif

class cFileSpec;

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
   virtual void Begin(const cFileSpec & spec) = 0;
   virtual bool Next(cFileSpec * pFileSpec, uint * pAttribs) = 0;
   virtual void End() = 0;
};

///////////////////////////////////////

TECH_API cFileIter * FileIterCreate();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FILEITER_H
