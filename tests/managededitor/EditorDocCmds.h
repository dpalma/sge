///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORDOCCMDS_H
#define INCLUDED_EDITORDOCCMDS_H

#include "EditorDocument.h"

#include "tech/comtools.h"
#include "tech/vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IEntity);


namespace ManagedEditor
{

   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: SelectEntityCommand
   //

   ref class SelectEntityCommand sealed : public EditorDocumentCommand
   {
   public:
      SelectEntityCommand(IEntity * pEntity);
      ~SelectEntityCommand();
      !SelectEntityCommand();

      virtual void Do() override;
      virtual bool CanUndo() override;
      virtual void Undo() override;
      property System::String ^ Label
      {
         virtual System::String ^ get() override;
      }

   private:
      IEntity * m_pEntity;
   };


   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: PlaceEntityCommand
   //

   ref class PlaceEntityCommand sealed : public EditorDocumentCommand
   {
   public:
      PlaceEntityCommand(System::String ^ entity, float nx, float nz);

      virtual void Do() override;
      virtual bool CanUndo() override;
      virtual void Undo() override;
      property System::String ^ Label
      {
         virtual System::String ^ get() override;
      }

   private:
      System::String ^ m_entity;
      float m_nx, m_nz;
      ulong m_placedEntityId;
   };

} // namespace ManagedEditor

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORDOCCMDS_H
