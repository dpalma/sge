/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"
#include "heightmap.h"
#include "terrain.h"
#include "editorTypes.h"

#include "resource.h"

#include "materialapi.h"

#include "readwriteapi.h"
#include "filespec.h"
#include "filepath.h"
#include "globalobj.h"

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////

void FlushCommandStack(std::stack<IEditorCommand *> * pCommandStack)
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
   m_pHeightMap(NULL),
   m_pTerrain(NULL)
{
}

////////////////////////////////////////

cEditorDoc::~cEditorDoc()
{
   Assert(m_pHeightMap == NULL);
}

////////////////////////////////////////

tResult cEditorDoc::New(const cMapSettings * pMapSettings)
{
   if (pMapSettings == NULL)
   {
      return E_POINTER;
   }

   SafeRelease(m_pMaterial);

   delete m_pTerrain, m_pTerrain = NULL;

   delete m_pHeightMap, m_pHeightMap = NULL;

   BOOL bResult = FALSE;

   cHeightMap * pHeightMap = NULL;
   if (pMapSettings->GetHeightData() == kHeightData_HeightMap)
   {
      pHeightMap = new cHeightMap(0.25f); // TODO: hard-coded height scale
      if (pHeightMap != NULL)
      {
         if (!pHeightMap->Load(pMapSettings->GetHeightMap()))
         {
            delete pHeightMap;
            pHeightMap = NULL;
         }
      }
   }

   m_pHeightMap = pHeightMap;

   cAutoIPtr<IEditorTileSet> pTileSet;

   UseGlobal(EditorTileManager);
   if (pEditorTileManager->GetTileSet(pMapSettings->GetTileSet(), &pTileSet) == S_OK)
   {
      pEditorTileManager->SetDefaultTileSet(pMapSettings->GetTileSet());

      Assert(m_pTerrain == NULL);
      m_pTerrain = new cTerrain;
      if (m_pTerrain != NULL)
      {
         if (m_pTerrain->Create(pMapSettings->GetXDimension(),
                                pMapSettings->GetZDimension(),
                                kDefaultStepSize,
                                pTileSet, 0, pHeightMap))
         {
            Assert(!m_pMaterial);
            if (pTileSet->GetMaterial(&m_pMaterial) == S_OK)
            {
               bResult = TRUE;
            }
         }
      }
   }

   return bResult ? S_OK : E_FAIL;
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

   FlushCommandStack(&m_undoStack);
   FlushCommandStack(&m_redoStack);

   m_bModified = false; // not modified anymore

   return S_OK;
}

////////////////////////////////////////

tResult cEditorDoc::Reset()
{
   delete m_pHeightMap, m_pHeightMap = NULL;

   delete m_pTerrain, m_pTerrain = NULL;

   SafeRelease(m_pMaterial);

   FlushCommandStack(&m_undoStack);
   FlushCommandStack(&m_redoStack);

   return S_OK;
}

////////////////////////////////////////

tResult cEditorDoc::IsModified()
{
   return m_bModified ? S_OK : S_FALSE;
}

////////////////////////////////////////

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

////////////////////////////////////////

tResult cEditorDoc::CanUndo(cStr * pLabel)
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

tResult cEditorDoc::Undo()
{
   UndoRedoHelper(&IEditorCommand::Undo, &m_undoStack, &m_redoStack);
   return S_OK;
}

////////////////////////////////////////

tResult cEditorDoc::CanRedo(cStr * pLabel)
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

tResult cEditorDoc::Redo()
{
   UndoRedoHelper(&IEditorCommand::Do, &m_redoStack, &m_undoStack);
   return S_OK;
}

////////////////////////////////////////

const sTerrainVertex * cEditorDoc::GetVertexPointer() const
{
   return (m_pTerrain != NULL) ? m_pTerrain->GetVertexPointer() : NULL;
}

////////////////////////////////////////

size_t cEditorDoc::GetVertexCount() const
{
   return (m_pTerrain != NULL) ? m_pTerrain->GetVertexCount() : 0;
}

////////////////////////////////////////

tResult cEditorDoc::AddCommand(IEditorCommand * pCommand)
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
