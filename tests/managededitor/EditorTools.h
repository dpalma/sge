///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORTOOLS_H
#define INCLUDED_EDITORTOOLS_H

#include "ToolPalette.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// NAMESPACE: ManagedEditor
//

namespace ManagedEditor
{
   ref class EditorDocumentCommand;
   typedef array<EditorDocumentCommand ^> EditorDocumentCommandArray;

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorTool
   //

   ref class EditorTool abstract
   {
   public:
      virtual EditorDocumentCommandArray ^ OnMouseClick(System::Windows::Forms::MouseEventArgs ^ e) = 0;
      virtual void OnMouseHover(System::Drawing::Point location) = 0;
   };

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorSelectTool
   //

   [ToolPaletteItem(Label="selectToolName", Image="select", Group="standardGroupName")]
   ref class EditorSelectTool : public EditorTool
   {
   public:
      virtual EditorDocumentCommandArray ^ OnMouseClick(System::Windows::Forms::MouseEventArgs ^ e) override;
      virtual void OnMouseHover(System::Drawing::Point location) override;
   };

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorPlaceEntityTool
   //

   [ToolPaletteItem(Label="placeEntityToolName", Image="placeentity", Group="standardGroupName")]
   ref class EditorPlaceEntityTool : public EditorTool
   {
   public:
      virtual EditorDocumentCommandArray ^ OnMouseClick(System::Windows::Forms::MouseEventArgs ^ e) override;
      virtual void OnMouseHover(System::Drawing::Point location) override;
   };

} // namespace ManagedEditor

#endif // INCLUDED_EDITORTOOLS_H
