///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "EditorDocument.h"

#include "saveloadapi.h"
#include "terrainapi.h"

#include "filespec.h"
#include "globalobj.h"
#include "readwriteapi.h"
#include "techstring.h"

void StringConvert(System::String ^ string, cStr * pStr);

namespace ManagedEditor
{

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: EditorDocument
//

EditorDocument::EditorDocument()
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

   return true;
}

bool EditorDocument::Open(System::String ^ fileName)
{
   cStr fileName2;
   StringConvert(fileName, &fileName2);

   cAutoIPtr<IReader> pReader;
   if (FileReaderCreate(cFileSpec(fileName2.c_str()), &pReader) != S_OK)
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
   if (FileWriterCreate(cFileSpec(fileName2.c_str()), &pWriter) != S_OK)
   {
      return false;
   }

   UseGlobal(SaveLoadManager);
   if (FAILED(pSaveLoadManager->Save(pWriter)))
   {
      ErrorMsg("An error occured during save\n");
      return false;
   }

   //ClearCommandStack(&m_undoStack);
   //ClearCommandStack(&m_redoStack);

   FileName = fileName;
   Modified = false; // Not modified anymore

   return true;
}


} // namespace ManagedEditor

///////////////////////////////////////////////////////////////////////////////
