/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORDOC_H)
#define INCLUDED_EDITORDOC_H

#include "comtools.h"
#include "afxcomtools.h"
#include "editorapi.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cHeightMap;
class cTerrain;
struct sTerrainVertex;

F_DECLARE_INTERFACE(IMaterial);

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorDoc
//

class cEditorDoc : public CDocument,
                   public cComObject<IMPLEMENTS(IEditorModel), cAfxComServices<cEditorDoc> >
{
protected: // create from serialization only
	cEditorDoc();
	DECLARE_DYNCREATE(cEditorDoc)

// Attributes
public:
   const sTerrainVertex * GetVertexPointer() const;
   size_t GetVertexCount() const;

   IMaterial * AccessMaterial();

   cTerrain * AccessTerrain();

// Operations
public:
   void GetMapDimensions(uint * pxd, uint * pzd) const;
   void GetMapExtents(uint * px, uint * pz) const;

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
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
   cHeightMap * m_pHeightMap;

   cAutoIPtr<IMaterial> m_pMaterial;

   cTerrain * m_pTerrain;
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

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_EDITORDOC_H)
