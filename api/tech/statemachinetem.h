////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_STATEMACHINETEM_H
#define INCLUDED_STATEMACHINETEM_H

#include "dbgalloc.h" // must be last header

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cState
//

////////////////////////////////////////

template <class T, typename UPDATEARG>
cState<T, UPDATEARG>::cState()
 : m_pfnStateEnter(NULL)
 , m_pfnStateExit(NULL)
 , m_pfnStateUpdate(NULL)
{
}

////////////////////////////////////////

template <class T, typename UPDATEARG>
cState<T, UPDATEARG>::cState(tStateEnterExitFn pfnStateEnter,
                             tStateEnterExitFn pfnStateExit,
                             tStateUpdateFn pfnStateUpdate)
 : m_pfnStateEnter(pfnStateEnter)
 , m_pfnStateExit(pfnStateExit)
 , m_pfnStateUpdate(pfnStateUpdate)
{
}

////////////////////////////////////////

template <class T, typename UPDATEARG>
cState<T, UPDATEARG>::~cState()
{
}

////////////////////////////////////////

template <class T, typename UPDATEARG>
void cState<T, UPDATEARG>::ExecuteStateEnter(T * pT) const
{
	if ((pT != NULL) && (m_pfnStateEnter != NULL))
   {
	   (pT->*m_pfnStateEnter)();
   }
}

////////////////////////////////////////

template <class T, typename UPDATEARG>
void cState<T, UPDATEARG>::ExecuteStateExit(T * pT) const
{
	if ((pT != NULL) && (m_pfnStateExit != NULL))
   {
	   (pT->*m_pfnStateExit)();
   }
}

////////////////////////////////////////

template <class T, typename UPDATEARG>
void cState<T, UPDATEARG>::ExecuteStateUpdate(T * pT, UPDATEARG arg) const
{
	if ((pT != NULL) && (m_pfnStateUpdate != NULL))
   {
	   (pT->*m_pfnStateUpdate)(arg);
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cStateMachine
//

////////////////////////////////////////

template <class T, typename UPDATEARG, class STATE>
cStateMachine<T, UPDATEARG, STATE>::cStateMachine()
 : m_initialState(&cStateMachine<T, UPDATEARG, STATE>::InitialStateEnter,
                  &cStateMachine<T, UPDATEARG, STATE>::InitialStateExit,
                  &cStateMachine<T, UPDATEARG, STATE>::InitialStateUpdate)
 , m_pCurrentState(&m_initialState)
{
}

////////////////////////////////////////

template <class T, typename UPDATEARG, class STATE>
cStateMachine<T, UPDATEARG, STATE>::~cStateMachine()
{
}

////////////////////////////////////////

template <class T, typename UPDATEARG, class STATE>
void cStateMachine<T, UPDATEARG, STATE>::Update(UPDATEARG arg)
{
   if (m_pCurrentState != NULL)
   {
      T * pT = static_cast<T*>(this);
      m_pCurrentState->ExecuteStateUpdate(pT, arg);
   }
}

////////////////////////////////////////

template <class T, typename UPDATEARG, class STATE>
bool cStateMachine<T, UPDATEARG, STATE>::IsCurrentState(const tState * pState) const
{
   return (m_pCurrentState == pState);
}

////////////////////////////////////////

template <class T, typename UPDATEARG, class STATE>
void cStateMachine<T, UPDATEARG, STATE>::GotoState(const tState * pNewState)
{
   T * pT = static_cast<T*>(this);
   if ((pNewState != NULL) && (pNewState != m_pCurrentState))
   {
      m_pCurrentState->ExecuteStateExit(pT);
      m_pCurrentState = pNewState;
      m_pCurrentState->ExecuteStateEnter(pT);
   }
}

////////////////////////////////////////

template <class T, typename UPDATEARG, class STATE>
bool cStateMachine<T, UPDATEARG, STATE>::IsCurrentInitialState() const
{
   return IsCurrentState(&m_initialState);
}

////////////////////////////////////////

template <class T, typename UPDATEARG, class STATE>
void cStateMachine<T, UPDATEARG, STATE>::GotoInitialState()
{
   GotoState(&m_initialState);
}

////////////////////////////////////////

template <class T, typename UPDATEARG, class STATE>
void cStateMachine<T, UPDATEARG, STATE>::InitialStateEnter()
{
   T * pT = static_cast<T*>(this);
   pT->OnInitialStateEnter();
}

////////////////////////////////////////

template <class T, typename UPDATEARG, class STATE>
void cStateMachine<T, UPDATEARG, STATE>::InitialStateExit()
{
   T * pT = static_cast<T*>(this);
   pT->OnInitialStateExit();
}

////////////////////////////////////////

template <class T, typename UPDATEARG, class STATE>
void cStateMachine<T, UPDATEARG, STATE>::InitialStateUpdate(UPDATEARG arg)
{
   T * pT = static_cast<T*>(this);
   pT->OnInitialStateUpdate(arg);
}


////////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_STATEMACHINETEM_H
