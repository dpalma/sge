/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORDOC_H)
#define INCLUDED_EDITORDOC_H

#include "editorapi.h"
#include "connptimpl.h"

#include <stack>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cHeightMap;
class cTerrain;
struct sTerrainVertex;

F_DECLARE_INTERFACE(IMaterial);

typedef std::stack<IEditorCommand *> tCommandStack;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorDoc
//

class cEditorDoc : public CComObjectRoot,
                   public CComCoClass<cEditorDoc, &CLSID_EditorDoc>,
                   public cConnectionPoint<IEditorModel, IEditorModelListener>,
                   public CUpdateUI<cEditorDoc>
{
public:
	cEditorDoc();
	virtual ~cEditorDoc();

   DECLARE_NO_REGISTRY()
   DECLARE_NOT_AGGREGATABLE(cEditorDoc)

   BEGIN_COM_MAP(cEditorDoc)
      COM_INTERFACE_ENTRY(IEditorModel)
   END_COM_MAP()

   // IEditorModel methods
   virtual tResult New(const cMapSettings * pMapSettings);
   virtual tResult Open(IReader * pReader);
   virtual tResult Save(IWriter * pWriter);

   const sTerrainVertex * GetVertexPointer() const;
   size_t GetVertexCount() const;

   IMaterial * AccessMaterial();

   cTerrain * AccessTerrain();

   virtual tResult AddCommand(IEditorCommand * pCommand);

   virtual tResult AddEditorModelListener(IEditorModelListener * pListener);
   virtual tResult RemoveEditorModelListener(IEditorModelListener * pListener);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cEditorDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

   BEGIN_UPDATE_UI_MAP(cEditorDoc)
      UPDATE_ELEMENT(ID_EDIT_UNDO, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
      UPDATE_ELEMENT(ID_EDIT_REDO, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
   END_UPDATE_UI_MAP()

// Generated message map functions
protected:
	//{{AFX_MSG(cEditorDoc)
	void OnEditUndo();
	void OnUpdateEditUndo();
	void OnEditRedo();
	void OnUpdateEditRedo();
	//}}AFX_MSG

private:
   cHeightMap * m_pHeightMap;

   cAutoIPtr<IMaterial> m_pMaterial;

   cTerrain * m_pTerrain;

   tCommandStack m_undoStack, m_redoStack;

   CString m_originalUndoText, m_originalRedoText;
};

////////////////////////////////////////

inline IMaterial * cEditorDoc::AccessMaterial()
{
   return m_pMaterial;
}

////////////////////////////////////////

inline cTerrain * cEditorDoc::AccessTerrain()
{
   return m_pTerrain;
}

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORDOC_H)
