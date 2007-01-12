///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORTOOLS_H
#define INCLUDED_EDITORTOOLS_H

#include "ToolPalette.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_HANDLE(HTERRAINVERTEX);

///////////////////////////////////////////////////////////////////////////////
//
// NAMESPACE: ManagedEditor
//

namespace ManagedEditor
{
   ref class EditorDocument;
   ref class EditorDocumentCommand;
   ref class EditorCompositeDocumentCommand;
   typedef array<EditorDocumentCommand ^> EditorDocumentCommandArray;

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorTool
   //

   ref class EditorTool abstract
   {
   };

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorSelectTool
   //

   [ToolPaletteItem(Label="selectToolName", Image="select", Group="standardGroupName")]
   ref class EditorSelectTool : public EditorTool
   {
   public:
      EditorDocumentCommandArray ^ OnMouseClick(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e, EditorDocument ^ doc);
   };

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorPlaceEntityTool
   //

   [ToolPaletteItem(Label="placeEntityToolName", Image="placeentity", Group="standardGroupName")]
   ref class EditorPlaceEntityTool : public EditorTool
   {
   public:
      EditorDocumentCommandArray ^ OnMouseClick(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e, EditorDocument ^ doc);
   };


   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorTerrainElevationTool
   //

   [ToolPaletteItem(Label="terrainElevationToolName", Image="TerrainElevation", Group="standardGroupName")]
   ref class EditorTerrainElevationTool : public EditorTool
   {
   public:
      EditorTerrainElevationTool();

      EditorDocumentCommandArray ^ OnMouseDown(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e, EditorDocument ^ doc);
      EditorDocumentCommandArray ^ OnMouseUp(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e, EditorDocument ^ doc);
      EditorDocumentCommandArray ^ OnMouseMove(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e, EditorDocument ^ doc);

      void OnMouseCaptureChanged(System::Object ^ sender, System::EventArgs ^ e);

   private:
      EditorDocumentCommand ^ m_command;
      HTERRAINVERTEX m_hHitVertex;
      System::Drawing::Point m_lastDragPoint;
      float m_elevDelta;
   };


   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorTerrainPlateauTool
   //

   [ToolPaletteItem(Label="terrainPlateauToolName", Image="TerrainPlateau", Group="standardGroupName")]
   ref class EditorTerrainPlateauTool : public EditorTool
   {
   public:
      EditorTerrainPlateauTool();

      EditorDocumentCommandArray ^ OnMouseDown(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e, EditorDocument ^ doc);
      EditorDocumentCommandArray ^ OnMouseUp(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e, EditorDocument ^ doc);
      EditorDocumentCommandArray ^ OnMouseMove(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e, EditorDocument ^ doc);

      void OnMouseCaptureChanged(System::Object ^ sender, System::EventArgs ^ e);

   private:
      EditorCompositeDocumentCommand ^ m_command;
      System::Collections::ArrayList ^ m_hitVertices;
      float m_elevation;
   };

} // namespace ManagedEditor

#endif // INCLUDED_EDITORTOOLS_H
