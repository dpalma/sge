///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scenemesh.h"
#include "mesh.h"
#include "ggl.h"
#include "gcommon.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneMesh
//

///////////////////////////////////////

cSceneMesh::cSceneMesh()
{
}

///////////////////////////////////////

cSceneMesh::~cSceneMesh()
{
}

///////////////////////////////////////

bool cSceneMesh::SetMesh(const char * pszMesh)
{
   SafeRelease(m_pMesh);

   m_pMesh = MeshLoad(AccessResourceManager(), pszMesh);

   if (m_pMesh != NULL)
   {
      tVec3 maxs, mins;
      m_pMesh->GetAABB(&maxs, &mins);

      tVec3 diff = maxs - mins;

      m_centroid = mins + (diff * 0.5f);

      m_bounds.SetRadius(0.5f * Max(diff.x, Max(diff.y, diff.z)));

      return true;
   }
   
   return false;
}

///////////////////////////////////////

IMesh * cSceneMesh::AccessMesh()
{
   return m_pMesh;
}

///////////////////////////////////////

void cSceneMesh::Render()
{
   if (m_pMesh != NULL)
   {
      glPushMatrix();
      glMultMatrixf(GetTransform().m);

      m_pMesh->Render();

      tChildren::iterator iter;
      for (iter = m_children.begin(); iter != m_children.end(); iter++)
      {
         (*iter)->Render();
      }
      
      glPopMatrix();
   }
}

///////////////////////////////////////

const cBoundingVolume * cSceneMesh::GetBoundingVolume() const
{
   m_bounds.SetCenter(GetTranslation() + m_centroid);
   return &m_bounds;
}

///////////////////////////////////////////////////////////////////////////////
