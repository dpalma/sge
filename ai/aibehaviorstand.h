////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AIBEHAVIORSTAND_H
#define INCLUDED_AIBEHAVIORSTAND_H

#include "ai/aiagentapi.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIBehaviorStand
//

class cAIBehaviorStand : public cComObject<IMPLEMENTS(IAIBehavior)>
{
public:
   cAIBehaviorStand();
   ~cAIBehaviorStand();

   virtual tResult Update(IAIAgent * pAgent, double elapsedTime);

private:
   bool m_bAnimStarted;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AIBEHAVIORSTAND_H
