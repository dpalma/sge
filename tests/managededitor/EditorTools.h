///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORTOOLS_H
#define INCLUDED_EDITORTOOLS_H

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
   // CLASS: EditorToolGroup
   //

   [System::AttributeUsage(System::AttributeTargets::Class, AllowMultiple=false)]
   ref class EditorToolGroup : public System::Attribute
   {
   public:
      EditorToolGroup();

      property System::String ^ Group
      {
         System::String ^ get();
         void set(System::String ^);
      }

   private:
      System::String ^ m_group;
   };

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

   [EditorToolGroup(Group = "standardGroupName")]
   ref class EditorSelectTool : public EditorTool
   {
   public:
      virtual void OnMouseHover(System::Drawing::Point location) override;
   };

} // namespace ManagedEditor

#endif // INCLUDED_EDITORTOOLS_H
