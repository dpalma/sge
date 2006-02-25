////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITY_H
#define INCLUDED_ENTITY_H

#include "entityapi.h"

#include "model.h"

#include "axisalignedbox.h"
#include "techstring.h"

#ifdef _MSC_VER
#pragma once
#endif


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelEntity
//

class cModelEntity : public cComObject<IMPLEMENTS(IEntity)>
{
public:
   cModelEntity(tEntityId id, const tChar * pszModel, const tVec3 & position);
   ~cModelEntity();

   virtual tEntityId GetId() const;

   virtual uint GetFlags() const;
   virtual uint SetFlags(uint flags, uint mask);

   virtual tResult GetModel(cStr * pModel) const;

   virtual tResult GetPosition(tVec3 * pPosition) const;

   virtual const tMatrix4 & GetWorldTransform() const;

   virtual const tAxisAlignedBox & GetBoundingBox() const;

   virtual void Update(double elapsedTime);
   virtual void Render();

private:
   cStr m_model;
   cModel * m_pModel;
   tBlendedVertices m_blendedVerts;
   cAutoIPtr<IModelAnimationController> m_pAnimController;

   tEntityId m_id;
   uint m_flags;

   tVec3 m_position;

   tAxisAlignedBox m_bbox;

   mutable bool m_bUpdateWorldTransform;
   mutable tMatrix4 m_worldTransform;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITY_H
