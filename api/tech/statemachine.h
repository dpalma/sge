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

template <class T, typename UPDATEARG>
class cState
{
   cState(const cState &);
   const cState & operator =(const cState &);

   typedef void (T::*tStateEnterExitFn)();
   typedef void (T::*tStateUpdateFn)(UPDATEARG);

public:
   cState();
   cState(tStateEnterExitFn pfnStateEnter, tStateEnterExitFn pfnStateExit, tStateUpdateFn pfnStateUpdate);
   ~cState();

   void ExecuteStateEnter(T * pT) const;
   void ExecuteStateExit(T * pT) const;
   void ExecuteStateUpdate(T * pT, UPDATEARG arg) const;

private:
   tStateEnterExitFn m_pfnStateEnter;
   tStateEnterExitFn m_pfnStateExit;
   tStateUpdateFn m_pfnStateUpdate;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cStateMachine
//

template <class T, typename UPDATEARG, class STATE = cState<T, UPDATEARG> >
class cStateMachine
{
public:
   cStateMachine();
   ~cStateMachine();

   void Update(UPDATEARG arg);

protected:
   typedef STATE tState;

   bool IsCurrentState(const tState * pState) const;
   void GotoState(const tState * pNewState);

   bool IsCurrentInitialState() const;
   void GotoInitialState();

   void InitialStateEnter();
   void InitialStateExit();
   void InitialStateUpdate(UPDATEARG arg);

   // Derived classes may over-ride these methods
   // to change the initial state behavior
   void OnInitialStateEnter() {}
   void OnInitialStateExit() {}
   void OnInitialStateUpdate(UPDATEARG) {}

private:
   tState m_initialState;
   const tState * m_pCurrentState;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_STATEMACHINE_H
