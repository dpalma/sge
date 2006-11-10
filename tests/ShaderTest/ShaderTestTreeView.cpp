/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ShaderTestTreeView.h"

#include "ShaderTestDoc.h"

#include "resource.h"       // main symbols

#include "engine/modeltypes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// cShaderTestTreeView

IMPLEMENT_DYNCREATE(cShaderTestTreeView, CTreeView)

cShaderTestTreeView::cShaderTestTreeView()
{
}

cShaderTestTreeView::~cShaderTestTreeView()
{
}

BEGIN_MESSAGE_MAP(cShaderTestTreeView, CTreeView)
	//{{AFX_MSG_MAP(cShaderTestTreeView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cShaderTestTreeView drawing

void cShaderTestTreeView::OnDraw(CDC * pDC)
{
   // Do nothing
}

/////////////////////////////////////////////////////////////////////////////

BOOL cShaderTestTreeView::PreCreateWindow(CREATESTRUCT & cs) 
{
   cs.style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
   return CTreeView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// cShaderTestTreeView diagnostics

#ifdef _DEBUG
void cShaderTestTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void cShaderTestTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

cShaderTestDoc* cShaderTestTreeView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(cShaderTestDoc)));
	return (cShaderTestDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// cShaderTestTreeView message handlers

void cShaderTestTreeView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();
	
	cShaderTestDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   Verify(GetTreeCtrl().DeleteAllItems());

   cAutoIPtr<IModel> pModel;
   if (pDoc && pDoc->GetModel(&pModel) == S_OK)
   {
      AddVertices(pModel);
      AddMaterials(pModel);
      AddSubMeshes(pModel);
      AddSkeleton(pModel);
   }
}

////////////////////////////////////////

void cShaderTestTreeView::AddVertices(IModel * pModel, HTREEITEM hParent)
{
}

////////////////////////////////////////

void cShaderTestTreeView::AddMaterials(IModel * pModel, HTREEITEM hParent)
{
}

////////////////////////////////////////

void cShaderTestTreeView::AddSubMeshes(IModel * pModel, HTREEITEM hParent)
{
   uint nMeshes = 0;
   const sModelMesh * pMeshes = NULL;
   if (pModel != NULL && pModel->GetMeshes(&nMeshes, &pMeshes) == S_OK)
   {
      HTREEITEM hSubMeshParent = GetTreeCtrl().InsertItem("SubMeshes", hParent);
      if (hSubMeshParent == NULL)
      {
         return;
      }

      for (uint i = 0; i < nMeshes; ++i)
      {
         const sModelMesh & modelMesh = pMeshes[i];

         cStr subMeshItem;
         Sprintf(&subMeshItem, "Sub mesh %d", i);

         HTREEITEM hSubMeshItem = GetTreeCtrl().InsertItem(subMeshItem.c_str(), hSubMeshParent);
         if (hSubMeshItem == NULL)
         {
            continue;
         }
      }
   }
}

////////////////////////////////////////

void cShaderTestTreeView::AddSkeleton(IModel * pModel, HTREEITEM hParent)
{
}

////////////////////////////////////////

void cShaderTestTreeView::AddBones(IModel * pModel, HTREEITEM hParent)
{
}

////////////////////////////////////////

void cShaderTestTreeView::AddAnimation(IModel * pModel, HTREEITEM hParent)
{
}
