///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "EditorDocument.h"

#include "engine/saveloadapi.h"
#include "engine/terrainapi.h"

#include "tech/filespec.h"
#include "tech/globalobj.h"
#include "tech/readwriteapi.h"
#include "tech/techstring.h"

void StringConvert(System::String ^ string, cStr * pStr);

namespace ManagedEditor
{

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorDocument
   //

   EditorDocument::EditorDocument()
    : m_undoStack(gcnew EditorDocumentCommandStack())
    , m_redoStack(gcnew EditorDocumentCommandStack())
   {
      Modified = false;
   }

   EditorDocument::~EditorDocument()
   {
   }

   // Return the document to its original state
   void EditorDocument::Reset()
   {
      FileName = "";
      Modified = false;
      UseGlobal(TerrainModel);
      pTerrainModel->Clear();
   }

   bool EditorDocument::New(const cTerrainSettings & ts)
   {
      Reset();
      Modified = true;

      bool bResult = false;

      UseGlobal(TerrainModel);
      if (pTerrainModel->Initialize(ts) == S_OK)
      {
         bResult = true;
      }

      return bResult;
   }

   bool EditorDocument::Open(System::String ^ fileName)
   {
      cStr fileName2;
      StringConvert(fileName, &fileName2);

      cAutoIPtr<IReader> pReader;
      if (FileReaderCreate(cFileSpec(fileName2.c_str()), kFileModeBinary, &pReader) != S_OK)
      {
         return false;
      }

      Reset();
      Modified = true; // Set modified flag during load

      UseGlobal(SaveLoadManager);
      if (FAILED(pSaveLoadManager->Load(pReader)))
      {
         ErrorMsg("An error occured during load\n");
         return false;
      }

      FileName = fileName;
      Modified = false; // Start off as unmodified

      return true;
   }

   bool EditorDocument::Save(System::String ^ fileName)
   {
      cStr fileName2;
      StringConvert(fileName, &fileName2);

      cAutoIPtr<IWriter> pWriter;
      if (FileWriterCreate(cFileSpec(fileName2.c_str()), kFileModeBinary, &pWriter) != S_OK)
      {
         return false;
      }

      UseGlobal(SaveLoadManager);
      if (FAILED(pSaveLoadManager->Save(pWriter)))
      {
         ErrorMsg("An error occured during save\n");
         return false;
      }

      m_undoStack->Clear();
      m_redoStack->Clear();

      FileName = fileName;
      Modified = false; // Not modified anymore

      return true;
   }

   void EditorDocument::AddDocumentCommand(EditorDocumentCommand ^ command, bool bDo)
   {
      if (command == nullptr)
      {
         return;
      }

      PushCommand(command, bDo);

      Modified = true;

      DocumentChange(this, gcnew DocumentChangeEventArgs(gcnew EditorDocumentCommandArray{command}));
   }

   void EditorDocument::AddDocumentCommands(EditorDocumentCommandArray ^ commands, bool bDo)
   {
      if (commands == nullptr)
      {
         return;
      }

      System::Collections::IEnumerator ^ e = commands->GetEnumerator();
      while (e->MoveNext())
      {
         EditorDocumentCommand ^ command = dynamic_cast<EditorDocumentCommand ^>(e->Current);
         if (command == nullptr)
         {
            continue;
         }

         PushCommand(command, bDo);
      }

      Modified = true;

      DocumentChange(this, gcnew DocumentChangeEventArgs(commands));
   }

   void EditorDocument::Undo()
   {
      if (m_undoStack->Count == 0)
      {
         return;
      }

      EditorDocumentCommand ^ command = dynamic_cast<EditorDocumentCommand ^>(m_undoStack->Peek());
      if (command)
      {
         command->Undo();
         m_undoStack->Pop();

         Modified = true;

         DocumentChange(this, gcnew DocumentChangeEventArgs(gcnew EditorDocumentCommandArray{command}));
      }
   }

   bool EditorDocument::CanUndo()
   {
      return ((m_undoStack->Count > 0)
         && (dynamic_cast<EditorDocumentCommand ^>(m_undoStack->Peek()) != nullptr));
   }

   void EditorDocument::Redo()
   {
      if (m_redoStack->Count == 0)
      {
         return;
      }

      EditorDocumentCommand ^ command = dynamic_cast<EditorDocumentCommand ^>(m_redoStack->Peek());
      if (command)
      {
         command->Do();
         m_redoStack->Pop();

         Modified = true;

         DocumentChange(this, gcnew DocumentChangeEventArgs(gcnew EditorDocumentCommandArray{command}));
      }
   }

   bool EditorDocument::CanRedo()
   {
      return ((m_redoStack->Count > 0)
         && (dynamic_cast<EditorDocumentCommand ^>(m_redoStack->Peek()) != nullptr));
   }

   void EditorDocument::PushCommand(EditorDocumentCommand ^ command, bool bDo)
   {
      if (command == nullptr)
      {
         return;
      }

      // TODO: try-catch here?
      if (bDo)
      {
         command->Do();
      }

      if (command->CanUndo())
      {
         m_undoStack->Push(command);
      }
      else
      {
         m_undoStack->Clear();
      }

      m_redoStack->Clear();
   }

} // namespace ManagedEditor

///////////////////////////////////////////////////////////////////////////////
