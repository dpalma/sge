/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"
#include "editorapi.h"
#include "terrainapi.h"
#include "editorTypes.h"
#include "MapSettingsDlg.h"

#include "resource.h"

#include "renderapi.h"
#include "materialapi.h"

#include "readwriteapi.h"
#include "filespec.h"
#include "filepath.h"
#include "globalobj.h"

#include "dbgalloc.h" // must be last header

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


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

IMPLEMENT_DYNCREATE_EX(cEditorDoc, CDocument)

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
 : m_bPromptForMapSettings(false)
{
}

cEditorDoc::~cEditorDoc()
{
}

BOOL cEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Add reinitialization code here (SDI documents will reuse this document)

   BOOL bResult = FALSE;

   // HACK: hard-coded 64
   cMapSettings mapSettings(64, 64, "");

   if (!m_bPromptForMapSettings)
   {
      // Once the app starts, every File->New should prompt for map settings
      m_bPromptForMapSettings = true;
   }
   else
   {
      cMapSettingsDlg dlg(kHeightData_None);

      // Shouldn't be allowed to cancel the dialog
      Verify(dlg.DoModal() == IDOK);

      dlg.GetMapSettings(&mapSettings);
   }

   cAutoIPtr<ITerrainModel> pTerrainModel;
   if (TerrainModelCreate(mapSettings, &pTerrainModel) == S_OK)
   {
      SetTerrainModel(pTerrainModel);
      bResult = TRUE;
   }
   else
   {
      ErrorMsg("Error creating terrain model\n");
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

tResult cEditorDoc::SetTerrainModel(ITerrainModel * pTerrainModel)
{
   SafeRelease(m_pTerrainModel);
   m_pTerrainModel = CTAddRef(pTerrainModel);

   // Tell the terrain renderer about the new terrain model
   UseGlobal(TerrainRenderer);
   if (!!pTerrainRenderer)
   {
      pTerrainRenderer->SetModel(pTerrainModel);
   }

   return S_OK;
}

tResult cEditorDoc::GetTerrainModel(ITerrainModel * * ppTerrainModel)
{
   return m_pTerrainModel.GetPointer(ppTerrainModel);
}

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

tResult cEditorDoc::AddEditorModelListener(IEditorModelListener * pListener)
{
   return Connect(pListener);
}

tResult cEditorDoc::RemoveEditorModelListener(IEditorModelListener * pListener)
{
   return Disconnect(pListener);
}

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc commands

BOOL cEditorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
   DeleteContents();
   SetModifiedFlag(); // set modified flag during load

   cAutoIPtr<IReader> pReader(FileCreateReader(cFileSpec(lpszPathName)));
   if (!pReader)
   {
      return FALSE;
   }

   // TODO

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

   // TODO

   FlushCommandStack(&m_undoStack);
   FlushCommandStack(&m_redoStack);

   SetModifiedFlag(FALSE); // not modified anymore

   return TRUE;
}

void cEditorDoc::DeleteContents() 
{
   SetTerrainModel(NULL);

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
