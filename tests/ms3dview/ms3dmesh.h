///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DMESH_H
#define INCLUDED_MS3DMESH_H

#include "mesh.h"

#include "readwriteapi.h"
#include "matrix4.h"
#include "vec3.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IResourceManager);
F_DECLARE_INTERFACE(IRenderDevice);
F_DECLARE_INTERFACE(ISkeleton);

typedef struct _CGprogram * CGprogram;
typedef struct _CGparameter * CGparameter;

///////////////////////////////////////////////////////////////////////////////
// MS3D file format structs

#pragma pack(push,1)

typedef struct
{
   char    id[10];                  // always "MS3D000000"
   int     version;                 // 4
} ms3d_header_t;

typedef struct
{
   byte    flags;                   // SELECTED | SELECTED2 | HIDDEN
   float   vertex[3];               //
   char    boneId;                  // -1 = no bone
   byte    referenceCount;
} ms3d_vertex_t;

typedef struct
{
   uint16    flags;                 // SELECTED | SELECTED2 | HIDDEN
   uint16    vertexIndices[3];      //
   float   vertexNormals[3][3];     //
   float   s[3];                    //
   float   t[3];                    //
   byte    smoothingGroup;          // 1 - 32
   byte    groupIndex;              //
} ms3d_triangle_t;

typedef struct
{
   char            name[32];        //
   float           ambient[4];      //
   float           diffuse[4];      //
   float           specular[4];     //
   float           emissive[4];     //
   float           shininess;       // 0.0f - 128.0f
   float           transparency;    // 0.0f - 1.0f
   char            mode;            // 0, 1, 2 is unused now
   char            texture[128];    // texture.bmp
   char            alphamap[128];   // alpha.bmp
} ms3d_material_t;

typedef struct
{
   float           time;            // time in seconds
   float           rotation[3];     // x, y, z angles
} ms3d_keyframe_rot_t;

typedef struct
{
   float           time;            // time in seconds
   float           position[3];     // local position
} ms3d_keyframe_pos_t;

#pragma pack(pop)

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dGroup
//

class cMs3dGroup
{
   friend class cReadWriteOps<cMs3dGroup>;

public:
   cMs3dGroup()
   {
   }

   const char * GetName() const { return name; }

   int GetMaterialIndex() const
   {
      return materialIndex;
   }

   const std::vector<uint16> & GetTriangleIndices() const
   {
      return triangleIndices;
   }

private:
   char name[32];
   std::vector<uint16> triangleIndices;
   char materialIndex;
};


//////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dMesh
//

class cMs3dMesh : public cComObject<IMPLEMENTS(IMesh)>
{
   cMs3dMesh(const cMs3dMesh &); // private, un-implemented
   const cMs3dMesh & operator =(const cMs3dMesh &); // private, un-implemented

public:
   cMs3dMesh();
   ~cMs3dMesh();

   virtual void GetAABB(tVec3 * pMaxs, tVec3 * pMins) const;
   virtual void Render(IRenderDevice * pRenderDevice) const;
   virtual uint GetVertexCount() const;
   virtual tResult GetVertexBuffer(IVertexBuffer * * ppVertexBuffer);
   virtual tResult LockVertexBuffer(uint lock, void * * ppData);
   virtual tResult UnlockVertexBuffer();
   virtual tResult AddMaterial(IMaterial * pMaterial);
   virtual tResult FindMaterial(const char * pszName, IMaterial * * ppMaterial) const;
   virtual uint GetMaterialCount() const;
   virtual tResult GetMaterial(uint index, IMaterial * * ppMaterial) const;
   virtual tResult AddSubMesh(ISubMesh * pSubMesh);
   virtual uint GetSubMeshCount() const;
   virtual tResult GetSubMesh(uint index, ISubMesh * * ppSubMesh) const;
   virtual tResult AttachSkeleton(ISkeleton * pSkeleton);
   virtual tResult GetSkeleton(ISkeleton * * ppSkeleton);

   tResult Load(const char * pszMesh, IRenderDevice * pRenderDevice, IResourceManager * pResourceManager);

   tResult PostRead();

   void SetFrame(float percent);

   // using software or vertex program rendering?
   bool IsRenderingSoftware() const;

   int GetGroupCount() const { return 0; }
   cMs3dGroup GetGroup(int index) const { return cMs3dGroup(); }

private:
   void RenderVertexProgram() const;

   cAutoIPtr<IMesh> m_pInnerMesh;

   std::vector<tMatrix4> m_boneMatrices;

   CGprogram m_program;
   CGparameter m_modelViewProjParam;
};

inline bool cMs3dMesh::IsRenderingSoftware() const
{
   return true;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DMESH_H
