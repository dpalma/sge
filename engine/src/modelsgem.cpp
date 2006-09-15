////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "model.h"

#include "renderapi.h"

#include "globalobj.h"
#include "resourceapi.h"
#include "readwriteapi.h"

#include "dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(ModelSgem);

#define LocalMsg(msg)            DebugMsgEx(ModelSgem,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(ModelSgem,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(ModelSgem,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(ModelSgem,msg,(a),(b),(c))

////////////////////////////////////////////////////////////////////////////////


static const byte g_sgemId[] = "MeGs";

void * ModelSgemLoad(IReader * pReader)
{
   if (pReader == NULL)
   {
      return NULL;
   }

   LocalMsg("Loading SGEM file...\n");

   //////////////////////////////
   // Read the header

   byte id[4];
   uint32 version;
   if (pReader->Read(id, sizeof(id)) != S_OK
      || pReader->Read(&version) != S_OK
      || memcmp(id, g_sgemId, sizeof(g_sgemId)) != 0)
   {
      ErrorMsg("Bad SGEM file header\n");
      return NULL;
   }

   //////////////////////////////

   int nMeshes = 0;
   if (pReader->Read(&nMeshes, sizeof(nMeshes)) != S_OK
      || nMeshes == 0)
   {
      return NULL;
   }

   LocalMsg1("%d Meshes\n", nMeshes);

   tModelMeshes meshes(nMeshes);

   for (int i = 0; i < nMeshes; ++i)
   {
      uint nVertices = 0;
      if (pReader->Read(&nVertices, sizeof(nVertices)) != S_OK
         || nVertices == 0)
      {
         return NULL;
      }

      std::vector<sModelVertex> verts(nVertices);
      if (pReader->Read(&verts[0], nVertices * sizeof(sModelVertex)) != S_OK)
      {
         return NULL;
      }

      static const ePrimitiveType primTypes[] = { kPT_Triangles, kPT_TriangleStrip, kPT_TriangleFan };

      int primType = -1;
      if (pReader->Read(&primType) != S_OK
         || primType < 0 || primType > 2)
      {
         return NULL;
      }

      int nIndices = 0;
      if (pReader->Read(&nIndices) != S_OK)
      {
         return NULL;
      }

      std::vector<uint16> indices(nIndices);
      if (pReader->Read(&indices[0], nIndices * sizeof(uint16)) != S_OK)
      {
         return NULL;
      }

      meshes[i] = cModelMesh(primTypes[primType], indices, -1);
   }

   return NULL;
}

///////////////////////////////////////

void ModelSgemUnload(void * pData)
{
   cModel * pModel = reinterpret_cast<cModel*>(pData);
   delete pModel;
}


////////////////////////////////////////////////////////////////////////////////

tResult ModelSgemResourceRegister()
{
   UseGlobal(ResourceManager);
   return pResourceManager->RegisterFormat(kRT_Model, _T("sgem"), ModelSgemLoad, NULL, ModelSgemUnload);
}

////////////////////////////////////////////////////////////////////////////////
