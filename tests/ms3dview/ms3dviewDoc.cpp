/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "ms3dviewDoc.h"

#include "ms3dviewView.h" // TODO HACK: really need to access the view?

#include "resourceapi.h"
#include "renderapi.h"
#include "filespec.h"
#include "filepath.h"
#include "globalobj.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMs3dviewDoc

IMPLEMENT_DYNCREATE(CMs3dviewDoc, CDocument)

BEGIN_MESSAGE_MAP(CMs3dviewDoc, CDocument)
	//{{AFX_MSG_MAP(CMs3dviewDoc)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_OPTIMIZE, OnUpdateToolsOptimize)
	ON_COMMAND(ID_TOOLS_OPTIMIZE, OnToolsOptimize)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_RENDERING, OnUpdateRendering)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMs3dviewDoc construction/destruction

CMs3dviewDoc::CMs3dviewDoc()
{
}

CMs3dviewDoc::~CMs3dviewDoc()
{
}

BOOL CMs3dviewDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

   SafeRelease(m_pMesh);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMs3dviewDoc diagnostics

#ifdef _DEBUG
void CMs3dviewDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMs3dviewDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMs3dviewDoc commands

BOOL CMs3dviewDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
   CMs3dviewView * pMs3dView = NULL;

	POSITION pos = GetFirstViewPosition();
   for (CView * pView = GetNextView(pos); pView != NULL; pView = GetNextView(pos))
   {
      pMs3dView = DYNAMIC_DOWNCAST(CMs3dviewView, pView);
      if (pMs3dView != NULL)
      {
         break;
      }
   }

   if (!pMs3dView)
   {
      DebugMsg("Have no appropriate view from which to get a rendering device\n");
      return FALSE;
   }

   DeleteContents();
   SetModifiedFlag();  // dirty during de-serialize

   cFileSpec file(lpszPathName);
   cFilePath path(file.GetPath());

   UseGlobal(ResourceManager);
   pResourceManager->AddDirectory(path.c_str());

   Assert(!m_pMesh);
   if (pResourceManager->LoadUncached(tResKey(file.GetFileName(), kRC_Mesh), pMs3dView->AccessRenderDevice(), (void**)&m_pMesh, NULL) != S_OK)
   {
   	DeleteContents();
      return E_FAIL;
   }

   PostRead();

   SetModifiedFlag(FALSE);

   return TRUE;
}

void CMs3dviewDoc::DeleteContents() 
{
   SafeRelease(m_pMesh);
	
	CDocument::DeleteContents();
}

void CMs3dviewDoc::OnUpdateRendering(CCmdUI * pCmdUI)
{
   // TODO
#if 0
   UINT stringId = (m_pMesh != NULL) && m_pMesh->IsRenderingSoftware() ? IDS_RENDERING_SOFTWARE : IDS_RENDERING_VERTEX_PROGRAM;
   CString string;
   VERIFY(string.LoadString(stringId));
   pCmdUI->SetText(string);
#endif
}

void CMs3dviewDoc::OnUpdateToolsOptimize(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!!m_pMesh);
}

void CMs3dviewDoc::OnToolsOptimize() 
{
	// TODO: Add your command handler code here
	
}

void CMs3dviewDoc::SetFrame(float pct)
{
   if (GetModel())
   {
      cAutoIPtr<ISkeleton> pSkeleton;
      cAutoIPtr<IKeyFrameAnimation> pAnimation;
      if (GetModel()->GetSkeleton(&pSkeleton) == S_OK
         && pSkeleton->GetAnimation(&pAnimation) == S_OK)
      {
         pSkeleton->GetBoneMatrices(pct * pAnimation->GetPeriod(), &m_boneMatrices);
      }
   }
}

tResult CMs3dviewDoc::PostRead()
{
   cAutoIPtr<ISkeleton> pSkeleton;
   if (m_pMesh->GetSkeleton(&pSkeleton) == S_OK)
   {
      m_boneMatrices.resize(pSkeleton->GetBoneCount());

      tMatrices inverses(pSkeleton->GetBoneCount());

      for (uint i = 0; i < inverses.size(); i++)
      {
         MatrixInvert(pSkeleton->GetBoneWorldTransform(i).m, inverses[i].m);
      }

      if (m_pMesh)
      {
         cAutoIPtr<IVertexBuffer> pVB;
         cAutoIPtr<IVertexDeclaration> pVertexDecl;

         // TODO: Handle sub-meshes too (not all meshes have a single shared vertex buffer)

         if (m_pMesh->GetVertexBuffer(&pVB) == S_OK)
         {
            sVertexElement elements[256];
            int nElements = _countof(elements);
            uint vertexSize;

            if (pVB->GetVertexDeclaration(&pVertexDecl) == S_OK
               && pVertexDecl->GetElements(elements, &nElements) == S_OK
               && pVertexDecl->GetVertexSize(&vertexSize) == S_OK)
            {
               uint positionOffset, normalOffset, indexOffset;

               for (int i = 0; i < nElements; i++)
               {
                  switch (elements[i].usage)
                  {
                     case kVDU_Position:
                     {
                        positionOffset = elements[i].offset;
                        break;
                     }

                     case kVDU_Normal:
                     {
                        normalOffset = elements[i].offset;
                        break;
                     }

                     case kVDU_Index:
                     {
                        indexOffset = elements[i].offset;
                        break;
                     }
                  }
               }

               // transform all vertices by the inverse of the affecting bone's absolute matrix
               byte * pVertexData;
               if (m_pMesh->LockVertexBuffer(kBL_Default, (void**)&pVertexData) == S_OK)
               {
                  for (uint i = 0; i < m_pMesh->GetVertexCount(); i++)
                  {
                     byte * pVertexBase = pVertexData + (i * vertexSize);

                     float * pPosition = reinterpret_cast<float *>(pVertexBase + positionOffset);
                     float * pNormal = reinterpret_cast<float *>(pVertexBase + normalOffset);
                     const float * pIndex = reinterpret_cast<const float *>(pVertexBase + indexOffset);

                     int index = (int)*pIndex;

                     // TODO: No size-checking is done for position and normal members
                     // (i.e., float1, float2, float3, etc.)

                     if (index >= 0)
                     {
                        tVec4 normal(pNormal[0],pNormal[1],pNormal[2],1);
                        tVec4 position(pPosition[0],pPosition[1],pPosition[2],1);

                        tVec4 nprime;
                        inverses[index].Transform(normal, &nprime);
                        memcpy(pNormal, nprime.v, 3 * sizeof(float));

                        tVec4 vprime;
                        inverses[index].Transform(position, &vprime);
                        memcpy(pPosition, vprime.v, 3 * sizeof(float));
                     }
                  }

                  m_pMesh->UnlockVertexBuffer();
               }
            }
         }
      }

      SetFrame(0);
   }

   return S_OK;
}
