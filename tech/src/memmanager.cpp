///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"
#include "memmanager.h"
#include <memory.h>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#include <ctime>
#endif

#include "dbgalloc.h" // must be last header

// REFERENCES
// http://www.zetetics.com/bj/papers/ms/pat4th-c.html

LOG_DEFINE_CHANNEL(MemMgr);

F_DECLARE_INTERFACE(IMemManagerPrivate);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IMemManagerPrivate
//

struct sMemStats
{
   int nBlocksFree;
   IMemManager::size_type sizeFree;

   int nBlocksInUse;
   IMemManager::size_type sizeInUse;
};

interface IMemManagerPrivate : IUnknown
{
   virtual void GetMemStats(sMemStats * pMemStats) const = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMemManager
//

class cMemManager : public cComObject2<IMPLEMENTS(IMemManager), IMPLEMENTS(IMemManagerPrivate)>
{
   friend IMemManager * MemManagerCreate(void * pMem, size_t size);

   cMemManager();
   cMemManager(ptr_type pMem, size_type size);

public:
   virtual ~cMemManager();

   virtual void ManageMemory(ptr_type pMem, size_type size);

   virtual ptr_type GetManagedMemory() const;

   virtual ptr_type Alloc(size_type size);
   virtual void Free(ptr_type p);

   void GetMemStats(sMemStats * pMemStats) const;

   static uint gm_memBlockSize;

private:
   struct sMemBlock;

   size_type BlockSize(sMemBlock * pBlock) const;

   ptr_type m_pMem;
   size_type m_memSize;

   // This struct really needs only 5 bytes. (With some work, the pointer and 
   // flag could actually be stuffed into 4 bytes.) However, it's best to keep 
   // the struct padded such that pointers will always be 4-byte aligned.
   struct sMemBlock
   {
      bool bFree;
      IMemManager::ptr_type pNext;
   };

   sMemBlock * m_pHead, * m_pTail;
};

uint cMemManager::gm_memBlockSize = sizeof(sMemBlock);

///////////////////////////////////////

cMemManager::cMemManager()
 : m_pMem(NULL),
   m_memSize(0),
   m_pHead(NULL),
   m_pTail(NULL)
{
}

///////////////////////////////////////

cMemManager::cMemManager(ptr_type pMem, size_type size)
 : m_pMem(pMem),
   m_memSize(size),
   m_pHead(NULL),
   m_pTail(NULL)
{
   ManageMemory(pMem, size);
}

///////////////////////////////////////

cMemManager::~cMemManager()
{
}

///////////////////////////////////////

void cMemManager::ManageMemory(ptr_type pMem, size_type size)
{
   m_pMem = pMem;
   m_memSize = size;

#ifndef NDEBUG
   if (m_pMem != NULL)
      memset(m_pMem, 0xCC, m_memSize);
#endif

   m_pHead = (sMemBlock *)m_pMem;
   if (m_pHead != NULL)
   {
      m_pHead->bFree = true;
      m_pHead->pNext = NULL;
   }
   m_pTail = m_pHead;

   DebugMsgEx2(MemMgr, "Managing memory at 0x%08X, %d bytes long\n", pMem, size);
}

///////////////////////////////////////

cMemManager::ptr_type cMemManager::GetManagedMemory() const
{
   return m_pMem;
}

///////////////////////////////////////

cMemManager::ptr_type cMemManager::Alloc(size_type size)
{
   Assert(size > sizeof(sMemBlock)); // not meant for such small allocations

   if (m_pMem == NULL)
      return NULL;

   sMemBlock * pBlock = (sMemBlock *)m_pHead;
   for (; pBlock != NULL; pBlock = (sMemBlock *)pBlock->pNext)
   {
      size_type blockBytes = BlockSize(pBlock);

      if (pBlock->bFree && (sizeof(*pBlock) + size) <= blockBytes)
      {
         sMemBlock * pNewBlock = (sMemBlock *)((byte *)pBlock + sizeof(*pBlock) + size);

         pNewBlock->bFree = true;
         pNewBlock->pNext = pBlock->pNext;
         if (pBlock->pNext != NULL)
            pBlock->pNext = pNewBlock;

         pBlock->bFree = false;
         pBlock->pNext = pNewBlock;

         if (m_pTail == pBlock)
            m_pTail = pNewBlock;

         pBlock++;
#ifndef NDEBUG
         memset(pBlock, 0xCD, size);
#endif
         return pBlock;
      }
   }

   return NULL;
}

///////////////////////////////////////

void cMemManager::Free(ptr_type p)
{
   sMemBlock * pBlockPrev = NULL;
   sMemBlock * pBlockPrevPrev = NULL;
   sMemBlock * pBlock = (sMemBlock *)m_pHead;
   for (; pBlock != NULL; pBlock = (sMemBlock *)pBlock->pNext)
   {
      if ((ptr_type)(pBlock + 1) == p)
      {
         pBlock->bFree = true;

         sMemBlock * pBlockNext = (sMemBlock *)pBlock->pNext;

         if ((pBlockPrev != NULL) && pBlockPrev->bFree)
         {
            if (m_pTail == pBlock)
               m_pTail = pBlockPrev;
            pBlockPrev->pNext = pBlockNext;
            // block pointed to by 'pBlock' ceases to exist
            pBlock = pBlockPrev;
            pBlockPrev = pBlockPrevPrev;
         }

         if ((pBlockNext != NULL) && pBlockNext->bFree)
         {
            if (m_pTail == pBlockNext)
               m_pTail = pBlock;
            pBlock->pNext = pBlockNext->pNext;
         }

#ifndef NDEBUG
         memset(pBlock + 1, 0xDD, BlockSize(pBlock));
#endif

         break;
      }

      pBlockPrevPrev = pBlockPrev;
      pBlockPrev = pBlock;
   }
}

///////////////////////////////////////

void cMemManager::GetMemStats(sMemStats * pMemStats) const
{
   Assert(pMemStats != NULL);

   sMemBlock * pBlock = (sMemBlock *)m_pHead;
   for (; pBlock != NULL; pBlock = (sMemBlock *)pBlock->pNext)
   {
      size_type blockBytes = BlockSize(pBlock);

      if (pBlock->bFree)
      {
         pMemStats->nBlocksFree++;
         pMemStats->sizeFree += blockBytes;
      }
      else
      {
         pMemStats->nBlocksInUse++;
         pMemStats->sizeInUse += blockBytes;
      }
   }
}

///////////////////////////////////////

cMemManager::size_type cMemManager::BlockSize(sMemBlock * pBlock) const
{
   Assert(pBlock != NULL);
   if (pBlock->pNext != NULL)
   {
      return (byte *)pBlock->pNext - (byte *)pBlock - sizeof(*pBlock);
   }
   else
   {
      Assert(pBlock == m_pTail);
      return m_memSize - ((byte *)pBlock - (byte *)m_pHead) - sizeof(*pBlock);
   }
}

///////////////////////////////////////////////////////////////////////////////

IMemManager * MemManagerCreate(void * pMem, size_t size)
{
   return static_cast<IMemManager *>(new cMemManager(pMem, size));
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cMemManagerTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cMemManagerTests);
      CPPUNIT_TEST(testAlloc);
      CPPUNIT_TEST(testFree);
      CPPUNIT_TEST(testCoalesce);
   CPPUNIT_TEST_SUITE_END();

public:
   void testAlloc()
   {
      char memory[100];

      cAutoIPtr<IMemManager> pMemMgr = MemManagerCreate((cMemManager::ptr_type)memory, sizeof(memory));

      char * p1 = (char *)pMemMgr->Alloc(21);
      strcpy(p1, "barfbarfbarfbarfbarf");

      char * p2 = (char *)pMemMgr->Alloc(20);

      CPPUNIT_ASSERT(pMemMgr->Alloc(50) == NULL);

      sMemStats memStats;

      cAutoIPtr<IMemManagerPrivate> pMemMgrPriv;
      CPPUNIT_ASSERT(pMemMgr->QueryInterface(IID_IMemManagerPrivate, (void**)&pMemMgrPriv) == S_OK);

      memset(&memStats, 0, sizeof(memStats));
      pMemMgrPriv->GetMemStats(&memStats);

      CPPUNIT_ASSERT(memStats.nBlocksInUse == 2);
      CPPUNIT_ASSERT(memStats.sizeInUse == 41);

      ulong totalSize = memStats.sizeInUse + (memStats.nBlocksInUse * cMemManager::gm_memBlockSize) +
                        memStats.sizeFree + (memStats.nBlocksFree * cMemManager::gm_memBlockSize);
      CPPUNIT_ASSERT(totalSize == sizeof(memory));
   }

   void testFree()
   {
      byte memory[256];

      cAutoIPtr<IMemManager> pMemMgr = MemManagerCreate((cMemManager::ptr_type)memory, sizeof(memory));

      cMemManager::ptr_type p1 = pMemMgr->Alloc(32);
      cMemManager::ptr_type p2 = pMemMgr->Alloc(32);
      cMemManager::ptr_type p3 = pMemMgr->Alloc(16);
      cMemManager::ptr_type p4 = pMemMgr->Alloc(64);

      sMemStats memStats;

      cAutoIPtr<IMemManagerPrivate> pMemMgrPriv;
      CPPUNIT_ASSERT(pMemMgr->QueryInterface(IID_IMemManagerPrivate, (void**)&pMemMgrPriv) == S_OK);

      memset(&memStats, 0, sizeof(memStats));
      pMemMgrPriv->GetMemStats(&memStats);

      CPPUNIT_ASSERT(memStats.nBlocksInUse == 4);
      CPPUNIT_ASSERT(memStats.sizeInUse == (32+32+16+64));

      pMemMgr->Free(p2);

      memset(&memStats, 0, sizeof(memStats));
      pMemMgrPriv->GetMemStats(&memStats);
      CPPUNIT_ASSERT(memStats.nBlocksInUse == 3);
      CPPUNIT_ASSERT(memStats.sizeInUse == (32+16+64));

      ulong totalSize = memStats.sizeInUse + (memStats.nBlocksInUse * cMemManager::gm_memBlockSize) +
                        memStats.sizeFree + (memStats.nBlocksFree * cMemManager::gm_memBlockSize);
      CPPUNIT_ASSERT(totalSize == sizeof(memory));
   }

   void testCoalesce()
   {
      srand(time(NULL));

      byte memory[1024];

      cAutoIPtr<IMemManager> pMemMgr = MemManagerCreate((cMemManager::ptr_type)memory, sizeof(memory));

      std::vector<cMemManager::ptr_type> pointers;

      cMemManager::ptr_type p = NULL;
      do
      {
         cMemManager::size_type allocSize = 12 + (rand() % 9);
         p = pMemMgr->Alloc(allocSize);
         if (p != NULL)
            pointers.push_back(p);
      }
      while (p != NULL);

      for (int i = 0; i < (2 * pointers.size()); i++)
      {
         int s1 = rand() % pointers.size(), s2 = s1;
         while (s1 == s2)
            s2 = rand() % pointers.size();
         cMemManager::ptr_type temp = pointers[s1];
         pointers[s1] = pointers[s2];
         pointers[s2] = temp;
      }

      for (i = 0; i < pointers.size(); i++)
      {
         pMemMgr->Free(pointers[i]);
      }

      sMemStats memStats;

      cAutoIPtr<IMemManagerPrivate> pMemMgrPriv;
      CPPUNIT_ASSERT(pMemMgr->QueryInterface(IID_IMemManagerPrivate, (void**)&pMemMgrPriv) == S_OK);

      memset(&memStats, 0, sizeof(memStats));
      pMemMgrPriv->GetMemStats(&memStats);

      CPPUNIT_ASSERT(memStats.nBlocksInUse == 0);
      CPPUNIT_ASSERT(memStats.sizeInUse == 0);

      CPPUNIT_ASSERT(memStats.nBlocksFree == 1);

      ulong totalSize = memStats.sizeInUse + (memStats.nBlocksInUse * cMemManager::gm_memBlockSize) +
                        memStats.sizeFree + (memStats.nBlocksFree * cMemManager::gm_memBlockSize);
      CPPUNIT_ASSERT(totalSize == sizeof(memory));
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(cMemManagerTests);

///////////////////////////////////////////////////////////////////////////////

#endif
