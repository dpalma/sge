/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "ms3dTreeView.h"
#include "ms3dviewDoc.h"

#include "skeleton.h"
#include "animation.h"

#include "render.h"
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

void cMs3dTreeView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////

BOOL cMs3dTreeView::PreCreateWindow(CREATESTRUCT& cs) 
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
	
	// TODO: Add your specialized code here and/or call the base class

	CMs3dviewDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   VERIFY(GetTreeCtrl().DeleteAllItems());

   if (pDoc && pDoc->GetModel())
   {
      AddVertices(pDoc->GetModel());
      AddMaterials(pDoc->GetModel());
      AddGroups(pDoc->GetModel());
      AddSkeleton(pDoc->GetModel());
   }
}

static void DescribeVertexElement(const sVertexElement & element, CString * pStr)
{
   Assert(pStr != NULL);
   pStr->Empty();

   switch (element.usage)
   {
   case kVDU_Position: *pStr += _T("Position"); break;
   case kVDU_Normal: *pStr += _T("Normal"); break;
   case kVDU_Color: *pStr += _T("Color"); break;
   case kVDU_TexCoord: *pStr += _T("TexCoord"); break;
   case kVDU_Weight: *pStr += _T("Weight"); break;
   case kVDU_Index: *pStr += _T("Index"); break;
   }

   switch (element.type)
   {
   case kVDT_Float1: *pStr += _T(": Float1"); break;
   case kVDT_Float2: *pStr += _T(": Float2"); break;
   case kVDT_Float3: *pStr += _T(": Float3"); break;
   case kVDT_Float4: *pStr += _T(": Float4"); break;
   case kVDT_UnsignedByte4: *pStr += _T(": UnsignedByte4"); break;
   case kVDT_Short2: *pStr += _T(": Short2"); break;
   case kVDT_Short4: *pStr += _T(": Short4"); break;
   }
}

void cMs3dTreeView::AddVertices(IMesh * pMesh, HTREEITEM hParent)
{
   int nVertices = pMesh->GetVertexCount();
   if (nVertices > 0)
   {
      cAutoIPtr<IVertexBuffer> pVertexBuffer;
      if (pMesh->GetVertexBuffer(&pVertexBuffer) == S_OK)
      {
         cAutoIPtr<IVertexDeclaration> pVertexDecl;
         if (pVertexBuffer->GetVertexDeclaration(&pVertexDecl) == S_OK)
         {
            sVertexElement elements[256];
            int nElements = _countof(elements);

            HTREEITEM hVertexDecl = GetTreeCtrl().InsertItem("Vertex Declaration", hParent);

            if (pVertexDecl->GetElements(elements, &nElements) == S_OK)
            {
               for (int i = 0; i < nElements; i++)
               {
                  CString str;
                  DescribeVertexElement(elements[i], &str);
                  if (!str.IsEmpty() && hVertexDecl != NULL)
                  {
                     GetTreeCtrl().InsertItem(str, hVertexDecl);
                  }
               }
            }

            HTREEITEM hVertices = GetTreeCtrl().InsertItem("Vertices", hParent);
            if (hVertices != NULL)
            {
            }
         }
      }
   }
}

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

void cMs3dTreeView::AddGroups(cMs3dMesh * pMesh, HTREEITEM hParent)
{
   int nGroups = pMesh->GetGroupCount();
   if (nGroups > 0)
   {
      HTREEITEM hGroups = GetTreeCtrl().InsertItem("Groups", hParent);
      if (hGroups != NULL)
      {
         for (int i = 0; i < nGroups; i++)
         {
            const cMs3dGroup & g = pMesh->GetGroup(i);
            HTREEITEM hGroup = GetTreeCtrl().InsertItem(g.GetName(), hGroups);
            if (hGroup != NULL)
            {
               CString str;
               str.Format("Material index: %d", g.GetMaterialIndex());
               GetTreeCtrl().InsertItem(str, hGroup);
               str.Format("Triangle count: %d", g.GetTriangleIndices().size());
               GetTreeCtrl().InsertItem(str, hGroup);
            }
         }
      }
   }
}

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
