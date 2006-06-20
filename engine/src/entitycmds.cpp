///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entityapi.h"
#include "cameraapi.h"
#include "terrainapi.h"

#include "inputapi.h"

#include "globalobj.h"
#include "keys.h"
#include "multivar.h"
#include "ray.h"

#include "dbgalloc.h" // must be last header


/////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(EntityCmds);

#define LocalMsg(msg)            DebugMsgEx(EntityCmds,(msg))
#define LocalMsg1(msg,a)         DebugMsgEx1(EntityCmds,(msg),(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(EntityCmds,(msg),(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(EntityCmds,(msg),(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(EntityCmds,(msg),(a),(b),(c),(d))


///////////////////////////////////////////////////////////////////////////////

tResult EntityCommandSpawn(IEntity * pEntity, const cMultiVar * pArgs, uint nArgs)
{
   if (pArgs == NULL)
   {
      return E_POINTER;
   }

   if (nArgs != 1 || !pArgs[0].IsString())
   {
      return E_INVALIDARG;
   }

   cAutoIPtr<IEntitySpawnComponent> pSpawnComponent;
   if (pEntity->GetComponent(kECT_Spawn, IID_IEntitySpawnComponent, &pSpawnComponent) == S_OK)
   {
      tVec3 rallyPoint;
      if (pSpawnComponent->GetRallyPoint(&rallyPoint) == S_OK)
      {
         UseGlobal(EntityManager);
         pEntityManager->SpawnEntity(pArgs[0].ToString(), rallyPoint);
      }
   }

   WarnMsgIf(!pSpawnComponent, "Attempting to spawn from entity with no spawn component\n");

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

class cSetRallyPointMode : public cComObject<IMPLEMENTS(IInputModalListener)>
{
public:
   cSetRallyPointMode(IEntity * pEntity);
   ~cSetRallyPointMode();

   // TODO: Over-ride QI to support IInputListener too

   virtual bool OnInputEvent(const sInputEvent * pEvent);
   virtual void CancelMode();

private:
   cAutoIPtr<IEntity> m_pEntity;
};

////////////////////////////////////////

cSetRallyPointMode::cSetRallyPointMode(IEntity * pEntity)
 : m_pEntity(CTAddRef(pEntity))
{
}

////////////////////////////////////////

cSetRallyPointMode::~cSetRallyPointMode()
{
}

////////////////////////////////////////

static bool GetTerrainLocation(const cRay & ray, tVec3 * pLocation)
{
   HTERRAINQUAD hQuad;
   UseGlobal(TerrainModel);
   if (pTerrainModel->GetQuadFromHitTest(ray, &hQuad) == S_OK)
   {
      tVec3 corners[4];
      if (pTerrainModel->GetQuadCorners(hQuad, corners) == S_OK)
      {
         if (ray.IntersectsTriangle(corners[0], corners[3], corners[2], pLocation)
            || ray.IntersectsTriangle(corners[2], corners[1], corners[0], pLocation))
         {
            return true;
         }
      }
   }
   return false;
}

bool cSetRallyPointMode::OnInputEvent(const sInputEvent * pEvent)
{
   Assert(pEvent != NULL);

   bool bResult = false;

   if (pEvent->key == kMouseLeft && pEvent->down)
   {
      float ndx, ndy;
      ScreenToNormalizedDeviceCoords(pEvent->point.x, pEvent->point.y, &ndx, &ndy);

      cRay pickRay;
      UseGlobal(Camera);
      if (pCamera->GeneratePickRay(ndx, ndy, &pickRay) == S_OK)
      {
         tVec3 location;
         if (GetTerrainLocation(pickRay, &location))
         {
            UseGlobal(Input);
            pInput->PopModalListener();

            Assert(!!m_pEntity);

            LocalMsg3("Set rally point to (%f, %f, %f)\n", location.x, location.y, location.z);

            cAutoIPtr<IEntitySpawnComponent> pSpawnComponent;
            if (m_pEntity->GetComponent(kECT_Spawn, IID_IEntitySpawnComponent, &pSpawnComponent) == S_OK)
            {
               pSpawnComponent->SetRallyPoint(location);
            }

            WarnMsgIf(!pSpawnComponent, "Attempting to set rally point for entity with no spawn component\n");

            bResult = true;
         }
      }
   }

   return bResult;
}

////////////////////////////////////////

void cSetRallyPointMode::CancelMode()
{
   LocalMsg("SetRallyPoint mode cancelled\n");
}

///////////////////////////////////////////////////////////////////////////////

tResult EntityCommandSetRallyPoint(IEntity * pEntity, const cMultiVar * pArgs, uint nArgs)
{
   cAutoIPtr<IInputModalListener> pMode(new cSetRallyPointMode(pEntity));
   if (!pMode)
   {
      return E_OUTOFMEMORY;
   }
   UseGlobal(Input);
   pInput->PushModalListener(pMode);
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

void RegisterBuiltinEntityCommands()
{
   UseGlobal(EntityCommandManager);
   pEntityCommandManager->RegisterCommand(_T("Spawn"), EntityCommandSpawn);
   pEntityCommandManager->RegisterCommand(_T("SetRallyPoint"), EntityCommandSetRallyPoint);
}

///////////////////////////////////////////////////////////////////////////////
