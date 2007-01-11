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

   EditorDocumentCommandArray ^ EditorSelectTool::OnMouseClick(System::Object ^ sender,
                                                               System::Windows::Forms::MouseEventArgs ^ e,
                                                               EditorDocument ^ doc)
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
            else
            {
               return gcnew EditorDocumentCommandArray{gcnew SelectCommand};
            }
         }
      }

      return nullptr;
   }


   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorPlaceEntityTool
   //

   EditorDocumentCommandArray ^ EditorPlaceEntityTool::OnMouseClick(System::Object ^ sender,
                                                                    System::Windows::Forms::MouseEventArgs ^ e,
                                                                    EditorDocument ^ doc)
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


   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorTerrainElevationTool
   //

   EditorTerrainElevationTool::EditorTerrainElevationTool()
    : m_command(nullptr)
    , m_hHitVertex(INVALID_HTERRAINVERTEX)
    , m_elevDelta(0)
   {
   }

   EditorDocumentCommandArray ^ EditorTerrainElevationTool::OnMouseDown(System::Object ^ sender,
                                                                        System::Windows::Forms::MouseEventArgs ^ e,
                                                                        EditorDocument ^ doc)
   {
      // Should have hit-tested for a vertex in OnMouseMove
      if (m_hHitVertex != INVALID_HTERRAINVERTEX)
      {
         UseGlobal(TerrainRenderer);
         pTerrainRenderer->EnableBlending(false);

         m_lastDragPoint.X = e->X;
         m_lastDragPoint.Y = e->Y;
      }

      System::Windows::Forms::Control ^ control = dynamic_cast<System::Windows::Forms::Control^>(sender);

      if (control)
      {
         control->MouseCaptureChanged += gcnew System::EventHandler(this, &EditorTerrainElevationTool::OnMouseCaptureChanged);
      }

      return nullptr;
   }

   EditorDocumentCommandArray ^ EditorTerrainElevationTool::OnMouseUp(System::Object ^ sender,
                                                                      System::Windows::Forms::MouseEventArgs ^ e,
                                                                      EditorDocument ^ doc)
   {
      UseGlobal(TerrainRenderer);
      pTerrainRenderer->EnableBlending(true);

      System::Windows::Forms::Control ^ control = dynamic_cast<System::Windows::Forms::Control^>(sender);

      if (control)
      {
         control->MouseCaptureChanged -= gcnew System::EventHandler(this, &EditorTerrainElevationTool::OnMouseCaptureChanged);
      }

      if (doc && m_command)
      {
         doc->AddDocumentCommand(m_command, false);
      }

      return nullptr;
   }

   EditorDocumentCommandArray ^ EditorTerrainElevationTool::OnMouseMove(System::Object ^ sender,
                                                                        System::Windows::Forms::MouseEventArgs ^ e,
                                                                        EditorDocument ^ doc)
   {
      UseGlobal(Renderer);
      UseGlobal(TerrainModel);
      UseGlobal(TerrainRenderer);

      HTERRAINVERTEX hHitVertex = INVALID_HTERRAINVERTEX;

      cRay pickRay;
      if (pRenderer->GenerateScreenPickRay(e->X, e->Y, &pickRay) == S_OK)
      {
         pTerrainModel->GetVertexFromHitTest(pickRay, &hHitVertex);
      }

      System::Windows::Forms::Control ^ control = dynamic_cast<System::Windows::Forms::Control^>(sender);

      if (control && control->Capture)
      {
         if (m_hHitVertex != INVALID_HTERRAINVERTEX)
         {
            m_elevDelta -= static_cast<float>(e->Y - m_lastDragPoint.Y);

            m_lastDragPoint.X = e->X;
            m_lastDragPoint.Y = e->Y;

            if (m_command)
            {
               m_command->Undo();
               m_command = nullptr;
            }

            m_command = static_cast<EditorDocumentCommand ^>(
               gcnew ChangeTerrainElevationCommand(m_hHitVertex, m_elevDelta));
            m_command->Do();
         }
      }
      else
      {
         m_hHitVertex = hHitVertex;
         if (hHitVertex != INVALID_HTERRAINVERTEX)
         {
            pTerrainRenderer->HighlightTerrainVertex(hHitVertex);
         }
         else
         {
            pTerrainRenderer->ClearHighlight();
         }
      }

      return nullptr;
   }

   void EditorTerrainElevationTool::OnMouseCaptureChanged(System::Object ^ sender, System::EventArgs ^ e)
   {
      UseGlobal(TerrainRenderer);
      pTerrainRenderer->EnableBlending(true);

      System::Windows::Forms::Control ^ control = dynamic_cast<System::Windows::Forms::Control^>(sender);

      if (control)
      {
         control->MouseCaptureChanged -= gcnew System::EventHandler(this, &EditorTerrainElevationTool::OnMouseCaptureChanged);
      }
   }

} // namespace ManagedEditor
