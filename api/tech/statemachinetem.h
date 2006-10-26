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

template <class T, typename ARG>
cState<T, ARG>::cState()
 : m_pfnStateEnter(NULL)
 , m_pfnState(NULL)
 , m_pfnStateExit(NULL)
{
}

////////////////////////////////////////

template <class T, typename ARG>
cState<T, ARG>::cState(tStateEnterExitFn pfnStateEnter, tStateUpdateFn pfnState, tStateEnterExitFn pfnStateExit)
 : m_pfnStateEnter(pfnStateEnter)
 , m_pfnState(pfnState)
 , m_pfnStateExit(pfnStateExit)
{
}

////////////////////////////////////////

template <class T, typename ARG>
cState<T, ARG>::~cState()
{
}

////////////////////////////////////////

template <class T, typename ARG>
void cState<T, ARG>::ExecuteStateEnter(T * pT) const
{
	if ((pT != NULL) && (m_pfnStateEnter != NULL))
   {
	   (pT->*m_pfnStateEnter)();
   }
}

////////////////////////////////////////

template <class T, typename ARG>
void cState<T, ARG>::ExecuteState(T * pT, ARG arg) const
{
	if ((pT != NULL) && (m_pfnState != NULL))
   {
	   (pT->*m_pfnState)(arg);
   }
}

////////////////////////////////////////

template <class T, typename ARG>
void cState<T, ARG>::ExecuteStateExit(T * pT) const
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

template <class T, typename ARG>
cStateMachine<T, ARG>::cStateMachine()
 : m_initialState(&cStateMachine<T, ARG>::InitialStateEnter,
                  &cStateMachine<T, ARG>::InitialState,
                  &cStateMachine<T, ARG>::InitialStateExit)
 , m_pCurrentState(&m_initialState)
{
}

////////////////////////////////////////

template <class T, typename ARG>
cStateMachine<T, ARG>::~cStateMachine()
{
}

////////////////////////////////////////

template <class T, typename ARG>
void cStateMachine<T, ARG>::Update(ARG arg)
{
   if (m_pCurrentState != NULL)
   {
      T * pT = static_cast<T*>(this);
      m_pCurrentState->ExecuteState(pT, arg);
   }
}

////////////////////////////////////////

template <class T, typename ARG>
bool cStateMachine<T, ARG>::IsCurrentState(const cState<T, ARG> * pState) const
{
   return (m_pCurrentState == pState);
}

////////////////////////////////////////

template <class T, typename ARG>
void cStateMachine<T, ARG>::GotoState(const cState<T, ARG> * pNewState)
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

template <class T, typename ARG>
bool cStateMachine<T, ARG>::IsCurrentInitialState() const
{
   return IsCurrentState(&m_initialState);
}

////////////////////////////////////////

template <class T, typename ARG>
void cStateMachine<T, ARG>::GotoInitialState()
{
   GotoState(&m_initialState);
}

////////////////////////////////////////

template <class T, typename ARG>
void cStateMachine<T, ARG>::InitialStateEnter()
{
   T * pT = static_cast<T*>(this);
   pT->OnInitialStateEnter();
}

////////////////////////////////////////

template <class T, typename ARG>
void cStateMachine<T, ARG>::InitialState(ARG arg)
{
   T * pT = static_cast<T*>(this);
   pT->OnInitialState(arg);
}

////////////////////////////////////////

template <class T, typename ARG>
void cStateMachine<T, ARG>::InitialStateExit()
{
   T * pT = static_cast<T*>(this);
   pT->OnInitialStateExit();
}


////////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_STATEMACHINETEM_H
