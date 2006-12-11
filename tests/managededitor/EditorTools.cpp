///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "EditorTools.h"
#include "EditorDocCmds.h"

#include "engine/entityapi.h"
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
   // CLASS: EditorSelectTool
   //

   void EditorSelectTool::OnMouseClick(System::Windows::Forms::MouseEventArgs ^ e)
   {
      UseGlobal(Renderer);
      UseGlobal(EntityManager);

      cRay pickRay;
      if (pRenderer->GenerateScreenPickRay(e->X, e->Y, &pickRay) == S_OK)
      {
         if (e->Button == System::Windows::Forms::MouseButtons::Left)
         {
            cAutoIPtr<IEntity> pEntity;
            if (pEntityManager->RayCast(pickRay, &pEntity) == S_OK)
            {
            }
            //else
            //{
            //   tVec3 location;
            //   if (GetTerrainLocation(pickRay, &location))
            //   {
            //      UseGlobal(EntityManager);
            //      pEntityManager->SpawnEntity(m_entity.c_str(), m_position);
            //   }
            //}
         }
      }
   }

   void EditorSelectTool::OnMouseHover(System::Drawing::Point location)
   {
      UseGlobal(Renderer);

      cRay pickRay;
      if (pRenderer->GenerateScreenPickRay(location.X, location.Y, &pickRay) == S_OK)
      {
         tVec3 location;
         if (GetTerrainLocation(pickRay, &location))
         {
            cStr temp;
            Sprintf(&temp, _T("Hit <%.2f, %.2f, %.2f>"), location.x, location.y, location.z);
         }
      }
   }


   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorPlaceEntityTool
   //

   void EditorPlaceEntityTool::OnMouseClick(System::Windows::Forms::MouseEventArgs ^ e)
   {
      UseGlobal(Renderer);
      UseGlobal(EntityManager);

      cRay pickRay;
      if (pRenderer->GenerateScreenPickRay(e->X, e->Y, &pickRay) == S_OK)
      {
         if (e->Button == System::Windows::Forms::MouseButtons::Left)
         {
            cAutoIPtr<IEntity> pEntity;
            if (pEntityManager->RayCast(pickRay, &pEntity) == S_OK)
            {
               // TODO: select hit entity
            }
            else
            {
               tVec3 location;
               if (GetTerrainLocation(pickRay, &location))
               {
                  UseGlobal(EntityManager);
                  pEntityManager->SpawnEntity("zombie", location);
               }
            }
         }
      }
   }

   void EditorPlaceEntityTool::OnMouseHover(System::Drawing::Point location)
   {
   }

} // namespace ManagedEditor
