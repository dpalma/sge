/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"
#include "terrain.h"
#include "editorTypes.h"
#include "terrainapi.h"

#include "resource.h"

#include "materialapi.h"

#include "readwriteapi.h"
#include "filespec.h"
#include "filepath.h"
#include "globalobj.h"

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////

void FlushCommandStack(tCommandStack * pCommandStack)
{
   if (pCommandStack != NULL)
   {
      while (!pCommandStack->empty())
      {
         pCommandStack->top()->Release();
         pCommandStack->pop();
      }
   }
}

/////////////////////////////////////////////////////////////////////////////

static void UndoRedoHelper(tResult (IEditorCommand::*pfnDoMethod)(),
                           tCommandStack * pSourceStack,
                           tCommandStack * pDestStack)
{
   Assert(pfnDoMethod != NULL);
   Assert(pSourceStack != NULL);
   Assert(pDestStack != NULL);

   IEditorCommand * pPrevious = NULL;
   while (!pSourceStack->empty())
   {
      IEditorCommand * pCommand = pSourceStack->top();

      if (pPrevious != NULL)
      {
         if (pCommand->Compare(pPrevious) != S_OK)
         {
            break;
         }
      }

      if ((pCommand->*pfnDoMethod)() == S_OK)
      {
         pSourceStack->pop();
         pDestStack->push(pCommand);
      }
      else
      {
         break;
      }

      pPrevious = pCommand;
   }
}

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCommandStack
//

////////////////////////////////////////

cCommandStack::cCommandStack()
{
}

////////////////////////////////////////

cCommandStack::~cCommandStack()
{
}

////////////////////////////////////////

tResult cCommandStack::FlushUndo()
{
   FlushCommandStack(&m_undoStack);
   return S_OK;
}

////////////////////////////////////////

tResult cCommandStack::FlushRedo()
{
   FlushCommandStack(&m_redoStack);
   return S_OK;
}

////////////////////////////////////////

tResult cCommandStack::CanUndo(cStr * pLabel)
{
   if (pLabel == NULL)
   {
      return E_POINTER;
   }

   if (m_undoStack.empty())
   {
      return S_FALSE;
   }
   else
   {
      return m_undoStack.top()->GetLabel(pLabel);
   }
}

////////////////////////////////////////

tResult cCommandStack::Undo()
{
   UndoRedoHelper(&IEditorCommand::Undo, &m_undoStack, &m_redoStack);
   return S_OK;
}

////////////////////////////////////////

tResult cCommandStack::CanRedo(cStr * pLabel)
{
   if (pLabel == NULL)
   {
      return E_POINTER;
   }

   if (m_redoStack.empty())
   {
      return S_FALSE;
   }
   else
   {
      return m_redoStack.top()->GetLabel(pLabel);
   }
}

////////////////////////////////////////

tResult cCommandStack::Redo()
{
   UndoRedoHelper(&IEditorCommand::Do, &m_redoStack, &m_undoStack);
   return S_OK;
}

////////////////////////////////////////

tResult cCommandStack::PushCommand(IEditorCommand * pCommand)
{
   if (pCommand == NULL)
   {
      return E_POINTER;
   }

   if (pCommand->Do() == S_OK)
   {
      if (pCommand->CanUndo() == S_OK)
      {
         m_undoStack.push(CTAddRef(pCommand));
      }
      else
      {
         FlushCommandStack(&m_undoStack);
      }

      FlushCommandStack(&m_redoStack);

      return S_OK;
   }

   return E_FAIL;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorDoc
//

////////////////////////////////////////

#if _ATL_VER >= 0x0700
OBJECT_ENTRY_AUTO(CLSID_EditorDoc, cEditorDoc)
#endif

////////////////////////////////////////

cEditorDoc::cEditorDoc()
 : m_bModified(false),
   m_pTerrain(NULL)
{
}

////////////////////////////////////////

cEditorDoc::~cEditorDoc()
{
   Reset();
}

////////////////////////////////////////

tResult cEditorDoc::New(const cMapSettings * pMapSettings)
{
   Verify(Reset() == S_OK);

   m_pTerrain = new cTerrain;
   if (m_pTerrain == NULL)
   {
      return E_OUTOFMEMORY;
   }
   else
   {
      UseGlobal(EditorTileManager);
      cAutoIPtr<IEditorTileSet> pTileSet;
      if (pEditorTileManager->GetTileSet(pMapSettings->GetTileSet(), &pTileSet) != S_OK)
      {
         WarnMsg1("Unable to find tile set \"%s\"; using default instead\n",
            pMapSettings->GetTileSet() == NULL ? "(NULL)" : pMapSettings->GetTileSet());
         pEditorTileManager->GetDefaultTileSet(&pTileSet);
      }

      cAutoIPtr<IHeightMap> pHeightMap;
      if (pMapSettings->GetHeightMap(&pHeightMap) == S_OK)
      {
         return m_pTerrain->Init(pMapSettings->GetXDimension(),
                                 pMapSettings->GetZDimension(),
                                 pTileSet,
                                 pHeightMap);
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cEditorDoc::Open(IReader * pReader)
{
   if (pReader == NULL)
   {
      return E_POINTER;
   }

   Reset();

   m_bModified = true; // set modified flag during load

   Assert(m_pTerrain == NULL);
   m_pTerrain = new cTerrain;
   if (m_pTerrain == NULL)
   {
      return E_OUTOFMEMORY;
   }

   if (FAILED(m_pTerrain->Read(pReader)))
   {
      return E_FAIL;
   }

   m_bModified = false; // start off as unmodified

   return S_OK;
}

////////////////////////////////////////

tResult cEditorDoc::Save(IWriter * pWriter)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (m_pTerrain != NULL)
   {
      if (FAILED(m_pTerrain->Write(pWriter)))
      {
         return E_FAIL;
      }
   }

   m_commandStack.FlushUndo();
   m_commandStack.FlushRedo();

   m_bModified = false; // not modified anymore

   return S_OK;
}

////////////////////////////////////////

tResult cEditorDoc::Reset()
{
   delete m_pTerrain, m_pTerrain = NULL;

   m_commandStack.FlushUndo();
   m_commandStack.FlushRedo();

   return S_OK;
}

////////////////////////////////////////

tResult cEditorDoc::IsModified()
{
   return m_bModified ? S_OK : S_FALSE;
}

////////////////////////////////////////

tResult cEditorDoc::CanUndo(cStr * pLabel)
{
   return m_commandStack.CanUndo(pLabel);
}

////////////////////////////////////////

tResult cEditorDoc::Undo()
{
   if (m_commandStack.Undo() == S_OK)
   {
      m_bModified = true;
      return S_OK;
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cEditorDoc::CanRedo(cStr * pLabel)
{
   return m_commandStack.CanRedo(pLabel);
}

////////////////////////////////////////

tResult cEditorDoc::Redo()
{
   if (m_commandStack.Redo() == S_OK)
   {
      m_bModified = true;
      return S_OK;
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cEditorDoc::AddCommand(IEditorCommand * pCommand)
{
   if (m_commandStack.PushCommand(pCommand) == S_OK)
   {
      m_bModified = true;
      return S_OK;
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cEditorDoc::AddEditorModelListener(IEditorModelListener * pListener)
{
   return Connect(pListener);
}

////////////////////////////////////////

tResult cEditorDoc::RemoveEditorModelListener(IEditorModelListener * pListener)
{
   return Disconnect(pListener);
}

/////////////////////////////////////////////////////////////////////////////
