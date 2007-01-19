///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aiagent.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgent
//

////////////////////////////////////////

cAIAgent::cAIAgent()
{
}

////////////////////////////////////////

cAIAgent::~cAIAgent()
{
   tAIBehaviorList::iterator iter = m_behaviorStack.begin(), end = m_behaviorStack.end();
   for (; iter != end; ++iter)
   {
      (*iter)->Release();
   }
   m_behaviorStack.clear();
}

////////////////////////////////////////

tResult cAIAgent::SetDefaultBehavior(IAIBehavior * pBehavior)
{
   SafeRelease(m_pDefaultBehavior);
   m_pDefaultBehavior = CTAddRef(pBehavior);
   return S_OK;
}

////////////////////////////////////////

tResult cAIAgent::GetDefaultBehavior(IAIBehavior * * ppBehavior)
{
   return m_pDefaultBehavior.GetPointer(ppBehavior);
}

////////////////////////////////////////

tResult cAIAgent::PushBehavior(IAIBehavior * pBehavior)
{
   if (pBehavior == NULL)
   {
      return E_POINTER;
   }
   m_behaviorStack.push_front(CTAddRef(pBehavior));
   return S_OK;
}

////////////////////////////////////////

tResult cAIAgent::PopBehavior()
{
   if (m_behaviorStack.empty())
   {
      return E_FAIL;
   }
   IAIBehavior * pBehavior = m_behaviorStack.front();
   m_behaviorStack.pop_front();
   SafeRelease(pBehavior);
   return S_OK;
}

////////////////////////////////////////

tResult cAIAgent::GetActiveBehavior(IAIBehavior * * ppBehavior)
{
   if (m_behaviorStack.empty())
   {
      return GetDefaultBehavior(ppBehavior);
   }
   else
   {
      IAIBehavior * pBehavior = m_behaviorStack.front();
      if (pBehavior == NULL)
      {
         return E_FAIL; // but, this shouldn't ever happen
      }
      *ppBehavior = CTAddRef(pBehavior);
      return S_OK;
   }
}

////////////////////////////////////////

tResult cAIAgent::SetLocationProvider(IAIAgentLocationProvider * pLocationProvider)
{
   SafeRelease(m_pLocationProvider);
   m_pLocationProvider = CTAddRef(pLocationProvider);
   return S_OK;
}

////////////////////////////////////////

tResult cAIAgent::GetLocationProvider(IAIAgentLocationProvider * * ppLocationProvider)
{
   return m_pLocationProvider.GetPointer(ppLocationProvider);
}

////////////////////////////////////////

tResult cAIAgent::SetAnimationProvider(IAIAgentAnimationProvider * pAnimationProvider)
{
   SafeRelease(m_pAnimationProvider);
   m_pAnimationProvider = CTAddRef(pAnimationProvider);
   return S_OK;
}

////////////////////////////////////////

tResult cAIAgent::GetAnimationProvider(IAIAgentAnimationProvider * * ppAnimationProvider)
{
   return m_pAnimationProvider.GetPointer(ppAnimationProvider);
}

////////////////////////////////////////

tResult AIAgentCreate(IAIAgent * * ppAgent)
{
   if (ppAgent == NULL)
   {
      return E_POINTER;
   }
   cAutoIPtr<IAIAgent> pAgent(static_cast<IAIAgent *>(new cAIAgent));
   if (!pAgent)
   {
      return E_OUTOFMEMORY;
   }
   return pAgent.GetPointer(ppAgent);
}


///////////////////////////////////////////////////////////////////////////////
