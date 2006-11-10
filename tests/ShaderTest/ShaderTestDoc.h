/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_MS3DVIEWDOC_H)
#define INCLUDED_MS3DVIEWDOC_H

#include "tech/comtools.h"
#include "tech/matrix4.h"
#include "tech/techstring.h"

#include "engine/entityapi.h"
#include "engine/modelapi.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////

typedef std::vector<tMatrix4> tMatrices;

/////////////////////////////////////////////////////////////////////////////

class cShaderTestDoc : public CDocument
{
protected: // create from serialization only
	cShaderTestDoc();
	DECLARE_DYNCREATE(cShaderTestDoc)

// Attributes
public:
   inline IModel * AccessModel() { return m_pModel; }
   inline tResult GetModel(IModel * * ppModel) { return m_pModel.GetPointer(ppModel); } 
   inline tResult GetModelRenderer(IEntityRenderComponent * * ppRenderComponent)
   {
      return m_pRenderComponent.GetPointer(ppRenderComponent);
   } 

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cShaderTestDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~cShaderTestDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(cShaderTestDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   afx_msg void OnUpdateRendering(CCmdUI * pCmdUI);

private:
   cStr m_model;
   cAutoIPtr<IModel> m_pModel;
   cAutoIPtr<IEntityRenderComponent> m_pRenderComponent;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_MS3DVIEWDOC_H)
