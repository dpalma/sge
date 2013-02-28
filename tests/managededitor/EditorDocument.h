///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORDOCUMENT_H
#define INCLUDED_EDITORDOCUMENT_H

#include "engine/entityapi.h"

#ifdef _MSC_VER
#pragma once
#endif

class cTerrainSettings;

namespace ManagedEditor
{

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorDocumentCommand
   //

   ref class EditorDocumentCommand abstract
   {
   public:
      virtual void Do() = 0;
      virtual bool CanUndo() = 0;
      virtual void Undo() = 0;
      property System::String ^ Label
      {
         virtual System::String ^ get() = 0;
      }
   };

   typedef array<EditorDocumentCommand ^> EditorDocumentCommandArray;

   typedef System::Collections::Generic::Stack<EditorDocumentCommand ^> EditorDocumentCommandStack;

   typedef System::Collections::Generic::List<EditorDocumentCommand ^> EditorDocumentCommandList;


   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorCompositeDocumentCommand
   //

   ref class EditorCompositeDocumentCommand sealed : public EditorDocumentCommand
   {
   public:
      EditorCompositeDocumentCommand();

      virtual void Do() override;
      virtual bool CanUndo() override;
      virtual void Undo() override;
      property System::String ^ Label
      {
         virtual System::String ^ get() override;
      }

      property EditorDocumentCommandList ^ Commands;
   };


   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: DocumentChangeEventArgs
   //

   ref class DocumentChangeEventArgs : public System::EventArgs
   {
   public:
      DocumentChangeEventArgs(EditorDocumentCommandArray ^ commands)
       : m_commands(commands) {}

      property EditorDocumentCommandArray ^ Commands
      {
         EditorDocumentCommandArray ^ get()
         {
            return m_commands;
         }
      }

   private:
      EditorDocumentCommandArray ^ m_commands;
   };


   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorDocument
   //

   ref class EditorDocument sealed
   {
   public:
      EditorDocument();
      ~EditorDocument();

      delegate void DocumentChangeHandler(System::Object ^ sender, DocumentChangeEventArgs ^ e);
      event DocumentChangeHandler ^ DocumentChange;

      void Reset();
      bool New(const cTerrainSettings & ts);
      bool Open(System::String ^ fileName);
      bool Save(System::String ^ fileName);

      void AddDocumentCommand(EditorDocumentCommand ^ command, bool bDo);
      void AddDocumentCommands(EditorDocumentCommandArray ^ commands, bool bDo);
      void Undo();
      bool CanUndo();
      void Redo();
      bool CanRedo();

      property bool Modified;
      property System::String ^ FileName;

      property EditorDocumentCommandStack ^ UndoStack
      {
         EditorDocumentCommandStack ^ get()
         {
            return m_undoStack;
         }
      }

      property EditorDocumentCommandStack ^ RedoStack
      {
         EditorDocumentCommandStack ^ get()
         {
            return m_redoStack;
         }
      }

   private:
      void PushCommand(EditorDocumentCommand ^ command, bool bDo);

      EditorDocumentCommandStack ^ m_undoStack;
      EditorDocumentCommandStack ^ m_redoStack;
   };


} // namespace ManagedEditor

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORDOCUMENT_H
