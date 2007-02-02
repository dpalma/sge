////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AIAGENTTASKSTAND_H
#define INCLUDED_AIAGENTTASKSTAND_H

#include "ai/aiagentapi.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgentTaskStand
//

class cAIAgentTaskStand : public cComObject<IMPLEMENTS(IAIAgentTask)>
{
public:
   cAIAgentTaskStand();
   ~cAIAgentTaskStand();

   virtual tResult Update(IAIAgent * pAgent, double elapsedTime);

private:
   bool m_bAnimStarted;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AIAGENTTASKSTAND_H
