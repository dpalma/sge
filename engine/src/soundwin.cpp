///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "soundwin.h"

#include "resourceapi.h"
#include "readwriteapi.h"

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>
#include <mmsystem.h>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header


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
   return S_OK;
}

///////////////////////////////////////

tResult cWinmmSoundManager::Term()
{
   return S_OK;
}

///////////////////////////////////////

tResult cWinmmSoundManager::Open(IReader * pReader, tSoundId * pSoundId)
{
   if (pReader == NULL || pSoundId == NULL)
   {
      return E_POINTER;
   }

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

      if (pReader->Read(pBuffer, length) != S_OK)
      {
         delete [] pBuffer;
         return E_FAIL;
      }

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
                  }
               }
            }
         }

         mmioClose(hMmio, 0);
      }

      delete [] pBuffer, pBuffer = NULL;
   }

   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cWinmmSoundManager::Close(tSoundId soundId)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cWinmmSoundManager::Play(tSoundId soundId)
{
   return E_NOTIMPL;
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

   tSoundId soundId;
   UseGlobal(SoundManager);
   if (pSoundManager->Open(pReader, &soundId) == S_OK)
   {
      return soundId;
   }

   return NULL;
}

///////////////////////////////////////

void WavSoundUnload(void * pData)
{
   UseGlobal(SoundManager);
   if (pSoundManager->Close(reinterpret_cast<tSoundId>(pData)) != S_OK)
   {
      WarnMsg1("Error closing sound %d\n", pData);
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
   static const tChar szTestSound[] = _T("beep.wav");
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
