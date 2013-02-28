///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitycmdmanager.h"

#include "tech/matrix4.inl"
#include "tech/multivar.h"

#include <tinyxml.h>

#include "tech/dbgalloc.h" // must be last header


extern void RegisterBuiltinEntityCommands();

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityCmdInstance
//

////////////////////////////////////////

cEntityCmdInstance::cEntityCmdInstance()
 : m_pfn(NULL)
{
}

////////////////////////////////////////

cEntityCmdInstance::cEntityCmdInstance(tEntityCommandFn pfn, const cMultiVar * pArgs, uint nArgs)
 : m_pfn(pfn)
{
   if (pArgs != NULL && nArgs > 0)
   {
      m_args.resize(nArgs);
      for (uint i = 0; i < nArgs; i++)
      {
         m_args[i] = pArgs[i];
      }
   }
}

////////////////////////////////////////

cEntityCmdInstance::cEntityCmdInstance(const cEntityCmdInstance & other)
 : m_pfn(other.m_pfn)
 , m_args(other.m_args.size())
{
   std::copy(other.m_args.begin(), other.m_args.end(), m_args.begin());
}

////////////////////////////////////////

const cEntityCmdInstance cEntityCmdInstance::operator =(const cEntityCmdInstance & other)
{
   m_pfn = other.m_pfn;
   m_args.resize(other.m_args.size());
   std::copy(other.m_args.begin(), other.m_args.end(), m_args.begin());
   return *this;
}

////////////////////////////////////////

tResult cEntityCmdInstance::Invoke(IEntity * pEntity)
{
   return (m_pfn != NULL) ? (*m_pfn)(pEntity, m_args.empty() ? NULL : &m_args[0], m_args.size()) : E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityCmdManager
//

////////////////////////////////////////

cEntityCmdManager::cEntityCmdManager()
 : m_cmdInstHandleBase(0)
{
   m_cmdInstHandleBase = reinterpret_cast<uint_ptr>(this);
}

////////////////////////////////////////

cEntityCmdManager::~cEntityCmdManager()
{
}

////////////////////////////////////////

tResult cEntityCmdManager::Init()
{
   RegisterBuiltinEntityCommands();
   return S_OK;
}

////////////////////////////////////////

tResult cEntityCmdManager::Term()
{
   return S_OK;
}

////////////////////////////////////////

tResult cEntityCmdManager::RegisterCommand(const tChar * pszCommand, tEntityCommandFn pfnCommand)
{
   if (pszCommand == NULL || pfnCommand == NULL)
   {
      return E_POINTER;
   }

   tEntityCommandMap::iterator f = m_entityCommandMap.find(pszCommand);
   if (f != m_entityCommandMap.end())
   {
      return S_FALSE;
   }

   m_entityCommandMap.insert(std::make_pair(pszCommand, pfnCommand));
   return S_OK;
}

////////////////////////////////////////

tResult cEntityCmdManager::RevokeCommand(const tChar * pszCommand)
{
   if (pszCommand == NULL)
   {
      return E_POINTER;
   }

   size_t nErased = m_entityCommandMap.erase(pszCommand);

   return (nErased > 0) ? S_OK : S_FALSE;
}

////////////////////////////////////////

tResult cEntityCmdManager::CompileCommand(const tChar * pszCommand, const cMultiVar * pArgs, uint nArgs, tEntityCmdInstance * pCmdInst)
{
   if (pszCommand == NULL || pCmdInst == NULL)
   {
      return E_POINTER;
   }

   tEntityCommandMap::iterator f = m_entityCommandMap.find(pszCommand);
   if (f == m_entityCommandMap.end())
   {
      return E_FAIL;
   }

   m_cmdInsts.push_back(cEntityCmdInstance(f->second, pArgs, nArgs));
   *pCmdInst = CmdInstFromIndex(m_cmdInsts.size() - 1);
   return S_OK;
}

////////////////////////////////////////

tResult cEntityCmdManager::ExecuteCommand(tEntityCmdInstance cmdInst, IEntity * pEntity)
{
   if (cmdInst == NULL)
   {
      return E_POINTER;
   }

   uint_ptr index = IndexFromCmdInst(cmdInst);
   if (index >= m_cmdInsts.size())
   {
      return E_INVALIDARG;
   }

   return m_cmdInsts[index].Invoke(pEntity);
}

////////////////////////////////////////

tResult cEntityCmdManager::ExecuteCommand(const tChar * pszCommand, const cMultiVar * pArgs, uint nArgs, IEntity * pEntity)
{
   if (pszCommand == NULL)
   {
      return E_POINTER;
   }

   tEntityCommandMap::iterator f = m_entityCommandMap.find(pszCommand);
   if (f == m_entityCommandMap.end())
   {
      return E_FAIL;
   }

   return (*(f->second))(pEntity, pArgs, nArgs);
}

////////////////////////////////////////

tResult EntityCommandManagerCreate()
{
   cAutoIPtr<IEntityCommandManager> p(static_cast<IEntityCommandManager*>(new cEntityCmdManager));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IEntityCommandManager, p);
}

///////////////////////////////////////////////////////////////////////////////
