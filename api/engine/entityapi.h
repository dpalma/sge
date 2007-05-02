///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYAPI_H
#define INCLUDED_ENTITYAPI_H

#include "enginedll.h"

#include "modelapi.h"

#include "tech/comtools.h"
#include "tech/matrix4.h"
#include "tech/quat.h"
#include "tech/techstring.h"
#include "tech/vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE_GUID(IEntity, "85E6F9DB-639F-411d-B365-86A8FBD1ACBF");
F_DECLARE_INTERFACE_GUID(IUpdatable, "0A85E22A-F905-458c-B96B-E0EBB4FECE0B");
F_DECLARE_INTERFACE_GUID(IEntityComponent, "D1A48ABA-7DB7-4fb7-96E3-72F79DFABA99");
F_DECLARE_INTERFACE_GUID(IEnumEntityComponents, "8E7028A3-ADEA-4d4c-8A92-65AFD3417555");
F_DECLARE_INTERFACE_GUID(IEntityComponentFactory, "6AA1EDA8-EEA0-404c-B92D-5275F8CF75D6");
F_DECLARE_INTERFACE_GUID(IEntityComponentRegistry, "B0263197-201A-4ca4-8AD6-896F03CDDD19");
F_DECLARE_INTERFACE_GUID(IEntityPositionComponent, "BA4B742C-8D6F-494a-827B-25F8A3B4801F");
F_DECLARE_INTERFACE_GUID(IEntityRenderComponent, "AF68F8F0-EFA5-49c6-AA91-C4E21BAF6D14");
F_DECLARE_INTERFACE_GUID(IEntitySpawnComponent, "612C76A2-151D-4322-9687-3374463BF7BA");
F_DECLARE_INTERFACE_GUID(IEntityBrainComponent, "4E16D11A-08F2-4321-AA91-B670D068B505");
F_DECLARE_INTERFACE_GUID(IEnumEntities, "FC22B764-3FBA-43da-97B0-56857D0A77E9");
F_DECLARE_INTERFACE_GUID(IEntityCommandManager, "4A55B143-2588-4fad-828A-E0C6C6539284");
F_DECLARE_INTERFACE_GUID(IEntityCommandUI, "06188D53-13DA-4e67-975B-A653460F8967");
F_DECLARE_INTERFACE_GUID(IEntityFactory, "B44642DB-A9BE-4e57-A70F-766837E5B1AE");
F_DECLARE_INTERFACE_GUID(IEntityManager, "92DB7247-E01C-4935-B35C-EB233295A4BE");
F_DECLARE_INTERFACE_GUID(IEntityManagerListener, "1EC6DB1A-C833-4b68-8705-D1A9FB5CC8D3");

class cMultiVar;
class cRay;
class TiXmlElement;

template <typename T> class cAxisAlignedBox;
typedef class cAxisAlignedBox<float> tAxisAlignedBox;


///////////////////////////////////////////////////////////////////////////////
//
// ENUM: eEntityComponentType
//

enum eEntityComponentType
{
   kECT_Position,
   kECT_Render,
   kECT_Spawn,             // for entities that spawn others
   kECT_Brain,
   kECT_Custom1,           // for use by application
   kECT_Custom2,           // for use by application
   kMaxEntityComponentTypes
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityComponent
//

interface IEntityComponent : IUnknown
{
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEnumEntityComponents
//

interface IEnumEntityComponents : IUnknown
{
   virtual tResult Next(ulong count, IEntityComponent * * ppComponents, ulong * pnComponents) = 0;
   virtual tResult Skip(ulong count) = 0;
   virtual tResult Reset() = 0;
   virtual tResult Clone(IEnumEntityComponents * * ppEnum) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityPositionComponent
//

interface IEntityPositionComponent : IEntityComponent
{
   virtual tResult SetPosition(const tVec3 & position) = 0;
   virtual tResult GetPosition(tVec3 * pPosition) const = 0;

   virtual tResult SetOrientation(const tQuat & orientation) = 0;
   virtual tResult GetOrientation(tQuat * pOrientation) const = 0;

   virtual const tMatrix4 & GetWorldTransform() const = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityRenderComponent
//

enum eEntityRenderFlags
{
   kERF_None         = 0,
   kERF_Selected     = (1 << 0),
};

interface IEntityRenderComponent : IEntityComponent
{
   virtual tResult GetBoundingBox(tAxisAlignedBox * pBBox) const = 0;

   virtual void Render(uint flags) = 0;
};

ENGINE_API tResult EntityCreateRenderComponent(const tChar * pszModel, IEntityRenderComponent * * ppRenderComponent);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntitySpawnComponent
//

interface IEntitySpawnComponent : IEntityComponent
{
   virtual uint GetMaxQueueSize() const = 0;

   virtual tResult SetRallyPoint(const tVec3 & rallyPoint) = 0;
   virtual tResult GetRallyPoint(tVec3 * pRallyPoint) const = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityBrainComponent
//

interface IEntityBrainComponent : IEntityComponent
{
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityComponentFactory
//

interface IEntityComponentFactory : IUnknown
{
   virtual tResult CreateComponent(const TiXmlElement * pTiXmlElement, IEntity * pEntity, IEntityComponent * * ppComponent) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityComponentRegistry
//

interface IEntityComponentRegistry : IUnknown
{
   virtual tResult RegisterComponentFactory(const tChar * pszComponent,
                                            IEntityComponentFactory * pFactory) = 0;
   virtual tResult RevokeComponentFactory(const tChar * pszComponent) = 0;

   virtual tResult CreateComponent(const TiXmlElement * pTiXmlElement, IEntity * pEntity,
                                   IEntityComponent * * ppComponent) = 0;
};

ENGINE_API tResult EntityComponentRegistryCreate();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntity
//

typedef ulong tEntityId;
const ulong kInvalidEntityId = 0;

interface IEntity : IUnknown
{
   virtual tResult GetTypeName(cStr * pTypeName) const = 0;

   virtual tEntityId GetId() const = 0;

   virtual tResult SetComponent(eEntityComponentType ect, IEntityComponent * pComponent) = 0;
   virtual tResult GetComponent(eEntityComponentType ect, IEntityComponent * * ppComponent) = 0;

   template <class INTRFC>
   tResult GetComponent(eEntityComponentType ect, REFGUID iid, INTRFC * * ppComponent)
   {
      cAutoIPtr<IEntityComponent> pComponent;
      tResult result = GetComponent(ect, &pComponent);
      if (result == S_OK)
      {
         return pComponent->QueryInterface(iid, reinterpret_cast<void**>(ppComponent));
      }
      return result;
   }

   virtual tResult RemoveComponent(eEntityComponentType ect) = 0;

   virtual tResult EnumComponents(REFGUID iid, IEnumEntityComponents * * ppEnum) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IUpdatable
//

interface IUpdatable : IUnknown
{
   virtual void Update(double elapsedTime) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEnumEntities
//

interface IEnumEntities : IUnknown
{
   virtual tResult Next(ulong count, IEntity * * ppEntities, ulong * pnEntities) = 0;
   virtual tResult Skip(ulong count) = 0;
   virtual tResult Reset() = 0;
   virtual tResult Clone(IEnumEntities * * ppEnum) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityCommandManager
//

typedef tResult (* tEntityCommandFn)(IEntity * pEntity, const cMultiVar * pArgs, uint nArgs);

DECLARE_HANDLE(tEntityCmdInstance);

interface IEntityCommandManager : IUnknown
{
   virtual tResult RegisterCommand(const tChar * pszCommand, tEntityCommandFn pfnCommand) = 0;
   virtual tResult RevokeCommand(const tChar * pszCommand) = 0;

   virtual tResult CompileCommand(const tChar * pszCommand, const cMultiVar * pArgs, uint nArgs, tEntityCmdInstance * pCmdInst) = 0;
   virtual tResult ExecuteCommand(tEntityCmdInstance cmdInst, IEntity * pEntity) = 0;
   virtual tResult ExecuteCommand(const tChar * pszCommand, const cMultiVar * pArgs, uint nArgs, IEntity * pEntity) = 0;
};

////////////////////////////////////////

ENGINE_API tResult EntityCommandManagerCreate();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityCommandUI
//

interface IEntityCommandUI : IUnknown
{
   virtual tResult SetEntityPanelId(const tChar * pszId) = 0;
   virtual tResult GetEntityPanelId(cStr * pId) = 0;
};

////////////////////////////////////////

ENGINE_API tResult EntityCommandUICreate();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityFactory
//

interface IEntityFactory : IUnknown
{
   virtual tResult CreateEntity(const tChar * pszEntityType, IEntity * * ppEntity) = 0;
};

ENGINE_API tResult EntityFactoryCreate();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityManager
//

interface IEntityManager : IUnknown
{
   virtual tResult AddEntityManagerListener(IEntityManagerListener * pListener) = 0;
   virtual tResult RemoveEntityManagerListener(IEntityManagerListener * pListener) = 0;

   virtual tResult SpawnEntity(const tChar * pszEntity, const tVec3 & position, tEntityId * pEntityId) = 0;
   inline tResult SpawnEntity(const tChar * pszEntity, const tVec3 & position)
   {
      return SpawnEntity(pszEntity, position, NULL);
   }

   virtual tResult RemoveEntity(tEntityId entityId) = 0;
   virtual tResult RemoveEntity(IEntity * pEntity) = 0;
   virtual void RemoveAll() = 0;

   virtual void RenderAll() = 0;

   virtual tResult RayCast(const cRay & ray, IEntity * * ppEntity) const = 0;
   virtual tResult BoxCast(const tAxisAlignedBox & box, IEnumEntities * * ppEnum) const = 0;

   virtual tResult Select(IEntity * pEntity) = 0;
   virtual tResult SelectBoxed(const tAxisAlignedBox & box) = 0;
   virtual tResult DeselectAll() = 0;
   virtual uint GetSelectedCount() const = 0;

   virtual tResult SetSelected(IEnumEntities * pEnum) = 0;
   virtual tResult GetSelected(IEnumEntities * * ppEnum) const = 0;
};

////////////////////////////////////////

ENGINE_API tResult EntityManagerCreate();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityManagerListener
//

interface IEntityManagerListener : IUnknown
{
   virtual void OnEntitySelectionChange() = 0;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYAPI_H
