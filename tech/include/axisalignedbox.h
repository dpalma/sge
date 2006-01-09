///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AXISALIGNEDBOX_H
#define INCLUDED_AXISALIGNEDBOX_H

#include "techdll.h"

#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAxisAlignedBox
//

class TECH_API cAxisAlignedBox
{
public:
   cAxisAlignedBox();
   cAxisAlignedBox(const tVec3 & mins, const tVec3 & maxs);
   cAxisAlignedBox(const cAxisAlignedBox & other);
   ~cAxisAlignedBox();

   const cAxisAlignedBox & operator =(const cAxisAlignedBox & other);

   const tVec3 & GetMins() const;
   const tVec3 & GetMaxs() const;

   void GetCentroid(tVec3 * pCentroid) const;

private:
   tVec3 m_mins, m_maxs;
};

////////////////////////////////////////

inline const tVec3 & cAxisAlignedBox::GetMins() const
{
   return m_mins;
}

////////////////////////////////////////

inline const tVec3 & cAxisAlignedBox::GetMaxs() const
{
   return m_maxs;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AXISALIGNEDBOX_H
