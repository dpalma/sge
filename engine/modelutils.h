////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODELUTILS_H
#define INCLUDED_MODELUTILS_H

#include "engine/modelapi.h"
#include "engine/modeltypes.h"

#include "tech/axisalignedbox.h"
#include "tech/techstring.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cBasicModelRenderer
//

class cBasicModelRenderer
{
   cBasicModelRenderer(const cBasicModelRenderer &);
   const cBasicModelRenderer & operator =(const cBasicModelRenderer &);

public:
   cBasicModelRenderer(IModel * pModel);
   ~cBasicModelRenderer();

   void SetModel(IModel * pModel) { SafeRelease(m_pModel); m_pModel = CTAddRef(pModel); }
   inline IModel * AccessModel() { return m_pModel; }
   tResult GetBoundingBox(tAxisAlignedBox * pBBox) const;
   void Render();

private:
   cAutoIPtr<IModel> m_pModel;
   tAxisAlignedBox m_bbox;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAnimatedModelRenderer
//

typedef std::vector<sBlendedVertex> tBlendedVertices;

class cAnimatedModelRenderer
{
   cAnimatedModelRenderer(const cAnimatedModelRenderer &);
   const cAnimatedModelRenderer & operator =(const cAnimatedModelRenderer &);

public:
   cAnimatedModelRenderer(const tChar * pszModel);
   ~cAnimatedModelRenderer();

   tResult GetBoundingBox(tAxisAlignedBox * pBBox) const;
   void Update(double elapsedTime);
   void Render();
   tResult SetAnimation(eModelAnimationType type);

private:
   cStr m_model;
   cAutoIPtr<IModel> m_pModel;
   std::vector<tMatrix34> m_blendMatrices;
   tBlendedVertices m_blendedVerts;
   cAutoIPtr<IModelAnimationController> m_pAnimController;
   tAxisAlignedBox m_bbox;
   std::vector<sModelVertex> m_modelVertices;
   std::vector<tMatrix34> m_bindMatrices;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODELUTILS_H
