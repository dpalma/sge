///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIPARSE_H
#define INCLUDED_GUIPARSE_H

#include "combase.h"
#include "guitypes.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

const tChar * SkipSpaceFwd(const tChar * psz);
const tChar * SkipSpaceBack(const tChar * psz);
const tChar * SkipSingleComment(const tChar * psz);

tResult GUIParseStyleDimension(const tChar * psz, int * pDimension, eGUIDimensionSpec * pSpec);
tResult GUIParseStyleFontSize(const tChar * psz, int * pSize, eGUIFontSizeType * pSizeType);
tResult GUIParseBool(const tChar * psz, bool * pBool);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIPARSE_H
