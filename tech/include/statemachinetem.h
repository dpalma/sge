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

template <class T>
cState<T>::cState()
 : m_pfnStateEnter(NULL)
 , m_pfnState(NULL)
 , m_pfnStateExit(NULL)
{
}

////////////////////////////////////////

template <class T>
cState<T>::cState(tStateFn pfnStateEnter, tStateFn pfnState, tStateFn pfnStateExit)
 : m_pfnStateEnter(pfnStateEnter)
 , m_pfnState(pfnState)
 , m_pfnStateExit(pfnStateExit)
{
}

////////////////////////////////////////

template <class T>
cState<T>::~cState()
{
}

////////////////////////////////////////

template <class T>
void cState<T>::ExecuteStateEnter(T * pT) const
{
	if ((pT != NULL) && (m_pfnStateEnter != NULL))
   {
	   (pT->*m_pfnStateEnter)();
   }
}

////////////////////////////////////////

template <class T>
void cState<T>::ExecuteState(T * pT) const
{
	if ((pT != NULL) && (m_pfnState != NULL))
   {
	   (pT->*m_pfnState)();
   }
}

////////////////////////////////////////

template <class T>
void cState<T>::ExecuteStateExit(T * pT) const
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

template <typename T>
cStateMachine<T>::cStateMachine()
 : m_initialState(&cStateMachine<T>::InitialStateEnter,
                  &cStateMachine<T>::InitialState,
                  &cStateMachine<T>::InitialStateExit)
 , m_pCurrentState(&m_initialState)
{
}

////////////////////////////////////////

template <typename T>
cStateMachine<T>::~cStateMachine()
{
}

////////////////////////////////////////

template <typename T>
void cStateMachine<T>::ExecuteCurrentState()
{
   if (m_pCurrentState != NULL)
   {
      m_pCurrentState->ExecuteState(pT);
   }
}

////////////////////////////////////////

template <typename T>
bool cStateMachine<T>::IsCurrentState(const cState<T> * pState) const
{
   return (m_pCurrentState == pState);
}

////////////////////////////////////////

template <typename T>
void cStateMachine<T>::GotoState(const cState<T> * pNewState)
{
   T * pT = static_cast<T*>(this);
   if ((pNewState != NULL) && (pNewState != m_pCurrentState))
   {
      m_pCurrentState->ExecuteStateExit(pT);
      m_pCurrentState = pNewState;
      m_pCurrentState->ExecuteStateEnter(pT);
      m_pCurrentState->ExecuteState(pT);
   }
}

////////////////////////////////////////

template <typename T>
bool cStateMachine<T>::IsCurrentInitialState() const
{
   return IsCurrentState(&m_initialState);
}

////////////////////////////////////////

template <typename T>
void cStateMachine<T>::GotoInitialState()
{
   GotoState(&m_initialState);
}

////////////////////////////////////////

template <typename T>
void cStateMachine<T>::InitialStateEnter()
{
   T * pT = static_cast<T*>(this);
   pT->OnInitialStateEnter();
}

////////////////////////////////////////

template <typename T>
void cStateMachine<T>::InitialState()
{
   T * pT = static_cast<T*>(this);
   pT->OnInitialState();
}

////////////////////////////////////////

template <typename T>
void cStateMachine<T>::InitialStateExit()
{
   T * pT = static_cast<T*>(this);
   pT->OnInitialStateExit();
}


////////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_STATEMACHINETEM_H
