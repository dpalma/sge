/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORDOC_H)
#define INCLUDED_EDITORDOC_H

#include "comtools.h"
#include "editorapi.h"

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

class cEditorDoc : public cComObject<IMPLEMENTS(IEditorModel)>,
                   public CUpdateUI<cEditorDoc>,
                   public CIdleHandler
{
protected: // create from serialization only

// Attributes
public:
	cEditorDoc();
	virtual ~cEditorDoc();

   const sTerrainVertex * GetVertexPointer() const;
   size_t GetVertexCount() const;

   IMaterial * AccessMaterial();

   cTerrain * AccessTerrain();

   virtual tResult AddCommand(IEditorCommand * pCommand);

// Operations
public:

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
