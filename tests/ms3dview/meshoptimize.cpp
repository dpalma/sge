/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "meshapi.h"

#include "renderapi.h"

#include "comtools.h"
#include "techmath.h"

#include "dbgalloc.h" // must be last header

const uint INVALID_UINT = ~0;

/////////////////////////////////////////////////////////////////////////////

tResult MeshOptimize(IMesh * pMesh)
{
   if (pMesh == NULL)
   {
      return E_POINTER;
   }

   tResult result = E_FAIL;

   cAutoIPtr<IVertexBuffer> pVertexBuffer;
   if (pMesh->GetVertexBuffer(&pVertexBuffer) == S_OK)
   {
      DebugMsg("cMesh::Optimize()\n");

      cAutoIPtr<IVertexDeclaration> pVertexDecl;
      if (pVertexBuffer->GetVertexDeclaration(&pVertexDecl) == S_OK)
      {
         sVertexElement elements[256];
         int nElements = _countof(elements);

         if (pVertexDecl->GetElements(elements, &nElements) == S_OK)
         {
            uint blendIndexOffset = INVALID_UINT;
            for (int i = 0; i < nElements; i++)
            {
               if (elements[i].usage == kVDU_Index
                  && elements[i].type == kVDT_Float1)
               {
                  blendIndexOffset = elements[i].offset;
                  break;
               }
            }

            if (blendIndexOffset != INVALID_UINT)
            {
               byte * pVertexData;
               if (pVertexBuffer->Lock(kBL_ReadOnly, (void**)&pVertexData) == S_OK)
               {
                  uint vertexSize = GetVertexSize(elements, nElements);

                  byte * pTempVertex = reinterpret_cast<byte *>(alloca(vertexSize));

                  int * vertexMap = reinterpret_cast<int *>(alloca(sizeof(int) * pMesh->GetVertexCount()));

                  uint i, j;

                  for (i = 0; i < pMesh->GetVertexCount(); i++)
                  {
                     vertexMap[i] = i;
                  }

#ifdef BUBBLE_SORT
                  for (i = GetVertexCount() - 1; i >= 0; i--)
                  {
                     for (j = 0; j < i; j++)
                     {
                        byte * pVertexJ = pVertexData + (j * vertexSize);

                        float * pBlendIndexJ = (float *)(pVertexJ + blendIndexOffset);
                        int blendIndexJ = Round(*pBlendIndexJ);

                        float * pBlendIndexJPlus1 = (float *)(pVertexJ + vertexSize + blendIndexOffset);
                        int blendIndexJPlus1 = Round(*pBlendIndexJPlus1);

                        if (blendIndexJ > blendIndexJPlus1)
                        {
                           int temp = vertexMap[j];
                           vertexMap[j] = vertexMap[j + 1];
                           vertexMap[j + 1] = temp;

                           memcpy(pTempVertex, pVertexJ, vertexSize);
                           memcpy(pVertexJ, pVertexJ + vertexSize, vertexSize);
                           memcpy(pVertexJ + vertexSize, pTempVertex, vertexSize);
                        }
                     }
                  }
#else
                  // selection sort
                  for (i = 0; i < pMesh->GetVertexCount(); i++)
                  {
                     uint iMin = i;

                     byte * pVertexI = pVertexData + (i * vertexSize);
                     int blendIndexI = Round(*(float *)(pVertexI + blendIndexOffset));

                     byte * pMinVertex = pVertexI;
                     int minBlendIndex = blendIndexI;

                     for (j = i + 1; j < pMesh->GetVertexCount(); j++)
                     {
                        byte * pVertexJ = pVertexData + (j * vertexSize);
                        int blendIndexJ = Round(*(float *)(pVertexJ + blendIndexOffset));

                        if (blendIndexJ < minBlendIndex)
                        {
                           iMin = j;
                           pMinVertex = pVertexJ;
                           minBlendIndex = blendIndexJ;
                        }
                     }

                     if (i != iMin)
                     {
                        vertexMap[iMin] = i;
                        vertexMap[i] = iMin;

//                        int temp = vertexMap[iMin];
//                        vertexMap[iMin] = vertexMap[i];
//                        vertexMap[i] = temp;

                        memcpy(pTempVertex, pMinVertex, vertexSize);
                        memcpy(pMinVertex, pVertexI, vertexSize);
                        memcpy(pVertexI, pTempVertex, vertexSize);
                     }
                  }
#endif

                  pVertexBuffer->Unlock();

                  for (i = 0; i < pMesh->GetSubMeshCount(); i++)
                  {
                     cAutoIPtr<ISubMesh> pSubMesh;
                     if (pMesh->GetSubMesh(i, &pSubMesh) == S_OK)
                     {
                        // TODO HACK: assumes index buffer uses 16-bit indices
                        uint16 * pIndices = NULL;
                        if (pSubMesh->LockIndexBuffer(kBL_Default, (void**)&pIndices) == S_OK)
                        {
                           for (j = 0; j < pSubMesh->GetIndexCount(); j++)
                           {
                              uint16 oldIndex = pIndices[j];
                              pIndices[j] = vertexMap[oldIndex];
                              DebugMsg2("   Vertex %d remapped to %d\n", oldIndex, vertexMap[oldIndex]);
                           }

                           pSubMesh->UnlockIndexBuffer();
                        }
                     }
                  }

                  result = S_OK;
               }
            }
         }
      }
   }

   return result;
}

/////////////////////////////////////////////////////////////////////////////
