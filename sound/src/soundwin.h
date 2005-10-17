///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SOUNDWIN_H
#define INCLUDED_SOUNDWIN_H

#include "soundapi.h"

#include "globalobjdef.h"

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>
#include <mmsystem.h>

#include <map>
#include <vector>

#ifdef _MSC_VER
#pragma once
#endif


#if 0
typedef struct wavehdr_tag WAVEHDR;
typedef struct tWAVEFORMATEX WAVEFORMATEX;
F_DECLARE_HANDLE(HWAVEOUT);
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cWavSound
//

class cWavSound
{
public:
   cWavSound(const WAVEFORMATEX & format, byte * pData, uint dataLength);
   ~cWavSound();

   const WAVEFORMATEX & GetFormat() const { return m_format; }

   bool IsPrepared() const;
   tResult Prepare(HWAVEOUT hWaveOut);
   tResult Unprepare();
   tResult Write();

private:
   WAVEFORMATEX m_format;
   byte * m_pData;
   uint m_dataLength;
   HWAVEOUT m_hWaveOut;
   WAVEHDR * m_pHdr;
};


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

private:
   static void CALLBACK WaveOutCallback(HWAVEOUT hWaveOut, uint msg, uint instance, uint param1, uint param2);

   struct sLessWaveFormat
   {
      bool operator()(const WAVEFORMATEX & lhs, const WAVEFORMATEX & rhs) const;
   };

   typedef std::map<WAVEFORMATEX, HWAVEOUT, sLessWaveFormat> tChannelMap;
   tChannelMap m_channels;

   std::vector<cWavSound *> m_sounds;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SOUNDWIN_H
