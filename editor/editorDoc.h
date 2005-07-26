/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORDOC_H)
#define INCLUDED_EDITORDOC_H

#include "comtools.h"
#include "connptimpl.h"
#include "afxcomtools.h"
#include "editorapi.h"

#include <stack>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef std::stack<IEditorCommand *> tCommandStack;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorDoc
//

typedef cConnectionPoint<IEditorModel, IEditorModelListener> tEditorDocConnPt;

class cEditorDoc : public CDocument,
                   public cComObject<tEditorDocConnPt, &IID_IEditorModel, cAfxComServices<cEditorDoc> >
{
protected: // create from serialization only
	cEditorDoc();
	DECLARE_DYNCREATE_EX(cEditorDoc)

// Attributes
public:

// Operations
public:
   virtual tResult AddCommand(IEditorCommand * pCommand);

   virtual tResult AddEditorModelListener(IEditorModelListener * pListener);
   virtual tResult RemoveEditorModelListener(IEditorModelListener * pListener);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cEditorDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~cEditorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(cEditorDoc)
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
   bool m_bPromptForMapSettings;

   cAutoIPtr<ITerrainModel> m_pTerrainModel;

   tCommandStack m_undoStack, m_redoStack;
   CString m_originalUndoText, m_originalRedoText;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_EDITORDOC_H)
