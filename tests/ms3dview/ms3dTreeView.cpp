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

static CString VertexUsageToString(tVertexDeclUsage usage)
{
   switch (usage)
   {
   case kVDU_Position: return CString(_T("Position")); break;
   case kVDU_Normal: return CString(_T("Normal")); break;
   case kVDU_Color: return CString(_T("Color")); break;
   case kVDU_TexCoord: return CString(_T("TexCoord")); break;
   case kVDU_Weight: return CString(_T("Weight")); break;
   case kVDU_Index: return CString(_T("Index")); break;
   }
   return CString(_T(""));
}

////////////////////////////////////////

static CString VertexDataTypeToString(tVertexDeclType type)
{
   switch (type)
   {
   case kVDT_Float1: return CString(_T("Float1")); break;
   case kVDT_Float2: return CString(_T("Float2")); break;
   case kVDT_Float3: return CString(_T("Float3")); break;
   case kVDT_Float4: return CString(_T("Float4")); break;
   case kVDT_UnsignedByte4: return CString(_T("UnsignedByte4")); break;
   case kVDT_Short2: return CString(_T("Short2")); break;
   case kVDT_Short4: return CString(_T("Short4")); break;
   }
   return CString(_T(""));
}

////////////////////////////////////////

static void DescribeVertexElement(const sVertexElement & element, CString * pStr)
{
   Assert(pStr != NULL);
   pStr->Empty();
   *pStr += VertexUsageToString(element.usage);
   *pStr += _T(": ");
   *pStr += VertexDataTypeToString(element.type);
}

////////////////////////////////////////

static void DescribeVertexData(const sVertexElement & element, const byte * pData, CString * pStr)
{
   Assert(pStr != NULL);
   pStr->Empty();

   *pStr += VertexUsageToString(element.usage);
   *pStr += _T(": ");

   CString data(_T("UNKNOWN"));

   switch (element.type)
   {
   case kVDT_Float1: data.Format("%f", *(float *)pData); break;
   case kVDT_Float2: data.Format("%f, %f", *(float *)pData, *((float *)pData + 1)); break;
   case kVDT_Float3: data.Format("%f, %f, %f", *(float *)pData, *((float *)pData + 1), *((float *)pData + 2)); break;
   case kVDT_Float4: data.Format("%f, %f, %f, %f", *(float *)pData, *((float *)pData + 1), *((float *)pData + 2), *((float *)pData + 3)); break;
   case kVDT_UnsignedByte4: data.Format("%x", *(uint *)pData); break;
   case kVDT_Short2: data.Format("%d, %d", *(short *)pData); break;
   case kVDT_Short4: data.Format("%d, %d, %d, %d", *(short *)pData, *((short *)pData + 1)); break;
   }

   *pStr += data;
}

////////////////////////////////////////

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
               byte * pVertexData;
               if (pVertexBuffer->Lock(kBL_ReadOnly, (void**)&pVertexData) == S_OK)
               {
                  uint vertexSize = GetVertexSize(elements, nElements);

                  CString item;
                  byte * pVertex = pVertexData;
                  for (uint i = 0; i < pMesh->GetVertexCount(); i++, pVertex += vertexSize)
                  {
                     item.Format("Vertex %d", i);
                     HTREEITEM hVertex = GetTreeCtrl().InsertItem(item, hVertices);
                     if (hVertex != NULL)
                     {
                        byte * pVertexElement = pVertex;
                        for (int j = 0; j < nElements; pVertexElement += GetVertexSize(&elements[j], 1), j++)
                        {
                           DescribeVertexData(elements[j], pVertexElement, &item);
                           if (!item.IsEmpty())
                           {
                              GetTreeCtrl().InsertItem(item, hVertex);
                           }
                        }
                     }
                  }

                  pVertexBuffer->Unlock();
               }
            }
         }
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
