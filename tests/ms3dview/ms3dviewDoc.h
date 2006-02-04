/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_MS3DVIEWDOC_H)
#define INCLUDED_MS3DVIEWDOC_H

#include "comtools.h"
#include "matrix4.h"

#include "model.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////

typedef std::vector<tMatrix4> tMatrices;

/////////////////////////////////////////////////////////////////////////////

class c3dmodelDoc : public CDocument
{
protected: // create from serialization only
	c3dmodelDoc();
	DECLARE_DYNCREATE(c3dmodelDoc)

// Attributes
public:
   inline cModel * AccessModel() { return m_pModel; }

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(c3dmodelDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~c3dmodelDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(c3dmodelDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   afx_msg void OnUpdateRendering(CCmdUI * pCmdUI);

private:
   cStr m_model;
   cModel * m_pModel;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_MS3DVIEWDOC_H)
