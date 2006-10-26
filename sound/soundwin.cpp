///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "soundwin.h"

#include "tech/resourceapi.h"
#include "tech/readwriteapi.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cWavSound
//

///////////////////////////////////////

cWavSound::cWavSound(const WAVEFORMATEX & format, byte * pData, uint dataLength)
 : m_pData(pData)
 , m_dataLength(dataLength)
 , m_hWaveOut(NULL)
{
   memcpy(&m_format, &format, sizeof(WAVEFORMATEX));
}

///////////////////////////////////////

cWavSound::~cWavSound()
{
   Unprepare();
   delete [] m_pData;
}

///////////////////////////////////////

bool cWavSound::IsPrepared() const
{
   return (m_hWaveOut != NULL);
}

///////////////////////////////////////

tResult cWavSound::Prepare(HWAVEOUT hWaveOut)
{
   ZeroMemory(&m_hdr, sizeof(WAVEHDR));
   m_hdr.dwBufferLength = m_dataLength;
   m_hdr.lpData = reinterpret_cast<LPSTR>(m_pData);
   m_hdr.dwUser = reinterpret_cast<uint_ptr>(this);

   if (waveOutPrepareHeader(hWaveOut, &m_hdr, sizeof(WAVEHDR)) == MMSYSERR_NOERROR)
   {
      m_hWaveOut = hWaveOut;
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cWavSound::Unprepare()
{
   if (m_hWaveOut == NULL)
   {
      return S_FALSE; // already un-prepared or never prepared in the 1st place
   }
   if (m_hWaveOut != NULL)
   {
      if (waveOutUnprepareHeader(m_hWaveOut, &m_hdr, sizeof(WAVEHDR)) == MMSYSERR_NOERROR)
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
   if (m_hWaveOut != NULL)
   {
      if (waveOutWrite(m_hWaveOut, &m_hdr, sizeof(WAVEHDR)) == MMSYSERR_NOERROR)
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
 : m_hWaveOutCallbackWnd(NULL)
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

   static const tChar szWaveOutCallbackWndClass[] = _T("WaveOutCallbackWnd");
   WNDCLASS wc = {0};
   wc.lpfnWndProc = WaveOutCallbackWndProc;
   wc.hInstance = GetModuleHandle(NULL);
   wc.lpszClassName = szWaveOutCallbackWndClass;
   wc.cbWndExtra = sizeof(void*);
   if (!RegisterClass(&wc))
   {
      ErrorMsg("An error occurred registering the audio output callback window class\n");
      return E_FAIL;
   }

   m_hWaveOutCallbackWnd = CreateWindow(wc.lpszClassName, NULL,
      0, 0, 0, 0, 0, NULL, NULL, wc.hInstance, this);
   if (m_hWaveOutCallbackWnd == NULL)
   {
      ErrorMsg("An error occurred creating the audio output callback window\n");
      return E_FAIL;
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
         m_closeQueue.insert(*iter);
      }
      m_sounds.clear();
   }

   {
      tChannelMap::iterator iter = m_channels.begin();
      for (; iter != m_channels.end(); iter++)
      {
         if (waveOutClose(iter->second) == WAVERR_STILLPLAYING)
         {
            waveOutReset(iter->second);
            waveOutClose(iter->second);
         }
      }
      m_channels.clear();
   }

   {
      std::set<cWavSound *>::iterator iter = m_closeQueue.begin();
      for (; iter != m_closeQueue.end(); iter++)
      {
         (*iter)->Unprepare();
         delete (*iter);
      }
      m_closeQueue.clear();
   }

   if (IsWindow(m_hWaveOutCallbackWnd))
   {
      DestroyWindow(m_hWaveOutCallbackWnd);
      m_hWaveOutCallbackWnd = NULL;
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
         m_closeQueue.insert(*iter);
         m_sounds.erase(iter);
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
               reinterpret_cast<uint_ptr>(m_hWaveOutCallbackWnd),
               0, CALLBACK_WINDOW) == MMSYSERR_NOERROR)
            {
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

void cWinmmSoundManager::OnDeviceOpen(HWAVEOUT hWaveOut)
{
   WAVEOUTCAPS waveOutCaps = {0};
   if (waveOutGetDevCaps(reinterpret_cast<uint_ptr>(hWaveOut),
      &waveOutCaps, sizeof(waveOutCaps)) == MMSYSERR_NOERROR)
   {
      InfoMsg1("Opened \"%s\" audio output device\n", waveOutCaps.szPname);
   }
}

///////////////////////////////////////

void cWinmmSoundManager::OnDeviceClose(HWAVEOUT hWaveOut)
{
}

///////////////////////////////////////

void cWinmmSoundManager::OnSoundDone(cWavSound * pWavSound)
{
   if ((pWavSound != NULL) && (m_closeQueue.find(pWavSound) != m_closeQueue.end()))
   {
      m_closeQueue.erase(pWavSound);
      pWavSound->Unprepare();
      delete pWavSound;
   }
}

///////////////////////////////////////

LRESULT CALLBACK cWinmmSoundManager::WaveOutCallbackWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   cWinmmSoundManager * pThis = NULL;
   if (message == WM_NCCREATE)
   {
      CREATESTRUCT * pcs = reinterpret_cast<CREATESTRUCT *>(lParam);
      pThis = reinterpret_cast<cWinmmSoundManager*>(pcs->lpCreateParams);
      SetWindowLongPtr(hWnd, 0, reinterpret_cast<long_ptr>(pThis));
   }
   else
   {
      pThis = reinterpret_cast<cWinmmSoundManager*>(GetWindowLongPtr(hWnd, 0));
   }

   switch (message)
   {
      case MM_WOM_OPEN:
      {
         HWAVEOUT hWaveOut = (HWAVEOUT)wParam;
         pThis->OnDeviceOpen(hWaveOut);
         break;
      }

      case MM_WOM_CLOSE:
      {
         HWAVEOUT hWaveOut = (HWAVEOUT)wParam;
         pThis->OnDeviceClose(hWaveOut);
         break;
      }

      case MM_WOM_DONE:
      {
         HWAVEOUT hWaveOut = (HWAVEOUT)wParam;
         WAVEHDR * pWaveHdr = (WAVEHDR*)lParam;
         cWavSound * pWavSound = reinterpret_cast<cWavSound*>(pWaveHdr->dwUser);
         pThis->OnSoundDone(pWavSound);
         break;
      }
   }

   return DefWindowProc(hWnd, message, wParam, lParam);
}

///////////////////////////////////////

bool cWinmmSoundManager::sLessWaveFormat::operator()(const WAVEFORMATEX & lhs, const WAVEFORMATEX & rhs) const
{
   return (memcmp(&lhs, &rhs, sizeof(WAVEFORMATEX)) < 0);
}

///////////////////////////////////////

extern tResult SoundResourceRegister();

tResult SoundManagerCreate()
{
   SoundResourceRegister();
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
         WarnMsg1("Error closing sound %p\n", pData);
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
