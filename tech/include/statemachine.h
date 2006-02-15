///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_STATEMACHINE_H
#define INCLUDED_STATEMACHINE_H

#include "techdll.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cState
//

template <class T>
class cState
{
   typedef void (T::*tStateFn)(void);

   cState(const cState &);
   void operator =(const cState &);

public:
   cState();
   cState(tStateFn pfnStateEnter, tStateFn pfnState, tStateFn pfnStateExit);
   ~cState();

   void ExecuteStateEnter(T * pT) const;
   void ExecuteState(T * pT) const;
   void ExecuteStateExit(T * pT) const;

private:
   tStateFn m_pfnStateEnter, m_pfnState, m_pfnStateExit;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cStateMachine
//

template <typename T>
class cStateMachine
{
public:
   cStateMachine();
   ~cStateMachine();

   void ExecuteCurrentState();

protected:
   bool IsCurrentState(const cState<T> * pState) const;
   void GotoState(const cState<T> * pNewState);

   bool IsCurrentInitialState() const;
   void GotoInitialState();

   void InitialStateEnter();
   void InitialState();
   void InitialStateExit();

   // Derived classes may over-ride these methods
   // to change the initial state behavior
   void OnInitialStateEnter() {}
   void OnInitialState() {}
   void OnInitialStateExit() {}

private:
   cState<T> m_initialState;
   const cState<T> * m_pCurrentState;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_STATEMACHINE_H
