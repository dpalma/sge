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
 , m_pfnStateUpdate(NULL)
 , m_pfnStateExit(NULL)
{
}

////////////////////////////////////////

template <class T, typename UPDATEARG>
cState<T, UPDATEARG>::cState(tStateEnterExitFn pfnStateEnter, tStateUpdateFn pfnStateUpdate, tStateEnterExitFn pfnStateExit)
 : m_pfnStateEnter(pfnStateEnter)
 , m_pfnStateUpdate(pfnStateUpdate)
 , m_pfnStateExit(pfnStateExit)
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
void cState<T, UPDATEARG>::ExecuteStateUpdate(T * pT, UPDATEARG arg) const
{
	if ((pT != NULL) && (m_pfnStateUpdate != NULL))
   {
	   (pT->*m_pfnStateUpdate)(arg);
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


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cStateMachine
//

////////////////////////////////////////

template <class T, typename UPDATEARG>
cStateMachine<T, UPDATEARG>::cStateMachine()
 : m_initialState(&cStateMachine<T, UPDATEARG>::InitialStateEnter,
                  &cStateMachine<T, UPDATEARG>::InitialStateUpdate,
                  &cStateMachine<T, UPDATEARG>::InitialStateExit)
 , m_pCurrentState(&m_initialState)
{
}

////////////////////////////////////////

template <class T, typename UPDATEARG>
cStateMachine<T, UPDATEARG>::~cStateMachine()
{
}

////////////////////////////////////////

template <class T, typename UPDATEARG>
void cStateMachine<T, UPDATEARG>::Update(UPDATEARG arg)
{
   if (m_pCurrentState != NULL)
   {
      T * pT = static_cast<T*>(this);
      m_pCurrentState->ExecuteStateUpdate(pT, arg);
   }
}

////////////////////////////////////////

template <class T, typename UPDATEARG>
bool cStateMachine<T, UPDATEARG>::IsCurrentState(const cState<T, UPDATEARG> * pState) const
{
   return (m_pCurrentState == pState);
}

////////////////////////////////////////

template <class T, typename UPDATEARG>
void cStateMachine<T, UPDATEARG>::GotoState(const cState<T, UPDATEARG> * pNewState)
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

template <class T, typename UPDATEARG>
bool cStateMachine<T, UPDATEARG>::IsCurrentInitialState() const
{
   return IsCurrentState(&m_initialState);
}

////////////////////////////////////////

template <class T, typename UPDATEARG>
void cStateMachine<T, UPDATEARG>::GotoInitialState()
{
   GotoState(&m_initialState);
}

////////////////////////////////////////

template <class T, typename UPDATEARG>
void cStateMachine<T, UPDATEARG>::InitialStateEnter()
{
   T * pT = static_cast<T*>(this);
   pT->OnInitialStateEnter();
}

////////////////////////////////////////

template <class T, typename UPDATEARG>
void cStateMachine<T, UPDATEARG>::InitialStateUpdate(UPDATEARG arg)
{
   T * pT = static_cast<T*>(this);
   pT->OnInitialStateUpdate(arg);
}

////////////////////////////////////////

template <class T, typename UPDATEARG>
void cStateMachine<T, UPDATEARG>::InitialStateExit()
{
   T * pT = static_cast<T*>(this);
   pT->OnInitialStateExit();
}


////////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_STATEMACHINETEM_H
