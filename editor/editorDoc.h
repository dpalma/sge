/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORDOC_H)
#define INCLUDED_EDITORDOC_H

#include "comtools.h"

#include "vec2.h"
#include "vec3.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cHeightMap;
struct sVertexElement;

F_DECLARE_INTERFACE(IEditorTileSet);

F_DECLARE_INTERFACE(IRenderDevice);
F_DECLARE_INTERFACE(IMaterial);

/////////////////////////////////////////////////////////////////////////////

struct sMapVertex
{
   tVec2 uv1;
   tVec3 rgb;
   tVec3 pos;
};

extern sVertexElement g_mapVertexDecl[];
extern uint g_nMapVertexMembers;

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
   const sMapVertex * GetVertexPointer() const;
   size_t GetVertexCount() const;

   IMaterial * AccessMaterial();

// Operations
public:
   float GetElevation(float nx, float nz) const;
   void GetMapDimensions(uint * pxd, uint * pzd) const;
   void GetMapExtents(uint * px, uint * pz) const;

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
   bool InitializeVertices(uint xDim, uint zDim, IEditorTileSet * pTileSet,
      uint defaultTile, cHeightMap * pHeightMap);

// Generated message map functions
protected:
	//{{AFX_MSG(cEditorDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
   cHeightMap * m_pHeightMap;

   uint m_xDim, m_zDim;

   cAutoIPtr<IMaterial> m_pMaterial;

   std::vector<sMapVertex> m_vertices;
};

////////////////////////////////////////

inline const sMapVertex * cEditorDoc::GetVertexPointer() const
{
   return &m_vertices[0];
}

////////////////////////////////////////

inline size_t cEditorDoc::GetVertexCount() const
{
   return m_vertices.size();
}

////////////////////////////////////////

inline IMaterial * cEditorDoc::AccessMaterial()
{
   return m_pMaterial;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_EDITORDOC_H)
