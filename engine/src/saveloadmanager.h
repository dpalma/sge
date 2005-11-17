////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SAVELOADMANAGER_H
#define INCLUDED_SAVELOADMANAGER_H

#include "saveloadapi.h"

#include "digraph.h"
#include "globalobjdef.h"
extern "C"
{
#include "md5.h"
}
#include "readwriteapi.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cVersionedParticipant
//

class cVersionedParticipant
{
   cVersionedParticipant(const cVersionedParticipant &);
   void operator =(const cVersionedParticipant &);

public:
   cVersionedParticipant();
   ~cVersionedParticipant();

   tResult AddVersion(int, ISaveLoadParticipant *);
   tResult RemoveVersion(int);

   tResult GetMostRecentVersion(int * pVersion) const;
   tResult GetParticipant(int version, ISaveLoadParticipant * *);

private:
   typedef std::map<int, ISaveLoadParticipant *> tVersionMap;
   tVersionMap m_versionMap;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMD5Writer
//

class cMD5Writer : public cComObject<IMPLEMENTS(IWriter)>
{
public:
   cMD5Writer(IWriter * pWriter);
   ~cMD5Writer();

   void FinalizeMD5(byte digest[16]);

   virtual tResult Tell(ulong * pPos);
   virtual tResult Seek(long pos, eSeekOrigin origin);
   virtual tResult Write(const char * value);
   virtual tResult Write(void * pValue, size_t cbValue, size_t * pcbWritten);

   template <typename T>
   tResult Write(T value)
   {
      return cReadWriteOps<T>::Write(this, value);
   }

private:
   cAutoIPtr<IWriter> m_pWriter;
   bool m_bUpdateMD5;
   MD5_CTX m_context;
};


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSaveLoadManager
//

class cSaveLoadManager : public cComObject2<IMPLEMENTS(ISaveLoadManager), IMPLEMENTS(IGlobalObject)>
{
public:
   cSaveLoadManager();
   ~cSaveLoadManager();

   DECLARE_NAME(SaveLoadManager)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult RegisterSaveLoadParticipant(REFGUID id, const cBeforeAfterConstraint * pConstraints,
      size_t nConstraints, int version, ISaveLoadParticipant *);
   virtual tResult RevokeSaveLoadParticipant(REFGUID id, int version);

   virtual tResult Save(IWriter *);
   virtual tResult Load(IReader *);

private:
   struct sLessGuid
   {
      bool operator()(const GUID * pLhs, const GUID * pRhs) const
      {
         return (memcmp(pLhs, pRhs, sizeof(GUID)) < 0);
      }
   };

   typedef std::map<const GUID *, cVersionedParticipant *, sLessGuid> tParticipantMap;
   tParticipantMap m_participantMap;

   typedef cDigraph<const GUID *, sLessGuid> tConstraintGraph;
   tConstraintGraph m_saveOrderConstraintGraph;

   struct sFileHeader
   {
      GUID id;
      int version;
   };

   struct sFileFooter
   {
      ulong offset;
      ulong length;
      byte digest[16];
   };

   struct sFileEntry
   {
      GUID id;
      int version;
      ulong offset;
      ulong length;
   };

   friend class cReadWriteOps<sFileHeader>;
   friend class cReadWriteOps<sFileFooter>;
   friend class cReadWriteOps<sFileEntry>;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SAVELOADMANAGER_H
