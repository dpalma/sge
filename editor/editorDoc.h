/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORDOC_H)
#define INCLUDED_EDITORDOC_H

#include "comtools.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cTiledGround;
class cHeightMap;

F_DECLARE_INTERFACE(IRenderDevice);

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorDoc
//

class cEditorDoc : public CDocument
{
protected: // create from serialization only
	cEditorDoc();
	DECLARE_DYNCREATE(cEditorDoc)

// Attributes
public:
   cTiledGround * AccessTiledGround() { return m_pGround; }

// Operations
public:
   bool SetTerrain(IRenderDevice * pRenderDevice, const char * pszHeightData, float heightScale, const char * pszTexture);
   float GetElevation(float nx, float nz) const;
   bool GetDimensions(uint * pxd, uint * pzd) const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cEditorDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~cEditorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(cEditorDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
   cHeightMap * m_pHeightMap;
   cTiledGround * m_pGround;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_EDITORDOC_H)
