////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYSELECTION_H
#define INCLUDED_ENTITYSELECTION_H

#include "engine/entityapi.h"

#include "tech/comenum.h"
#include "tech/connptimpl.h"
#include "tech/globalobjdef.h"

#include <set>

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////

typedef std::set<IEntity*, cCTLessInterface> tEntitySet;

typedef cComEnum<IEnumEntities,
                 &IID_IEnumEntities,
                 IEntity*,
                 CopyInterface<IEntity>,
                 tEntitySet> tComEnumEntitySet;

typedef cComObject<tComEnumEntitySet, &IID_IEnumEntities> tEntitySetEnum;

template <>
void tComEnumEntitySet::Initialize(tEntitySet::const_iterator iter,
                                   tEntitySet::const_iterator last)
{
   for (; iter != last; ++iter)
   {
      IEntity * t = NULL;
      CopyInterface<IEntity>::Copy(&t, &(*iter));
      m_elements.insert(t);
   }
   m_iterator = m_elements.begin();
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntitySelection
//

class cEntitySelection : public cComObject3<IMPLEMENTSCP(IEntitySelection, IEntitySelectionListener),
                                            IMPLEMENTS(IGlobalObject),
                                            IMPLEMENTS(IEntityManagerListener)>
{
public:
   cEntitySelection();
   ~cEntitySelection();

   DECLARE_NAME(EntitySelection)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult AddEntitySelectionListener(IEntitySelectionListener * pListener);
   virtual tResult RemoveEntitySelectionListener(IEntitySelectionListener * pListener);

   virtual tResult Select(IEntity * pEntity);
   virtual tResult SelectBoxed(const tAxisAlignedBox & box);
   virtual tResult DeselectAll();
   virtual uint GetSelectedCount() const;
   virtual tResult SetSelected(IEnumEntities * pEnum);
   virtual tResult GetSelected(IEnumEntities * * ppEnum) const;
   virtual tResult IsSelected(IEntity * pEntity) const;

   virtual void OnRemoveEntity(IEntity * pEntity);

private:

   tEntitySet m_selected;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYSELECTION_H
