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
// CLASS: cEntityModel
//
// Helper class for cEntity that handles details of rendering an animated model

class cEntityModel
{
public:
   cEntityModel(const tChar * pszModel);
   ~cEntityModel();
   const cStr & GetModel() const { return m_model; }
   const tAxisAlignedBox & GetBoundingBox() const;
   void Update(double elapsedTime);
   void Render();
private:
   cStr m_model;
   cModel * m_pModel;
   tBlendedVertices m_blendedVerts;
   cAutoIPtr<IModelAnimationController> m_pAnimController;
   tAxisAlignedBox m_bbox;
};


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntity
//

class cEntity : public cComObject<IMPLEMENTS(IEntity)>
{
public:
   cEntity(tEntityId id, const tChar * pszModel, const tVec3 & position);
   ~cEntity();

   virtual tEntityId GetId() const;

   virtual uint GetFlags() const;
   virtual uint SetFlags(uint flags, uint mask);

   virtual tResult GetModel(cStr * pModel) const;

   virtual tResult GetPosition(tVec3 * pPosition) const;

   virtual const tMatrix4 & GetWorldTransform() const;

   virtual const tAxisAlignedBox & GetBoundingBox() const;

   virtual void Update(double elapsedTime);
   virtual void Render();

   virtual tResult AddComponent(REFGUID guid, IEntityComponent * pComponent);
   virtual tResult FindComponent(REFGUID guid, IEntityComponent * * ppComponent);

private:
   tEntityId m_id;
   uint m_flags;

   tVec3 m_position;

   mutable bool m_bUpdateWorldTransform;
   mutable tMatrix4 m_worldTransform;

   cEntityModel m_modelHelper;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITY_H
