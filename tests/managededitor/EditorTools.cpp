///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "EditorTools.h"

#include "engine/terrainapi.h"
#include "render/renderapi.h"
#include "tech/globalobj.h"
#include "tech/ray.h"


///////////////////////////////////////////////////////////////////////////////
//
// NAMESPACE: ManagedEditor
//

namespace ManagedEditor
{
   static bool GetTerrainLocation(const cRay & ray, tVec3 * pLocation)
   {
      HTERRAINQUAD hQuad;
      UseGlobal(TerrainModel);
      if (pTerrainModel->GetQuadFromHitTest(ray, &hQuad) == S_OK)
      {
         tVec3 corners[4];
         if (pTerrainModel->GetQuadCorners(hQuad, corners) == S_OK)
         {
            if (ray.IntersectsTriangle(corners[0], corners[3], corners[2], pLocation)
               || ray.IntersectsTriangle(corners[2], corners[1], corners[0], pLocation))
            {
               return true;
            }
         }
      }
      return false;
   }

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorToolGroup
   //

   EditorToolGroup::EditorToolGroup()
    : m_group(nullptr)
   {
   }

   System::String ^ EditorToolGroup::Group::get()
   {
      return m_group;
   }

   void EditorToolGroup::Group::set(System::String ^ group)
   {
      m_group = group;
   }


   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorSelectTool
   //

   void EditorSelectTool::OnMouseHover(System::Drawing::Point location)
   {
      UseGlobal(Renderer);

      cRay pickRay;
      if (pRenderer->GenerateScreenPickRay(location.X, location.Y, &pickRay) == S_OK)
      {
         tVec3 intersect;
         if (GetTerrainLocation(pickRay, &intersect))
         {
            cStr temp;
            Sprintf(&temp, _T("Hit <%.2f, %.2f, %.2f>"), intersect.x, intersect.y, intersect.z);
         }
      }
   }

} // namespace ManagedEditor
