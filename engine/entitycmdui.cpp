///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitycmdui.h"

#include "gui/guielementapi.h"

#include "tech/multivar.h"

#include <tinyxml.h>

#include <vector>

#include "tech/dbgalloc.h" // must be last header

using namespace std;

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(EntityCmdUI);

#define LocalMsg(msg)            DebugMsgEx(EntityCmdUI,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(EntityCmdUI,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(EntityCmdUI,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(EntityCmdUI,msg,(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(EntityCmdUI,msg,(a),(b),(c),(d))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(EntityCmdUI,(cond),msg)
#define LocalMsgIf1(cond,msg,a)        DebugMsgIfEx1(EntityCmdUI,(cond),msg,(a))
#define LocalMsgIf2(cond,msg,a,b)      DebugMsgIfEx2(EntityCmdUI,(cond),msg,(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)    DebugMsgIfEx3(EntityCmdUI,(cond),msg,(a),(b),(c))
#define LocalMsgIf4(cond,msg,a,b,c,d)  DebugMsgIfEx4(EntityCmdUI,(cond),msg,(a),(b),(c),(d))

///////////////////////////////////////////////////////////////////////////////

#define ENTITYCMDUICOMPONENTA "commands"
#define ENTITYCMDUICOMPONENTW L"commands"
#ifdef _UNICODE
#define ENTITYCMDUICOMPONENT ENTITYCMDUICOMPONENTW
#else
#define ENTITYCMDUICOMPONENT ENTITYCMDUICOMPONENTA
#endif
static const char g_szElementCommand[] = "command";
static const char g_szElementArgument[] = "argument";
static const char g_szAttribName[] = "name";
static const char g_szAttribImage[] = "image";
static const char g_szAttribToolTip[] = "tooltip";


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityCmdInfo
//

////////////////////////////////////////

cEntityCmdInfo::cEntityCmdInfo()
 : m_cmdInst(NULL)
{
}

////////////////////////////////////////

cEntityCmdInfo::cEntityCmdInfo(const tChar * pszImage, const tChar * pszTooltip, tEntityCmdInstance cmdInst)
 : m_image((pszImage != NULL) ? pszImage : _T(""))
 , m_toolTip((pszTooltip != NULL) ? pszTooltip : _T(""))
 , m_cmdInst(cmdInst)
{
}

////////////////////////////////////////

cEntityCmdInfo::cEntityCmdInfo(const cEntityCmdInfo & other)
 : m_image(other.m_image)
 , m_toolTip(other.m_toolTip)
 , m_cmdInst(other.m_cmdInst)
{
}

////////////////////////////////////////

const cEntityCmdInfo cEntityCmdInfo::operator =(const cEntityCmdInfo & other)
{
   m_image.assign(other.m_image);
   m_toolTip.assign(other.m_toolTip);
   m_cmdInst = other.m_cmdInst;
   return *this;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityCmdUI
//

////////////////////////////////////////

cEntityCmdUI::cEntityCmdUI()
{
}

////////////////////////////////////////

cEntityCmdUI::~cEntityCmdUI()
{
}

////////////////////////////////////////

ulong STDMETHODCALLTYPE cEntityCmdUI::AddRef()
{
   return cNonDelegatingUnknown::DoAddRef();
}

////////////////////////////////////////

ulong STDMETHODCALLTYPE cEntityCmdUI::Release()
{
   return cNonDelegatingUnknown::DoRelease();
}

////////////////////////////////////////

tResult STDMETHODCALLTYPE cEntityCmdUI::QueryInterface(REFGUID iid, void * * ppvObject)
{
   const struct sQIPair pairs[] =
   {
      { static_cast<IEntityCommandUI *>(this),        &IID_IEntityCommandUI },
      { static_cast<IGlobalObject *>(this),           &IID_IGlobalObject },
      { static_cast<IEntityManagerListener *>(this),  &IID_IEntityManagerListener },
      { static_cast<IGUIEventListener *>(this),       &IID_IGUIEventListener },
   };
   return cNonDelegatingUnknown::DoQueryInterface(pairs, _countof(pairs), iid, ppvObject);
}

////////////////////////////////////////

tResult cEntityCmdUI::Init()
{
   UseGlobal(EntityManager);
   pEntityManager->AddEntityManagerListener(static_cast<IEntityManagerListener*>(this));

   UseGlobal(EntityComponentRegistry);
   pEntityComponentRegistry->RegisterComponentFactory(ENTITYCMDUICOMPONENT, static_cast<IEntityComponentFactory*>(this));

   UseGlobal(GUIContext);
   pGUIContext->AddEventListener(static_cast<IGUIEventListener*>(this));

   return S_OK;
}

////////////////////////////////////////

tResult cEntityCmdUI::Term()
{
   SafeRelease(m_pTargetEntity);
   m_activeCmds.clear();

   UseGlobal(GUIContext);
   pGUIContext->RemoveEventListener(static_cast<IGUIEventListener*>(this));

   UseGlobal(EntityManager);
   pEntityManager->RemoveEntityManagerListener(static_cast<IEntityManagerListener*>(this));

   UseGlobal(EntityComponentRegistry);
   pEntityComponentRegistry->RevokeComponentFactory(ENTITYCMDUICOMPONENT);

   return S_OK;
}

////////////////////////////////////////

void cEntityCmdUI::OnEntitySelectionChange()
{
   SafeRelease(m_pTargetEntity);
   m_activeCmds.clear();

   UseGlobal(GUIContext);
   cAutoIPtr<IGUIElement> pPanelElement;
   cAutoIPtr<IGUIContainerElement> pContainer;
   if (pGUIContext->GetElementById(m_entityPanelId.c_str(), &pPanelElement) == S_OK)
   {
      if (pPanelElement->QueryInterface(IID_IGUIContainerElement, (void**)&pContainer) == S_OK)
      {
         pContainer->RemoveAll();
      }
   }

   if (!pContainer)
   {
      WarnMsg1("Unable to find entity command panel element with id %s\n", m_entityPanelId.c_str());
      return;
   }

   UseGlobal(EntityManager);
   if (pEntityManager->GetSelectedCount() == 1)
   {
      cAutoIPtr<IEnumEntities> pEnum;
      if (pEntityManager->GetSelected(&pEnum) == S_OK)
      {
         cAutoIPtr<IEntity> pEntity;
         ulong nEntities = 0;
         if (pEnum->Next(1, &pEntity, &nEntities) == S_OK && nEntities == 1)
         {
            cStr typeName;
            if (pEntity->GetTypeName(&typeName) == S_OK)
            {
               m_pTargetEntity = pEntity; // No AddRef--assigning smart pointers

               uint index = 0;
               tEntityTypeCmdMap::iterator first = m_entityTypeCmdMap.lower_bound(typeName);
               tEntityTypeCmdMap::iterator last = m_entityTypeCmdMap.upper_bound(typeName);
               for (tEntityTypeCmdMap::iterator iter = first; iter != last; iter++, index++)
               {
                  const cEntityCmdInfo & cmdInfo = iter->second;

                  cAutoIPtr<IGUIButtonElement> pButton;
                  if (GUIButtonCreate(&pButton) == S_OK)
                  {
                     cStr id;
                     pButton->SetId(Sprintf(&id, _T("%sCmd%d"), typeName.c_str(), index).c_str());
                     m_activeCmds[id] = cmdInfo.GetCmdInstance();
                     Verify(pButton->SetText(cmdInfo.GetToolTip()) == S_OK);
                     Verify(pContainer->AddElement(pButton) == S_OK);
                  }
               }
            }
         }
      }
   }

   pGUIContext->RequestLayout(pContainer);
}

////////////////////////////////////////

tResult cEntityCmdUI::OnEvent(IGUIEvent * pEvent)
{
   if (pEvent == NULL)
   {
      return E_POINTER;
   }

   if (!!m_pTargetEntity && !m_activeCmds.empty())
   {
      tGUIEventCode eventCode;
      if (pEvent->GetEventCode(&eventCode) == S_OK
         && eventCode == kGUIEventClick)
      {
         cAutoIPtr<IGUIElement> pClicked;
         if (pEvent->GetSourceElement(&pClicked) == S_OK)
         {
            cStr id;
            if (pClicked->GetId(&id) == S_OK)
            {
               tActiveCmdMap::iterator f = m_activeCmds.find(id);
               if (f != m_activeCmds.end())
               {
                  UseGlobal(EntityCommandManager);
                  pEntityCommandManager->ExecuteCommand(f->second, m_pTargetEntity);
               }
            }
         }
      }
   }

   return S_OK;
}

////////////////////////////////////////

tResult cEntityCmdUI::SetEntityPanelId(const tChar * pszId)
{
   if (pszId == NULL)
   {
      m_entityPanelId.clear();
   }
   else
   {
      m_entityPanelId.assign(pszId);
   }
   return S_OK;
}

////////////////////////////////////////

tResult cEntityCmdUI::GetEntityPanelId(cStr * pId)
{
   if (pId == NULL)
   {
      return E_POINTER;
   }
   if (m_entityPanelId.empty())
   {
      return S_FALSE;
   }
   pId->assign(m_entityPanelId);
   return S_OK;
}

////////////////////////////////////////

static tResult EntityCmdParseArgs(const TiXmlElement * pElement, vector<cMultiVar> * pArgs)
{
   if (pElement == NULL || pArgs == NULL)
   {
      return E_POINTER;
   }

   uint count = 0;

   for (const TiXmlElement * pChild = pElement->FirstChildElement();
      pChild != NULL; pChild = pChild->NextSiblingElement())
   {
      const char * pszValue = pChild->Attribute("value");
      if (_stricmp(pChild->Value(), g_szElementArgument) == 0 && pszValue != NULL)
      {
         pArgs->push_back(cMultiVar(pszValue));
         count++;
      }
   }

   return (count > 0) ? S_OK : S_FALSE;
}

////////////////////////////////////////

tResult cEntityCmdUI::CreateComponent(const TiXmlElement * pTiXmlElement,
                                      IEntity * pEntity,
                                      IEntityComponent * * ppComponent)
{
   if (pTiXmlElement == NULL || pEntity == NULL)
   {
      return E_POINTER;
   }

   if (_stricmp(pTiXmlElement->Value(), ENTITYCMDUICOMPONENTA) != 0)
   {
      return E_INVALIDARG;
   }

   cStr typeName;
   if (pEntity->GetTypeName(&typeName) != S_OK)
   {
      return E_FAIL;
   }

   tEntityTypeCmdMap::iterator f = m_entityTypeCmdMap.find(typeName);
   if (f != m_entityTypeCmdMap.end())
   {
      // Commands for this type already registered
      return S_FALSE;
   }

   UseGlobal(EntityCommandManager);

   for (const TiXmlElement * pTiXmlChild = pTiXmlElement->FirstChildElement();
      pTiXmlChild != NULL; pTiXmlChild = pTiXmlChild->NextSiblingElement())
   {
      Assert(pTiXmlChild->Type() == TiXmlNode::ELEMENT);

      if ((_stricmp(pTiXmlChild->Value(), g_szElementCommand) == 0)
         && (pTiXmlChild->Attribute(g_szAttribName) != NULL))
      {
         cMultiVar cmdName(pTiXmlChild->Attribute(g_szAttribName));
         cMultiVar cmdImage(pTiXmlChild->Attribute(g_szAttribImage));
         cMultiVar cmdToolTip(pTiXmlChild->Attribute(g_szAttribToolTip));

         vector<cMultiVar> args;
         if (SUCCEEDED(EntityCmdParseArgs(pTiXmlChild, &args)))
         {
            LocalMsg3("Entity type %s supports command %s (%d args)\n", typeName.c_str(), cmdName.ToString(), args.size());

            tEntityCmdInstance cmdInst = NULL;
            if (pEntityCommandManager->CompileCommand(cmdName, args.empty() ? NULL : &args[0], args.size(), &cmdInst) == S_OK)
            {
               m_entityTypeCmdMap.insert(make_pair(typeName, cEntityCmdInfo(cmdImage, cmdToolTip, cmdInst)));
            }
         }
      }
   }

   // This function doesn't actually return a component
   return S_FALSE;
}

////////////////////////////////////////

tResult EntityCommandUICreate()
{
   cAutoIPtr<IEntityCommandUI> p(static_cast<IEntityCommandUI*>(new cEntityCmdUI));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IEntityCommandUI, p);
}

///////////////////////////////////////////////////////////////////////////////
