///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENEMESH_H
#define INCLUDED_SCENEMESH_H

#include "scenenode.h"
#include "comtools.h"
#include "sim.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IMesh);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneMesh
//

class cSceneMesh : public cSceneNode
{
   cSceneMesh(const cSceneMesh &);
   const cSceneMesh & operator =(const cSceneMesh &);

public:
   cSceneMesh();
   virtual ~cSceneMesh();

   bool SetMesh(const char * pszMesh);
   IMesh * AccessMesh();

   virtual void Render();

   virtual float GetBoundingSphereRadius() const;

private:
   tResult PostRead();

   void Animate(double elapsedTime);

   class cSimClient : public cComObject<IMPLEMENTS(ISimClient)>
   {
      void CDECL operator delete(void *) { Assert(!"Should never be called"); }
   public:
      virtual void DeleteThis() { /* do not delete */ }
      virtual void OnFrame(double elapsedTime);
   };

   friend class cSimClient;
   cSimClient m_simClient;

   cAutoIPtr<IMesh> m_pMesh;
   float m_boundingSphereRadius;

   std::vector<tMatrix4> m_boneMatrices;
   float m_animationTime;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENEMESH_H
