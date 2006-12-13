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
   };

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorSelectTool
   //

   [ToolPaletteItem(Label="selectToolName", Image="select", Group="standardGroupName")]
   ref class EditorSelectTool : public EditorTool
   {
   public:
      EditorDocumentCommandArray ^ OnMouseClick(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);
   };

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorPlaceEntityTool
   //

   [ToolPaletteItem(Label="placeEntityToolName", Image="placeentity", Group="standardGroupName")]
   ref class EditorPlaceEntityTool : public EditorTool
   {
   public:
      EditorDocumentCommandArray ^ OnMouseClick(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);
   };

} // namespace ManagedEditor

#endif // INCLUDED_EDITORTOOLS_H
