///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SOUNDWIN_H
#define INCLUDED_SOUNDWIN_H

#include "soundapi.h"

#include "globalobjdef.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cWinmmSoundManager
//

class cWinmmSoundManager : public cComObject2<IMPLEMENTS(ISoundManager), IMPLEMENTS(IGlobalObject)>
{
public:
   cWinmmSoundManager();
   ~cWinmmSoundManager();

   DECLARE_NAME(WinmmSoundManager)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult Open(IReader * pReader, tSoundId * pSoundId);
   virtual tResult Close(tSoundId soundId);

   virtual tResult Play(tSoundId soundId);
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SOUNDWIN_H
