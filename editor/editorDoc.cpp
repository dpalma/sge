/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"
#include "heightmap.h"
#include "editorapi.h"
#include "terrain.h"
#include "editorTypes.h"

#include "resource.h"

#include "renderapi.h"
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

IMPLEMENT_DYNCREATE(cEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(cEditorDoc, CDocument)
	//{{AFX_MSG_MAP(cEditorDoc)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc construction/destruction

cEditorDoc::cEditorDoc()
 : m_pHeightMap(NULL),
   m_pTerrain(NULL)
{
}

cEditorDoc::~cEditorDoc()
{
   Assert(m_pHeightMap == NULL);
}

BOOL cEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Add reinitialization code here (SDI documents will reuse this document)

   Assert(AccessEditorApp() != NULL);

   SafeRelease(m_pMaterial);

   delete m_pTerrain, m_pTerrain = NULL;

   delete m_pHeightMap, m_pHeightMap = NULL;

   BOOL bResult = FALSE;

   cMapSettings mapSettings;

   if (AccessEditorApp()->GetMapSettings(&mapSettings) == S_OK)
   {
      cHeightMap * pHeightMap = NULL;
      if (mapSettings.GetHeightData() == kHeightData_HeightMap)
      {
         pHeightMap = new cHeightMap(0.25f); // TODO: hard-coded height scale
         if (pHeightMap != NULL)
         {
            if (!pHeightMap->Load(mapSettings.GetHeightMap()))
            {
               delete pHeightMap;
               pHeightMap = NULL;
            }
         }
      }

      m_pHeightMap = pHeightMap;

      cAutoIPtr<IEditorTileSet> pTileSet;

      UseGlobal(EditorTileManager);
      if (pEditorTileManager->GetTileSet(mapSettings.GetTileSet(), &pTileSet) == S_OK)
      {
         pEditorTileManager->SetDefaultTileSet(mapSettings.GetTileSet());

         Assert(m_pTerrain == NULL);
         m_pTerrain = new cTerrain;
         if (m_pTerrain != NULL)
         {
            if (m_pTerrain->Create(mapSettings.GetXDimension(),
                                 mapSettings.GetZDimension(),
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
      else
      {
         bResult = TRUE;
      }

   }

	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc serialization

void cEditorDoc::Serialize(CArchive& ar)
{
   Assert(!"This method should never be called");
}

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc diagnostics

#ifdef _DEBUG
void cEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void cEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc operations

const sTerrainVertex * cEditorDoc::GetVertexPointer() const
{
   return (m_pTerrain != NULL) ? m_pTerrain->GetVertexPointer() : NULL;
}

size_t cEditorDoc::GetVertexCount() const
{
   return (m_pTerrain != NULL) ? m_pTerrain->GetVertexCount() : 0;
}

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc commands

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

BOOL cEditorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
   DeleteContents();
   SetModifiedFlag(); // set modified flag during load

   cAutoIPtr<IReader> pReader(FileCreateReader(cFileSpec(lpszPathName)));
   if (!pReader)
   {
      return FALSE;
   }

   Assert(m_pTerrain == NULL);
   m_pTerrain = new cTerrain;
   if (m_pTerrain == NULL)
   {
      ErrorMsg("Error allocating new terrain object\n");
      return FALSE;
   }
   else
   {
      if (FAILED(m_pTerrain->Read(pReader)))
      {
         ErrorMsg1("Error loading document %s\n", lpszPathName);
         return FALSE;
      }
   }

   SetModifiedFlag(FALSE); // start off as unmodified

   return TRUE;
}

BOOL cEditorDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
   cAutoIPtr<IWriter> pWriter(FileCreateWriter(cFileSpec(lpszPathName)));
   if (!pWriter)
   {
      return FALSE;
   }

   if (m_pTerrain != NULL)
   {
      if (FAILED(m_pTerrain->Write(pWriter)))
      {
         ErrorMsg1("Error saving document %s\n", lpszPathName);
         return FALSE;
      }
   }

   FlushCommandStack(&m_undoStack);
   FlushCommandStack(&m_redoStack);

   SetModifiedFlag(FALSE); // not modified anymore

   return TRUE;
}

void cEditorDoc::DeleteContents() 
{
   delete m_pHeightMap, m_pHeightMap = NULL;

   delete m_pTerrain, m_pTerrain = NULL;

   SafeRelease(m_pMaterial);

   FlushCommandStack(&m_undoStack);
   FlushCommandStack(&m_redoStack);

   CDocument::DeleteContents();
}

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

void cEditorDoc::OnEditUndo() 
{
   UndoRedoHelper(&IEditorCommand::Undo, &m_undoStack, &m_redoStack);
}

void cEditorDoc::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
   if (m_originalUndoText.IsEmpty() && (pCmdUI->m_pMenu != NULL))
   {
      pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, m_originalUndoText, MF_BYCOMMAND);
   }

   if (m_undoStack.empty())
   {
      pCmdUI->Enable(FALSE);
      pCmdUI->SetText(m_originalUndoText);
   }
   else
   {
      pCmdUI->Enable(TRUE);

      cStr label;
      if (m_undoStack.top()->GetLabel(&label) == S_OK)
      {
         CString menuText;
         menuText.Format(IDS_UNDO_TEXT, label.c_str());
         pCmdUI->SetText(menuText);
      }
   }
}

void cEditorDoc::OnEditRedo() 
{
   UndoRedoHelper(&IEditorCommand::Do, &m_redoStack, &m_undoStack);
}

void cEditorDoc::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
   if (m_originalRedoText.IsEmpty() && (pCmdUI->m_pMenu != NULL))
   {
      pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, m_originalRedoText, MF_BYCOMMAND);
   }

   if (m_redoStack.empty())
   {
      pCmdUI->Enable(FALSE);
      pCmdUI->SetText(m_originalRedoText);
   }
   else
   {
      pCmdUI->Enable(TRUE);

      cStr label;
      if (m_redoStack.top()->GetLabel(&label) == S_OK)
      {
         CString menuText;
         menuText.Format(IDS_REDO_TEXT, label.c_str());
         pCmdUI->SetText(menuText);
      }
   }
}
