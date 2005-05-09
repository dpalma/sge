/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_MS3DVIEWDOC_H)
#define INCLUDED_MS3DVIEWDOC_H

#include "meshapi.h"
#include "animation.h"
#include "skeleton.h"
#include "comtools.h"

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
   inline IMesh * GetModel() { return m_pMesh; }
   void SetFrame(float pct);
   const tMatrix4 * GetBlendMatrices() { return &m_boneMatrices[0]; }
   uint GetBlendMatrixCount() const { return m_boneMatrices.size(); }

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMs3dviewDoc)
	public:
	virtual BOOL OnNewDocument();
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
   tResult PostRead();

// Generated message map functions
protected:
	//{{AFX_MSG(CMs3dviewDoc)
	afx_msg void OnUpdateToolsOptimize(CCmdUI* pCmdUI);
	afx_msg void OnToolsOptimize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   afx_msg void OnUpdateRendering(CCmdUI * pCmdUI);

private:
   cAutoIPtr<IMesh> m_pMesh;
   tMatrices m_boneMatrices;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_MS3DVIEWDOC_H)
