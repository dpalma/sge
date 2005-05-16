///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIRENDER_H
#define INCLUDED_GUIRENDER_H

#include "guiapi.h"

#include "globalobj.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIRenderingTools
//

class cGUIRenderingTools : public cGlobalObject<IMPLEMENTS(IGUIRenderingTools)>
{
public:
   cGUIRenderingTools();
   ~cGUIRenderingTools();

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult SetDefaultFont(IRenderFont * pFont);
   virtual tResult GetDefaultFont(IRenderFont * * ppFont);

private:
   cAutoIPtr<IRenderFont> m_pFont;
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_GUIRENDER_H
