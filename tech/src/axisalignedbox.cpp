///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "axisalignedbox.h"

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAxisAlignedBox
//

////////////////////////////////////////

cAxisAlignedBox::cAxisAlignedBox()
{
}

////////////////////////////////////////

cAxisAlignedBox::cAxisAlignedBox(const tVec3 & mins, const tVec3 & maxs)
 : m_mins(mins)
 , m_maxs(maxs)
{
}

////////////////////////////////////////

cAxisAlignedBox::cAxisAlignedBox(const cAxisAlignedBox & other)
 : m_mins(other.m_mins)
 , m_maxs(other.m_maxs)
{
}

////////////////////////////////////////

cAxisAlignedBox::~cAxisAlignedBox()
{
}

////////////////////////////////////////

const cAxisAlignedBox & cAxisAlignedBox::operator =(const cAxisAlignedBox & other)
{
   m_mins = other.m_mins;
   m_maxs = other.m_maxs;
   return *this;
}

////////////////////////////////////////

void cAxisAlignedBox::GetCentroid(tVec3 * pCentroid) const
{
   if (pCentroid != NULL)
   {
      *pCentroid = tVec3((m_mins.x + m_maxs.x) / 2, (m_mins.y + m_maxs.y) / 2, (m_mins.z + m_maxs.z) / 2);
   }
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cAxisAlignedBoxTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cAxisAlignedBoxTests);
   CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cAxisAlignedBoxTests);

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
