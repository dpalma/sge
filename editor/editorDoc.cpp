/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"

#include "editorTypes.h"
#include "MapPropertiesDlg.h"
#include "MapSettingsDlg.h"

#include "resource.h"

#include "engineapi.h"
#include "terrainapi.h"

#include "readwriteapi.h"
#include "filespec.h"
#include "filepath.h"
#include "globalobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////

void ClearCommandStack(std::stack<IEditorCommand *> * pCommandStack)
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
	ON_COMMAND(ID_FILE_MAP_PROPERTIES, OnFileMapProperties)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc construction/destruction

cEditorDoc::cEditorDoc()
 : m_bPromptForMapSettings(false)
 , m_bHaveMapProperties(false)
{
   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RegisterSaveLoadParticipant(SAVELOADID_MapProperties, 1,
      static_cast<ISaveLoadParticipant*>(this));
}

cEditorDoc::~cEditorDoc()
{
}

BOOL cEditorDoc::OnNewDocument()
{
   cTerrainSettings terrainSettings;

   cStr defaultTileSet;
   UseGlobal(EditorApp);
   if (pEditorApp->GetDefaultTileSet(&defaultTileSet) == S_OK)
   {
      terrainSettings.SetTileSet(defaultTileSet.c_str());
   }

   if (m_bPromptForMapSettings)
   {
      cMapSettingsDlg dlg(terrainSettings);

      if (dlg.DoModal() != IDOK)
      {
         return FALSE;
      }

      dlg.GetTerrainSettings(&terrainSettings);
   }

   if (!m_bPromptForMapSettings)
   {
      // Once the app starts, every File->New should prompt for map settings
      m_bPromptForMapSettings = true;
   }

	if (!CDocument::OnNewDocument())
		return FALSE;

	// Add reinitialization code here (SDI documents will reuse this document)

   BOOL bResult = FALSE;

   UseGlobal(TerrainModel);
   if (pTerrainModel->Initialize(terrainSettings) == S_OK)
   {
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

tResult cEditorDoc::AddCommand(IEditorCommand * pCommand, bool bDo)
{
   if (pCommand == NULL)
   {
      return E_POINTER;
   }

   if (!bDo || pCommand->Do() == S_OK)
   {
      if (pCommand->CanUndo() == S_OK)
      {
         m_undoStack.push(CTAddRef(pCommand));
      }
      else
      {
         ClearCommandStack(&m_undoStack);
      }

      ClearCommandStack(&m_redoStack);

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

tResult cEditorDoc::Save(IWriter * pWriter)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   // cEditorDoc's implementation of ISaveLoadParticipant is for the
   // map properties only. All other content is saved/loaded by other
   // participants. cEditorDoc drives this by calling ISaveLoadManager's
   // Save and Load methods when appropriate (File->Open menu, e.g.)

   if (m_bHaveMapProperties)
   {
      cMapProperties mapProperties(m_title, m_author, m_description, m_numPlayers);
      return pWriter->Write(mapProperties);
   }

   return S_FALSE;
}

tResult cEditorDoc::Load(IReader * pReader, int version)
{
   if (pReader == NULL)
   {
      return E_POINTER;
   }

   cMapProperties mapProperties;
   if (pReader->Read(&mapProperties) == S_OK)
   {
      m_bHaveMapProperties = true;
      m_title = mapProperties.GetTitle();
      m_author = mapProperties.GetAuthor();
      m_description = mapProperties.GetDescription();
      m_numPlayers = mapProperties.GetNumPlayers();
      return S_OK;
   }

   return S_FALSE;
}

void cEditorDoc::Reset()
{
   DeleteContents();
}

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc commands

BOOL cEditorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
   DeleteContents();
   SetModifiedFlag(); // set modified flag during load

   cAutoIPtr<IReader> pReader;
   if (FileReaderCreate(cFileSpec(lpszPathName), kFileModeBinary, &pReader) != S_OK)
   {
      return FALSE;
   }

   UseGlobal(SaveLoadManager);
   if (FAILED(pSaveLoadManager->Load(pReader)))
   {
      ErrorMsg("An error occured during load\n");
      return FALSE;
   }

   SetModifiedFlag(FALSE); // start off as unmodified

   return TRUE;
}

BOOL cEditorDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
   cAutoIPtr<IWriter> pWriter;
   if (FileWriterCreate(cFileSpec(lpszPathName), kFileModeBinary, &pWriter) != S_OK)
   {
      return FALSE;
   }

   UseGlobal(SaveLoadManager);
   if (FAILED(pSaveLoadManager->Save(pWriter)))
   {
      ErrorMsg("An error occured during save\n");
      return FALSE;
   }

   ClearCommandStack(&m_undoStack);
   ClearCommandStack(&m_redoStack);

   SetModifiedFlag(FALSE); // not modified anymore

   return TRUE;
}

void cEditorDoc::DeleteContents() 
{
   UseGlobal(TerrainModel);
   pTerrainModel->Clear();

   ClearCommandStack(&m_undoStack);
   ClearCommandStack(&m_redoStack);

   CDocument::DeleteContents();
}

void cEditorDoc::OnCloseDocument()
{
   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RevokeSaveLoadParticipant(SAVELOADID_MapProperties, 1);

   CDocument::OnCloseDocument();
}

static tResult UndoRedoHelper(tResult (IEditorCommand::*pfnDoMethod)(),
                              tCommandStack * pSourceStack,
                              tCommandStack * pDestStack)
{
   Assert(pfnDoMethod != NULL);
   Assert(pSourceStack != NULL);
   Assert(pDestStack != NULL);

   if (pSourceStack->empty())
   {
      Assert(!"UndoRedoHelper called on empty stack\n");
      return E_FAIL;
   }

   IEditorCommand * pCommand = pSourceStack->top();

   if ((pCommand->*pfnDoMethod)() == S_OK)
   {
      pSourceStack->pop();
      pDestStack->push(pCommand);
      return S_OK;
   }

   return E_FAIL;
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
      if (m_undoStack.top()->GetLabel(&label) != S_OK)
      {
         label.assign(m_originalUndoText);
      }

      CString menuText;
      menuText.Format(IDS_UNDO_TEXT, label.c_str());
      pCmdUI->SetText(menuText);
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
      if (m_redoStack.top()->GetLabel(&label) != S_OK)
      {
         label.assign(m_originalRedoText);
      }

      CString menuText;
      menuText.Format(IDS_REDO_TEXT, label.c_str());
      pCmdUI->SetText(menuText);
   }
}

void cEditorDoc::OnFileMapProperties()
{
   cMapPropertiesDlg dlg;
   if (m_bHaveMapProperties)
   {
      dlg.SetTitle(m_title);
      dlg.SetAuthor(m_author);
      dlg.SetNumPlayers(m_numPlayers);
      dlg.SetDescription(m_description);
   }
   if (dlg.DoModal() == IDOK)
   {
      m_bHaveMapProperties = true;
      dlg.GetTitle(&m_title);
      dlg.GetAuthor(&m_author);
      m_numPlayers = dlg.GetNumPlayers();
      dlg.GetDescription(&m_description);
   }
}
