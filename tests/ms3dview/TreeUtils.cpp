/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "TreeUtils.h"

#include "renderapi.h"

#include "resource.h"       // main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

CString VertexUsageToString(tVertexDeclUsage usage)
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

/////////////////////////////////////////////////////////////////////////////

CString VertexDataTypeToString(tVertexDeclType type)
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

/////////////////////////////////////////////////////////////////////////////

void DescribeVertexElement(const sVertexElement & element, CString * pStr)
{
   Assert(pStr != NULL);
   pStr->Empty();
   *pStr += VertexUsageToString(element.usage);
   *pStr += _T(": ");
   *pStr += VertexDataTypeToString(element.type);
}

/////////////////////////////////////////////////////////////////////////////

void DescribeVertexData(const sVertexElement & element, const byte * pData, CString * pStr)
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

/////////////////////////////////////////////////////////////////////////////

void AddVertices(uint nVertices, IVertexBuffer * pVertexBuffer, CTreeCtrl * pTreeCtrl, HTREEITEM hParent)
{
   if (nVertices > 0 && pVertexBuffer != NULL && pTreeCtrl != NULL)
   {
      ASSERT_VALID(pTreeCtrl);

      cAutoIPtr<IVertexDeclaration> pVertexDecl;
      if (pVertexBuffer->GetVertexDeclaration(&pVertexDecl) == S_OK)
      {
         sVertexElement elements[256];
         int nElements = _countof(elements);

         HTREEITEM hVertexDecl = pTreeCtrl->InsertItem("Vertex Declaration", hParent);

         if (pVertexDecl->GetElements(elements, &nElements) == S_OK)
         {
            for (int i = 0; i < nElements; i++)
            {
               CString str;
               DescribeVertexElement(elements[i], &str);
               if (!str.IsEmpty() && hVertexDecl != NULL)
               {
                  pTreeCtrl->InsertItem(str, hVertexDecl);
               }
            }
         }

         HTREEITEM hVertices = pTreeCtrl->InsertItem("Vertices", hParent);
         if (hVertices != NULL)
         {
            byte * pVertexData;
            if (pVertexBuffer->Lock(kBL_ReadOnly, (void**)&pVertexData) == S_OK)
            {
               uint vertexSize = GetVertexSize(elements, nElements);

               CString item;
               byte * pVertex = pVertexData;
               for (uint i = 0; i < nVertices; i++, pVertex += vertexSize)
               {
                  item.Format("Vertex %d", i);
                  HTREEITEM hVertex = pTreeCtrl->InsertItem(item, hVertices);
                  if (hVertex != NULL)
                  {
                     byte * pVertexElement = pVertex;
                     for (int j = 0; j < nElements; pVertexElement += GetVertexSize(&elements[j], 1), j++)
                     {
                        DescribeVertexData(elements[j], pVertexElement, &item);
                        if (!item.IsEmpty())
                        {
                           pTreeCtrl->InsertItem(item, hVertex);
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

/////////////////////////////////////////////////////////////////////////////
