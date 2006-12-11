///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORDOCUMENT_H
#define INCLUDED_EDITORDOCUMENT_H

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
      virtual System::String ^ GetLabel() = 0;
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

      void Reset();
      bool New(const cTerrainSettings & ts);
      bool Open(System::String ^ fileName);
      bool Save(System::String ^ fileName);

      void AddDocumentCommand(EditorDocumentCommand ^ command, bool bDo);
      void Undo();
      bool CanUndo();
      void Redo();
      bool CanRedo();

      property bool Modified;
      property System::String ^ FileName;

   private:
      System::Collections::Stack ^ m_undoStack;
      System::Collections::Stack ^ m_redoStack;
   };


} // namespace ManagedEditor

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORDOCUMENT_H
