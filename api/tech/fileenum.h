///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FILEENUM_H
#define INCLUDED_FILEENUM_H

#include "techdll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

class cFileSpec;

F_DECLARE_INTERFACE(IEnumFiles);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEnumFiles
//

////////////////////////////////////////

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

////////////////////////////////////////

interface IEnumFiles : IUnknown
{
   virtual tResult Next(ulong count, cFileSpec * pFileSpecs, uint * pAttribs, ulong * pnElements) = 0;
   virtual tResult Skip(ulong count) = 0;
   virtual tResult Reset() = 0;
   virtual tResult Clone(IEnumFiles * * ppEnum) = 0;
};

////////////////////////////////////////

TECH_API tResult EnumFiles(const cFileSpec & spec, IEnumFiles * * ppEnumFiles);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FILEENUM_H
