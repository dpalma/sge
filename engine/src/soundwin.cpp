///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "soundwin.h"

#include "resourceapi.h"
#include "readwriteapi.h"

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cWavSound
//

///////////////////////////////////////

cWavSound::cWavSound(const WAVEFORMATEX & format, byte * pData, uint dataLength)
 : m_pData(pData)
 , m_dataLength(dataLength)
 , m_hWaveOut(NULL)
 , m_pHdr(NULL)
{
   memcpy(&m_format, &format, sizeof(WAVEFORMATEX));
}

///////////////////////////////////////

cWavSound::~cWavSound()
{
   Unprepare();
   delete [] m_pData;
   delete m_pHdr;
}

///////////////////////////////////////

bool cWavSound::IsPrepared() const
{
   return (m_hWaveOut != NULL);
}

///////////////////////////////////////

tResult cWavSound::Prepare(HWAVEOUT hWaveOut)
{
   if (m_pHdr == NULL)
   {
      m_pHdr = new WAVEHDR;
      if (m_pHdr == NULL)
      {
         return E_OUTOFMEMORY;
      }
   }

   ZeroMemory(m_pHdr, sizeof(WAVEHDR));
   m_pHdr->dwBufferLength = m_dataLength;
   m_pHdr->lpData = reinterpret_cast<LPSTR>(m_pData);

   if (waveOutPrepareHeader(hWaveOut, m_pHdr, sizeof(WAVEHDR)) == MMSYSERR_NOERROR)
   {
      m_hWaveOut = hWaveOut;
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cWavSound::Unprepare()
{
   if (m_hWaveOut != NULL && m_pHdr != NULL)
   {
      if (waveOutUnprepareHeader(m_hWaveOut, m_pHdr, sizeof(WAVEHDR)) == MMSYSERR_NOERROR)
      {
         m_hWaveOut = NULL;
         return S_OK;
      }
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cWavSound::Write()
{
   if (m_hWaveOut != NULL && m_pHdr != NULL)
   {
      if (waveOutWrite(m_hWaveOut, m_pHdr, sizeof(WAVEHDR)) == MMSYSERR_NOERROR)
      {
         return S_OK;
      }
   }
   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cWinmmSoundManager
//

///////////////////////////////////////

cWinmmSoundManager::cWinmmSoundManager()
{
}

///////////////////////////////////////

cWinmmSoundManager::~cWinmmSoundManager()
{
}

///////////////////////////////////////

tResult cWinmmSoundManager::Init()
{
   uint nDevices = waveOutGetNumDevs();
   if (nDevices == 0)
   {
      InfoMsg("No sound devices found on the system\n");
      return S_FALSE;
   }

   return S_OK;
}

///////////////////////////////////////

tResult cWinmmSoundManager::Term()
{
   {
      std::vector<cWavSound *>::iterator iter = m_sounds.begin();
      for (; iter != m_sounds.end(); iter++)
      {
         (*iter)->Unprepare();
         delete (*iter);
      }
      m_sounds.clear();
   }

   {
      tChannelMap::iterator iter = m_channels.begin();
      for (; iter != m_channels.end(); iter++)
      {
         waveOutClose(iter->second);
      }
      m_channels.clear();
   }

   return S_OK;
}

///////////////////////////////////////

tResult cWinmmSoundManager::Open(IReader * pReader, tSoundId * pSoundId)
{
   if (pReader == NULL || pSoundId == NULL)
   {
      return E_POINTER;
   }

   tResult result = E_FAIL;

   ulong length;
   if (pReader->Seek(0, kSO_End) == S_OK
      && pReader->Tell(&length) == S_OK
      && pReader->Seek(0, kSO_Set) == S_OK)
   {
      byte * pBuffer = new byte[length];
      if (pBuffer == NULL)
      {
         return E_OUTOFMEMORY;
      }

      if (pReader->Read(pBuffer, length) == S_OK)
      {
         MMIOINFO mmioInfo = {0};
         mmioInfo.pchBuffer = reinterpret_cast<HPSTR>(pBuffer);
         mmioInfo.cchBuffer = length;
         mmioInfo.fccIOProc = FOURCC_MEM;
         HMMIO hMmio = mmioOpen(NULL, &mmioInfo, MMIO_READ);
         if (hMmio != NULL)
         {
            MMCKINFO mmParentChunk = {0};
			   mmParentChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
            MMRESULT mmResult = mmioDescend(hMmio, &mmParentChunk, NULL, MMIO_FINDRIFF);
			   if (mmResult == MMSYSERR_NOERROR) 
			   {
               MMCKINFO mmSubChunk = {0};
			      mmSubChunk.ckid = mmioFOURCC('f', 'm', 't', ' '); 
			      if (mmioDescend(hMmio, &mmSubChunk, &mmParentChunk, MMIO_FINDCHUNK) == MMSYSERR_NOERROR) 
               {
                  WAVEFORMATEX waveFormat;
      			   if (mmioRead(hMmio, reinterpret_cast<HPSTR>(&waveFormat), mmSubChunk.cksize) == mmSubChunk.cksize)
                  {
                     // Ascend out of the "fmt" chunk
         			   mmioAscend(hMmio, &mmSubChunk, 0); 

			            mmSubChunk.ckid = mmioFOURCC('d', 'a', 't', 'a'); 
			            if (mmioDescend(hMmio, &mmSubChunk, &mmParentChunk, MMIO_FINDCHUNK) == MMSYSERR_NOERROR)
                     {
                        uint soundLength = mmSubChunk.cksize;
                        byte * pSound = new byte[soundLength];
                        if (pSound == NULL)
                        {
                           result = E_OUTOFMEMORY;
                        }
                        else
                        {
                           if (mmioRead(hMmio, reinterpret_cast<HPSTR>(pSound), soundLength) == soundLength)
                           {
                              cWavSound * pWavSound = new cWavSound(waveFormat, pSound, soundLength);
                              if (pWavSound == NULL)
                              {
                                 delete [] pSound, pSound = NULL;
                                 result = E_OUTOFMEMORY;
                              }
                              else
                              {
                                 m_sounds.push_back(pWavSound);
                                 *pSoundId = reinterpret_cast<tSoundId>(pWavSound);
                                 result = S_OK;
                              }
                           }
                           else
                           {
                              delete [] pSound, pSound = NULL;
                           }
                        }
                     }
                  }
               }
            }

            mmioClose(hMmio, 0);
         }
      }

      delete [] pBuffer, pBuffer = NULL;
   }

   return result;
}

///////////////////////////////////////

tResult cWinmmSoundManager::Close(tSoundId soundId)
{
   cWavSound * pWavSound = reinterpret_cast<cWavSound*>(soundId);
   if (pWavSound == NULL)
   {
      return E_INVALIDARG;
   }

   std::vector<cWavSound*>::iterator iter = m_sounds.begin();
   for (; iter != m_sounds.end(); iter++)
   {
      if (pWavSound == *iter)
      {
         m_sounds.erase(iter);
         pWavSound->Unprepare();
         delete pWavSound;
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cWinmmSoundManager::Play(tSoundId soundId)
{
   cWavSound * pWavSound = reinterpret_cast<cWavSound*>(soundId);
   if (pWavSound == NULL)
   {
      return E_INVALIDARG;
   }

   if (!pWavSound->IsPrepared())
   {
      HWAVEOUT hWaveOut = NULL;

      {
         tChannelMap::iterator f = m_channels.find(pWavSound->GetFormat());
         if (f != m_channels.end())
         {
            hWaveOut = f->second;
         }
         else
         {
            const WAVEFORMATEX & format = pWavSound->GetFormat();
            if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &format,
               reinterpret_cast<uint_ptr>(WaveOutCallback),
               0, CALLBACK_FUNCTION) == MMSYSERR_NOERROR)
            {
               WAVEOUTCAPS waveOutCaps = {0};
               if (waveOutGetDevCaps(reinterpret_cast<uint_ptr>(hWaveOut),
                  &waveOutCaps, sizeof(waveOutCaps)) == MMSYSERR_NOERROR)
               {
                  InfoMsg1("Opened \"%s\" audio output device\n", waveOutCaps.szPname);
               }

               m_channels[format] = hWaveOut;
            }
         }
      }

      if (hWaveOut == NULL || pWavSound->Prepare(hWaveOut) != S_OK)
      {
         return E_FAIL;
      }
   }

   if (pWavSound->Write() != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

///////////////////////////////////////

void CALLBACK cWinmmSoundManager::WaveOutCallback(HWAVEOUT hWaveOut, uint msg, uint instance, uint param1, uint param2)
{
}

///////////////////////////////////////

bool cWinmmSoundManager::sLessWaveFormat::operator()(const WAVEFORMATEX & lhs, const WAVEFORMATEX & rhs) const
{
   return (memcmp(&lhs, &rhs, sizeof(WAVEFORMATEX)) < 0);
}

///////////////////////////////////////

tResult SoundManagerCreate()
{
   cAutoIPtr<ISoundManager> p(static_cast<ISoundManager*>(new cWinmmSoundManager));
   return RegisterGlobalObject(IID_ISoundManager, p);
}


///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////

void * WavSoundLoad(IReader * pReader)
{
   if (pReader == NULL)
   {
      return NULL;
   }

   UseGlobal(SoundManager);
   if (!!pSoundManager)
   {
      tSoundId soundId;
      if (pSoundManager->Open(pReader, &soundId) == S_OK)
      {
         return soundId;
      }
   }

   return NULL;
}

///////////////////////////////////////

void WavSoundUnload(void * pData)
{
   UseGlobal(SoundManager);
   if (!!pSoundManager)
   {
      if (pSoundManager->Close(reinterpret_cast<tSoundId>(pData)) != S_OK)
      {
         WarnMsg1("Error closing sound %d\n", pData);
      }
   }
}


///////////////////////////////////////////////////////////////////////////////

tResult SoundResourceRegister()
{
   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      if (pResourceManager->RegisterFormat(kRT_WavSound, "wav", WavSoundLoad, NULL, WavSoundUnload) == S_OK)
      {
         return S_OK;
      }
   }
   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////

class cSoundTests : public CppUnit::TestCase
{
   void Test1();

   CPPUNIT_TEST_SUITE(cSoundTests);
      CPPUNIT_TEST(Test1);
   CPPUNIT_TEST_SUITE_END();
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cSoundTests);

////////////////////////////////////////

void cSoundTests::Test1()
{
   static const tChar szTestSound[] = _T("click.wav");
   UseGlobal(ResourceManager);
   void * pvSoundId;
   if (pResourceManager->Load(szTestSound, kRT_WavSound, NULL, &pvSoundId) == S_OK)
   {
      tSoundId soundId = (tSoundId)pvSoundId;
      UseGlobal(SoundManager);
      pSoundManager->Play(soundId);
      pResourceManager->Unload(szTestSound, kRT_WavSound);
   }
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
