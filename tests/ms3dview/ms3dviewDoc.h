// ms3dviewDoc.h : interface of the CMs3dviewDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MS3DVIEWDOC_H__FA1FD203_9798_4816_B288_5BDCA368C118__INCLUDED_)
#define AFX_MS3DVIEWDOC_H__FA1FD203_9798_4816_B288_5BDCA368C118__INCLUDED_

#include "ms3dmesh.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMs3dviewDoc : public CDocument
{
protected: // create from serialization only
	CMs3dviewDoc();
	DECLARE_DYNCREATE(CMs3dviewDoc)

// Attributes
public:
   inline cMs3dMesh * GetModel() { return m_pMesh; }

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMs3dviewDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMs3dviewDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMs3dviewDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   afx_msg void OnUpdateRendering(CCmdUI * pCmdUI);

private:
   cAutoIPtr<cMs3dMesh> m_pMesh;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MS3DVIEWDOC_H__FA1FD203_9798_4816_B288_5BDCA368C118__INCLUDED_)
