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
   typedef cStateMachine<T, UPDATEARG, STATE> tStateMachine;
   typedef STATE tState;

   cStateMachine(const tState * pInitialState);
   ~cStateMachine();

   void Update(UPDATEARG arg);

protected:
   bool IsCurrentState(const tState * pState) const;
   void GotoState(const tState * pNewState);

private:
   const tState * m_pCurrentState;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_STATEMACHINE_H
