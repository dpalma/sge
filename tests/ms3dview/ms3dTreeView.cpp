/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "ms3dTreeView.h"
#include "ms3dviewDoc.h"
#include "TreeUtils.h"

#include "skeleton.h"
#include "animation.h"

#include "renderapi.h"
#include "material.h"
#include "color.h"

#include <vector>

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

CMs3dviewDoc* cMs3dTreeView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMs3dviewDoc)));
	return (CMs3dviewDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// cMs3dTreeView message handlers

void cMs3dTreeView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();
	
	CMs3dviewDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   VERIFY(GetTreeCtrl().DeleteAllItems());

   if (pDoc && pDoc->GetModel())
   {
      AddVertices(pDoc->GetModel());
      AddMaterials(pDoc->GetModel());
      AddSubMeshes(pDoc->GetModel());
      AddSkeleton(pDoc->GetModel());
   }
}

////////////////////////////////////////

void cMs3dTreeView::AddVertices(IMesh * pMesh, HTREEITEM hParent)
{
   uint nVertices = pMesh->GetVertexCount();
   if (nVertices > 0)
   {
      cAutoIPtr<IVertexBuffer> pVertexBuffer;
      if (pMesh->GetVertexBuffer(&pVertexBuffer) == S_OK)
      {
         ::AddVertices(nVertices, pVertexBuffer, &GetTreeCtrl(), hParent);
      }
   }
}

////////////////////////////////////////

void cMs3dTreeView::AddMaterials(IMesh * pMesh, HTREEITEM hParent)
{
   int nMaterials = pMesh->GetMaterialCount();
   if (nMaterials > 0)
   {
      HTREEITEM hMaterials = GetTreeCtrl().InsertItem("Materials", hParent);
      if (hMaterials != NULL)
      {
         for (int i = 0; i < nMaterials; i++)
         {
            cAutoIPtr<IMaterial> pMaterial;
            if ((pMesh->GetMaterial(i, &pMaterial) == S_OK) && (pMaterial != NULL))
            {
               HTREEITEM hMaterial = GetTreeCtrl().InsertItem(pMaterial->GetName(), hMaterials);
               if (hMaterial != NULL)
               {
                  float s;
                  CString str;
                  cColor color;
                  if (pMaterial->GetAmbient(&color) == S_OK)
                  {
                     str.Format("Ambient: %.2f, %.2f, %.2f, %.2f",
                        color.GetPointer()[0], color.GetPointer()[1], color.GetPointer()[2], color.GetPointer()[3]);
                     GetTreeCtrl().InsertItem(str, hMaterial);
                  }
                  if (pMaterial->GetDiffuse(&color) == S_OK)
                  {
                     str.Format("Diffuse: %.2f, %.2f, %.2f, %.2f",
                        color.GetPointer()[0], color.GetPointer()[1], color.GetPointer()[2], color.GetPointer()[3]);
                     GetTreeCtrl().InsertItem(str, hMaterial);
                  }
                  if (pMaterial->GetSpecular(&color) == S_OK)
                  {
                     str.Format("Specular: %.2f, %.2f, %.2f, %.2f",
                        color.GetPointer()[0], color.GetPointer()[1], color.GetPointer()[2], color.GetPointer()[3]);
                     GetTreeCtrl().InsertItem(str, hMaterial);
                  }
                  if (pMaterial->GetEmissive(&color) == S_OK)
                  {
                     str.Format("Emissive: %.2f, %.2f, %.2f, %.2f",
                        color.GetPointer()[0], color.GetPointer()[1], color.GetPointer()[2], color.GetPointer()[3]);
                     GetTreeCtrl().InsertItem(str, hMaterial);
                  }
                  if (pMaterial->GetShininess(&s) == S_OK)
                  {
                     str.Format("Shininess: %.2f", s);
                     GetTreeCtrl().InsertItem(str, hMaterial);
                  }
               }
            }
         }
      }
   }
}

////////////////////////////////////////

void cMs3dTreeView::AddSubMeshes(IMesh * pMesh, HTREEITEM hParent)
{
   uint nSubMeshes = pMesh->GetSubMeshCount();
   if (nSubMeshes > 0)
   {
      HTREEITEM hSubMeshes = GetTreeCtrl().InsertItem("SubMeshes", hParent);
      if (hSubMeshes != NULL)
      {
         for (uint i = 0; i < nSubMeshes; i++)
         {
            cAutoIPtr<ISubMesh> pSubMesh;
            if (pMesh->GetSubMesh(i, &pSubMesh) == S_OK)
            {
               HTREEITEM hSubMesh = GetTreeCtrl().InsertItem(pSubMesh->GetMaterialName(), hSubMeshes);
               if (hSubMesh != NULL)
               {
                  // TODO HACK: assumes index buffer uses 16-bit indices
                  uint16 * pIndices = NULL;
                  if (pSubMesh->LockIndexBuffer(kBL_Default, (void**)&pIndices) == S_OK)
                  {
                     for (uint i = 0; i < pSubMesh->GetIndexCount(); i++)
                     {
                        CString item;
                        item.Format("Index %d = %d", i, pIndices[i]);
                        GetTreeCtrl().InsertItem(item, hSubMesh);
                     }

                     pSubMesh->UnlockIndexBuffer();
                  }
               }
            }
         }
      }
   }
}

////////////////////////////////////////

void cMs3dTreeView::AddSkeleton(IMesh * pMesh, HTREEITEM hParent)
{
   cAutoIPtr<ISkeleton> pSkeleton;
   if (pMesh->GetSkeleton(&pSkeleton) == S_OK)
   {
      HTREEITEM hSkel = GetTreeCtrl().InsertItem("Skeleton", hParent);
      if (hSkel != NULL)
      {
         AddBones(pSkeleton, hSkel);
         AddAnimation(pSkeleton, hSkel);
      }
   }
}

////////////////////////////////////////

void cMs3dTreeView::AddBones(ISkeleton * pSkeleton, HTREEITEM hParent)
{
   int nBones = pSkeleton != NULL ? pSkeleton->GetBoneCount() : 0;
   if (nBones > 0)
   {
      HTREEITEM hBones = GetTreeCtrl().InsertItem("Bones", hParent);
      if (hBones != NULL)
      {
         for (int i = 0; i < nBones; i++)
         {
            const char * pszBone = pSkeleton->GetBoneName(i);
            HTREEITEM hBone = GetTreeCtrl().InsertItem(pszBone, hBones);
         }
      }
   }
}

////////////////////////////////////////

void cMs3dTreeView::AddAnimation(ISkeleton * pSkeleton, HTREEITEM hParent)
{
   int nBones = pSkeleton != NULL ? pSkeleton->GetBoneCount() : 0;
   if (nBones > 0)
   {
      cAutoIPtr<IKeyFrameAnimation> pAnim;
      if (pSkeleton->GetAnimation(&pAnim) == S_OK)
      {
         for (int i = 0; i < nBones; i++)
         {
            CString title;
            title.Format("Animation %d", i);

            HTREEITEM hAnim = GetTreeCtrl().InsertItem(title, hParent);
            if (hAnim != NULL)
            {
               cAutoIPtr<IKeyFrameInterpolator> pInterp;
               if (pAnim->GetInterpolator(i, &pInterp) == S_OK)
               {
                  size_t nKeyFrames = 0;

                  if (pInterp->GetKeyFrames(NULL, &nKeyFrames) == S_OK)
                  {
                     sKeyFrame * pKeyFrames = (sKeyFrame *)_alloca(nKeyFrames * sizeof(sKeyFrame));

                     if (pInterp->GetKeyFrames(pKeyFrames, &nKeyFrames) == S_OK)
                     {
                        for (uint j = 0; j < nKeyFrames; j++)
                        {
                           title.Format("Key Frame %d", j);
                           HTREEITEM hKeyFrame = GetTreeCtrl().InsertItem(title, hAnim);
                           if (hKeyFrame != NULL)
                           {
                              title.Format("Position: %.2f, %.2f, %.2f", 
                                 pKeyFrames[j].translation.x,
                                 pKeyFrames[j].translation.y,
                                 pKeyFrames[j].translation.z);
                              GetTreeCtrl().InsertItem(title, hKeyFrame);

                              title.Format("Rotation: %.2f, %.2f, %.2f, %.2f", 
                                 pKeyFrames[j].rotation.w,
                                 pKeyFrames[j].rotation.x,
                                 pKeyFrames[j].rotation.y,
                                 pKeyFrames[j].rotation.z);
                              GetTreeCtrl().InsertItem(title, hKeyFrame);

                              title.Format("Time: %.2f", 
                                 pKeyFrames[j].time);
                              GetTreeCtrl().InsertItem(title, hKeyFrame);
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
}
