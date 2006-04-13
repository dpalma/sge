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
      EditorToolGroup(System::String ^ groupName);

      property System::String ^ Group
      {
         System::String ^ get();
      }

   private:
      System::String ^ m_groupName;
   };

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorTool
   //

   ref class EditorTool abstract
   {
   public:
   };

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorSelectTool
   //

   [EditorToolGroup("Standard")]
   ref class EditorSelectTool : public EditorTool
   {
   public:
   };

} // namespace ManagedEditor

#endif // INCLUDED_EDITORTOOLS_H
