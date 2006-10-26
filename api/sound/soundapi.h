///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SOUNDAPI_H
#define INCLUDED_SOUNDAPI_H

#include "sounddll.h"
#include "tech/comtools.h"

#pragma once

F_DECLARE_INTERFACE(ISoundManager);

F_DECLARE_INTERFACE(IReader);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISoundManager
//

#define kRT_WavSound _T("WavSound")

DECLARE_HANDLE(tSoundId);

interface ISoundManager : IUnknown
{
   virtual tResult Open(IReader * pReader, tSoundId * pSoundId) = 0;
   virtual tResult Close(tSoundId soundId) = 0;

   virtual tResult Play(tSoundId soundId) = 0;
};

///////////////////////////////////////

SOUND_API tResult SoundManagerCreate();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SOUNDAPI_H
