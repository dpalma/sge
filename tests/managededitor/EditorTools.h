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

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorTool
   //

   ref class EditorTool abstract
   {
   public:
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
      virtual void OnMouseHover(System::Drawing::Point location) override;
   };

} // namespace ManagedEditor

#endif // INCLUDED_EDITORTOOLS_H
