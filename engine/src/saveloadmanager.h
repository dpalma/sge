////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SAVELOADMANAGER_H
#define INCLUDED_SAVELOADMANAGER_H

#include "engine/saveloadapi.h"

#include "tech/digraph.h"
#include "tech/globalobjdef.h"
#include "tech/readwriteapi.h"

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

   virtual tResult OpenSingleEntry(IReader * pReader, REFGUID id, IReader * * ppEntryReader);

   virtual void Reset();

private:
   struct sFileEntry;
   tResult LoadEntryTable(IReader * pReader, std::vector<sFileEntry> * pEntries);

   struct sLessGuid
   {
      bool operator()(const GUID * pLhs, const GUID * pRhs) const
      {
         return (memcmp(pLhs, pRhs, sizeof(GUID)) < 0);
      }
   };

   typedef std::map<const GUID *, cVersionedParticipant *, sLessGuid> tParticipantMap;
   tParticipantMap m_participantMap;

   typedef cDigraph<const GUID *, int, sLessGuid> tConstraintGraph;
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
