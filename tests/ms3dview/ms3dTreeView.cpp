/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "ms3dTreeView.h"
#include "ms3dviewDoc.h"

#include "resource.h"       // main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// cMs3dTreeView

IMPLEMENT_DYNCREATE(cMs3dTreeView, CTreeView)

cMs3dTreeView::cMs3dTreeView()
{
}

cMs3dTreeView::~cMs3dTreeView()
{
}

BEGIN_MESSAGE_MAP(cMs3dTreeView, CTreeView)
	//{{AFX_MSG_MAP(cMs3dTreeView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cMs3dTreeView drawing

void cMs3dTreeView::OnDraw(CDC * pDC)
{
   // Do nothing
}

/////////////////////////////////////////////////////////////////////////////

BOOL cMs3dTreeView::PreCreateWindow(CREATESTRUCT & cs) 
{
   cs.style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
	
	return CTreeView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// cMs3dTreeView diagnostics

#ifdef _DEBUG
void cMs3dTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void cMs3dTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

c3dmodelDoc* cMs3dTreeView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(c3dmodelDoc)));
	return (c3dmodelDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// cMs3dTreeView message handlers

void cMs3dTreeView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();
	
	c3dmodelDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   VERIFY(GetTreeCtrl().DeleteAllItems());

   if (pDoc && pDoc->AccessModel())
   {
      AddVertices(pDoc->AccessModel());
      AddMaterials(pDoc->AccessModel());
      AddSubMeshes(pDoc->AccessModel());
      AddSkeleton(pDoc->AccessModel());
   }
}

////////////////////////////////////////

void cMs3dTreeView::AddVertices(cModel * pModel, HTREEITEM hParent)
{
   const tModelVertices & vertices = pModel->GetVertices();
   if (!vertices.empty())
   {
   }
}

////////////////////////////////////////

void cMs3dTreeView::AddMaterials(cModel * pModel, HTREEITEM hParent)
{
}

////////////////////////////////////////

void cMs3dTreeView::AddSubMeshes(cModel * pModel, HTREEITEM hParent)
{
   HTREEITEM hSubMeshParent = NULL;
   tModelMeshes::const_iterator iter = pModel->BeginMeshses();
   for (int index = 0; iter != pModel->EndMeshses(); iter++, index++)
   {
      if (index == 0)
      {
         hSubMeshParent = GetTreeCtrl().InsertItem("SubMeshes", hParent);
         if (hSubMeshParent == NULL)
         {
            return;
         }
      }

      const cModelMesh & modelMesh = *iter;

      cStr subMeshItem;
      subMeshItem.Format("Sub mesh %d", index);

      HTREEITEM hSubMeshItem = GetTreeCtrl().InsertItem(subMeshItem.c_str(), hSubMeshParent);
      if (hSubMeshItem == NULL)
      {
         continue;
      }
   }
}

////////////////////////////////////////

void cMs3dTreeView::AddSkeleton(cModel * pModel, HTREEITEM hParent)
{
}

////////////////////////////////////////

void cMs3dTreeView::AddBones(cModel * pModel, HTREEITEM hParent)
{
}

////////////////////////////////////////

void cMs3dTreeView::AddAnimation(cModel * pModel, HTREEITEM hParent)
{
}
