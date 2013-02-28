////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYCMDMANAGER_H
#define INCLUDED_ENTITYCMDMANAGER_H

#include "engine/entityapi.h"

#include "tech/globalobjdef.h"

#include <map>
#include <vector>

#ifdef _MSC_VER
#pragma once
#endif


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityCmdInstance
//

class cEntityCmdInstance
{
public:
   cEntityCmdInstance();
   cEntityCmdInstance(tEntityCommandFn pfn, const cMultiVar * pArgs, uint nArgs);
   cEntityCmdInstance(const cEntityCmdInstance & other);

   const cEntityCmdInstance operator =(const cEntityCmdInstance & other);

   inline tResult operator ()(IEntity * pEntity)
   {
      return Invoke(pEntity);
   }

   tResult Invoke(IEntity * pEntity);

private:
   tEntityCommandFn m_pfn;
   std::vector<cMultiVar> m_args;
};


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityCmdManager
//

class cEntityCmdManager : public cComObject2<IMPLEMENTS(IEntityCommandManager), IMPLEMENTS(IGlobalObject)>
{
public:
   cEntityCmdManager();
   ~cEntityCmdManager();

   DECLARE_NAME(EntityCmdManager)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult RegisterCommand(const tChar * pszCommand, tEntityCommandFn pfnCommand);
   virtual tResult RevokeCommand(const tChar * pszCommand);

   virtual tResult CompileCommand(const tChar * pszCommand, const cMultiVar * pArgs, uint nArgs, tEntityCmdInstance * pCmdInst);
   virtual tResult ExecuteCommand(tEntityCmdInstance cmdInst, IEntity * pEntity);
   virtual tResult ExecuteCommand(const tChar * pszCommand, const cMultiVar * pArgs, uint nArgs, IEntity * pEntity);

private:
   typedef std::map<cStr, tEntityCommandFn> tEntityCommandMap;
   tEntityCommandMap m_entityCommandMap;

   inline tEntityCmdInstance CmdInstFromIndex(uint_ptr index)
   {
      return reinterpret_cast<tEntityCmdInstance>(index ^ m_cmdInstHandleBase);
   }

   inline uint_ptr IndexFromCmdInst(tEntityCmdInstance cmdInst)
   {
      return reinterpret_cast<uint_ptr>(cmdInst) ^ m_cmdInstHandleBase;
   }

   uint_ptr m_cmdInstHandleBase;

   typedef std::vector<cEntityCmdInstance> tEntityCmdInstances;
   tEntityCmdInstances m_cmdInsts;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYCMDMANAGER_H
