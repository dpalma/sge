///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "soundnull.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cNullSoundManager
//

///////////////////////////////////////

cNullSoundManager::cNullSoundManager()
{
}

///////////////////////////////////////

cNullSoundManager::~cNullSoundManager()
{
}

///////////////////////////////////////

tResult cNullSoundManager::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cNullSoundManager::Term()
{
   return S_OK;
}

///////////////////////////////////////

tResult cNullSoundManager::Open(IReader * pReader, tSoundId * pSoundId)
{
   if (pReader == NULL || pSoundId == NULL)
   {
      return E_POINTER;
   }

   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cNullSoundManager::Close(tSoundId soundId)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cNullSoundManager::Play(tSoundId soundId)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult NullSoundManagerCreate()
{
   cAutoIPtr<ISoundManager> p(static_cast<ISoundManager*>(new cNullSoundManager));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_ISoundManager, p);
}

///////////////////////////////////////////////////////////////////////////////
