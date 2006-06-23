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

template <class T, typename ARG>
class cState
{
   cState(const cState &);
   void operator =(const cState &);

   typedef void (T::*tStateEnterExitFn)();
   typedef void (T::*tStateUpdateFn)(ARG);

public:
   cState();
   cState(tStateEnterExitFn pfnStateEnter, tStateUpdateFn pfnState, tStateEnterExitFn pfnStateExit);
   ~cState();

   void ExecuteStateEnter(T * pT) const;
   void ExecuteState(T * pT, ARG arg) const;
   void ExecuteStateExit(T * pT) const;

private:
   tStateEnterExitFn m_pfnStateEnter;
   tStateUpdateFn m_pfnState;
   tStateEnterExitFn m_pfnStateExit;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cStateMachine
//

template <class T, typename ARG>
class cStateMachine
{
public:
   cStateMachine();
   ~cStateMachine();

   void Update(ARG arg);

protected:
   typedef cState<T, ARG> tState;

   bool IsCurrentState(const tState * pState) const;
   void GotoState(const tState * pNewState);

   bool IsCurrentInitialState() const;
   void GotoInitialState();

   void InitialStateEnter();
   void InitialState(ARG arg);
   void InitialStateExit();

   // Derived classes may over-ride these methods
   // to change the initial state behavior
   void OnInitialStateEnter() {}
   void OnInitialState(ARG) {}
   void OnInitialStateExit() {}

private:
   tState m_initialState;
   const tState * m_pCurrentState;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_STATEMACHINE_H
