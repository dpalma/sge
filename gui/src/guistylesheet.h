///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUISTYLESHEET_H
#define INCLUDED_GUISTYLESHEET_H

#include "guistyleapi.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyleSelector
//

class cGUIStyleSelector
{
   bool operator ==(const cGUIStyleSelector & other) const;
   bool operator >(const cGUIStyleSelector & other) const;

public:
   explicit cGUIStyleSelector(const tChar * pszSelector);
   cGUIStyleSelector(const tChar * pszType, const tChar * pszClass);
   cGUIStyleSelector(const cGUIStyleSelector & other);
   const cGUIStyleSelector & operator =(const cGUIStyleSelector & other);
   bool operator <(const cGUIStyleSelector & other) const;
private:
   cStr m_type, m_class;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyleSheet
//

class cGUIStyleSheet : public cComObject<IMPLEMENTS(IGUIStyleSheet)>
{
public:
   cGUIStyleSheet();
   ~cGUIStyleSheet();

   virtual tResult AddRule(const tChar * pszSelector, IGUIStyle * pStyle);
   virtual tResult GetStyle(const tChar * pszType, const tChar * pszClass, IGUIStyle * * ppStyle) const;

private:
   typedef std::map<cGUIStyleSelector, IGUIStyle*> tStyleMap;
   std::map<cGUIStyleSelector, IGUIStyle*> m_styleMap;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUISTYLESHEET_H
