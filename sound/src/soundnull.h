///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SOUNDNULL_H
#define INCLUDED_SOUNDNULL_H

#include "sound/soundapi.h"

#include "tech/globalobjdef.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cNullSoundManager
//

class cNullSoundManager : public cComObject2<IMPLEMENTS(ISoundManager), IMPLEMENTS(IGlobalObject)>
{
public:
   cNullSoundManager();
   ~cNullSoundManager();

   DECLARE_NAME(NullSoundManager)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult Open(IReader * pReader, tSoundId * pSoundId);
   virtual tResult Close(tSoundId soundId);

   virtual tResult Play(tSoundId soundId);
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SOUNDNULL_H
