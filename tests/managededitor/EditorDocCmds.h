///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORDOCCMDS_H
#define INCLUDED_EDITORDOCCMDS_H

#include "EditorDocument.h"

#include "tech/vec3.h"

#ifdef _MSC_VER
#pragma once
#endif


namespace ManagedEditor
{

   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: PlaceEntityCommand
   //

   ref class PlaceEntityCommand sealed : public EditorDocumentCommand
   {
   public:
      PlaceEntityCommand(System::String ^ entity, float x, float z);
      ~PlaceEntityCommand();

      virtual void Do() override;
      virtual bool CanUndo() override;
      virtual void Undo() override;
      virtual System::String ^ GetLabel() override;

   private:
      System::String ^ m_entity;
      float m_x, m_z;
   };

} // namespace ManagedEditor

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORDOCCMDS_H
