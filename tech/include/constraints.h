///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CONSTRAINTS_H
#define INCLUDED_CONSTRAINTS_H

#include "techtypes.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

enum eConstrainWhen
{
   kCW_After,
   kCW_Before,
};

enum eConstrainAgainst
{
   kCA_Guid,
   kCA_Name,
};

///////////////////////////////////////////////////////////////////////////////

struct sConstraint
{
   ////////////////////////////////////

   sConstraint()
   {
      against = kCA_Guid;
      pGUID = NULL;
      when = kCW_After;
   }

   ////////////////////////////////////

   sConstraint(const GUID * pG, eConstrainWhen w)
   {
      against = kCA_Guid;
      pGUID = pG;
      when = w;
   }

   ////////////////////////////////////

   sConstraint(const tChar * pszN, eConstrainWhen w)
   {
      against = kCA_Name;
      pszName = pszN;
      when = w;
   }

   ////////////////////////////////////

   eConstrainAgainst against;
   union
   {
      const GUID * pGUID;
      const tChar * pszName;
   };
   eConstrainWhen when;
};

///////////////////////////////////////////////////////////////////////////////

#define DEFAULT_CONSTRAINTS g_constraints

#define BEGIN_CONSTRAINTS_NAMED(name) \
   static sConstraint name[] = {

#define BEGIN_CONSTRAINTS() \
   BEGIN_CONSTRAINTS_NAMED(DEFAULT_CONSTRAINTS)

#define AFTER_GUID(guid) \
   sConstraint(&guid, kCW_After),

#define BEFORE_GUID(guid) \
   sConstraint(&guid, kCW_Before),

#define AFTER_NAME(name) \
   sConstraint(name, kCW_After),

#define BEFORE_NAME(name) \
   sConstraint(name, kCW_Before),

#define END_CONSTRAINTS() \
   };

#define CONSTRAINTS_NAMED(name) \
   name, _countof(name)

#define CONSTRAINTS() \
   CONSTRAINTS_NAMED(DEFAULT_CONSTRAINTS)

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CONSTRAINTS_H
