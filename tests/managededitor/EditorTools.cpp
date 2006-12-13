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


   static EditorDocumentCommand ^ MakeSelectCommand(const cRay & pickRay)
   {
      XYZ<float> ^ rayOrigin = gcnew XYZ<float>(
         pickRay.GetOrigin().x,
         pickRay.GetOrigin().y,
         pickRay.GetOrigin().z);
      XYZ<float> ^ rayDirection = gcnew XYZ<float>(
         pickRay.GetDirection().x,
         pickRay.GetDirection().y,
         pickRay.GetDirection().z);
      return gcnew SelectCommand(rayOrigin, rayDirection);
   }


   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorSelectTool
   //

   EditorDocumentCommandArray ^ EditorSelectTool::OnMouseClick(System::Windows::Forms::MouseEventArgs ^ e)
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
               return gcnew EditorDocumentCommandArray{MakeSelectCommand(pickRay)};
            }
         }
      }

      return nullptr;
   }

   void EditorSelectTool::OnMouseHover(System::Drawing::Point location)
   {
   }


   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorPlaceEntityTool
   //

   EditorDocumentCommandArray ^ EditorPlaceEntityTool::OnMouseClick(System::Windows::Forms::MouseEventArgs ^ e)
   {
      UseGlobal(Renderer);
      UseGlobal(EntityManager);
      UseGlobal(TerrainModel);

      cRay pickRay;
      if (pRenderer->GenerateScreenPickRay(e->X, e->Y, &pickRay) == S_OK)
      {
         if (e->Button == System::Windows::Forms::MouseButtons::Left)
         {
            cAutoIPtr<IEntity> pEntity;
            if (pEntityManager->RayCast(pickRay, &pEntity) == S_OK)
            {
               return gcnew EditorDocumentCommandArray{MakeSelectCommand(pickRay)};
            }
            else
            {
               tVec3 location;
               if (GetTerrainLocation(pickRay, &location))
               {
                  cTerrainSettings terrainSettings;
                  Verify(pTerrainModel->GetTerrainSettings(&terrainSettings) == S_OK);

                  float nx = location.x / (terrainSettings.GetTileCountX() * terrainSettings.GetTileSize());
                  float nz = location.z / (terrainSettings.GetTileCountZ() * terrainSettings.GetTileSize());

                  return gcnew EditorDocumentCommandArray{gcnew PlaceEntityCommand("zombie", nx, nz)};
               }
            }
         }
      }

      return nullptr;
   }

   void EditorPlaceEntityTool::OnMouseHover(System::Drawing::Point location)
   {
   }

} // namespace ManagedEditor
