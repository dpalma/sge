/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_MENUITEMINFO_H)
#define INCLUDED_MENUITEMINFO_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMenuItemInfoEx
//

class cMenuItemInfoEx : public CMenuItemInfo
{
public:
   cMenuItemInfoEx(uint id, const tChar * pszString);
   cMenuItemInfoEx(uint id, const tChar * pszString, uint state);
};

////////////////////////////////////////

cMenuItemInfoEx::cMenuItemInfoEx(uint id, const tChar * pszString)
{
#ifdef MIIM_STRING
   fMask = MIIM_ID | MIIM_STRING;
#else
   fMask = MIIM_ID | MIIM_TYPE;
   fType = MFT_STRING;
#endif
   wID = id;
   dwTypeData = const_cast<LPSTR>(pszString);
   cch = lstrlen(pszString);
}

////////////////////////////////////////

cMenuItemInfoEx::cMenuItemInfoEx(uint id, const tChar * pszString, uint state)
{
#ifdef MIIM_STRING
   fMask = MIIM_ID | MIIM_STATE | MIIM_STRING;
#else
   fMask = MIIM_ID | MIIM_STATE | MIIM_TYPE;
   fType = MFT_STRING;
#endif
   wID = id;
   fState = state;
   dwTypeData = const_cast<LPSTR>(pszString);
   cch = lstrlen(pszString);
}

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MENUITEMINFO_H
