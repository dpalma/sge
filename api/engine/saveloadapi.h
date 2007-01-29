////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SAVELOADAPI_H
#define INCLUDED_SAVELOADAPI_H

/// @file saveloadapi.h
/// Interface definitions for a simple serialization system

#include "enginedll.h"
#include "tech/comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE_GUID(ISaveLoadManager, "A7D128C5-74AD-4f46-848B-33A9371978A6");
F_DECLARE_INTERFACE_GUID(ISaveLoadParticipant, "D67CE569-DB15-453b-B616-03D63572C6C6");
F_DECLARE_INTERFACE_GUID(ISaveLoadListener, "3D422401-83C4-4816-89E3-F5880DA70785");

F_DECLARE_INTERFACE(IWriter);
F_DECLARE_INTERFACE(IReader);

class cBeforeAfterConstraint;

////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISaveLoadManager
//

interface ISaveLoadManager : IUnknown
{
   virtual tResult RegisterSaveLoadParticipant(REFGUID id,
      const cBeforeAfterConstraint * pConstraints, size_t nConstraints,
      int version, ISaveLoadParticipant *) = 0;

   inline tResult RegisterSaveLoadParticipant(REFGUID id, int version, ISaveLoadParticipant * p)
   {
      return RegisterSaveLoadParticipant(id, NULL, 0, version, p);
   }

   virtual tResult RevokeSaveLoadParticipant(REFGUID id, int version) = 0;

   virtual tResult Save(IWriter * pWriter) = 0;
   virtual tResult Load(IReader * pReader) = 0;

   virtual tResult OpenSingleEntry(IReader * pReader, REFGUID id, IReader * * ppEntryReader) = 0;

   virtual void Reset() = 0;

	virtual tResult AddSaveLoadListener(ISaveLoadListener * pListener) = 0;
	virtual tResult RemoveSaveLoadListener(ISaveLoadListener * pListener) = 0;
};

///////////////////////////////////////

ENGINE_API tResult SaveLoadManagerCreate();

////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISaveLoadParticipant
//

interface ISaveLoadParticipant : IUnknown
{
   virtual tResult Save(IWriter * pWriter) = 0;
   virtual tResult Load(IReader * pReader, int version) = 0;
   virtual void Reset() = 0;
};

////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISaveLoadListener
//

interface ISaveLoadListener : IUnknown
{
   virtual void OnBeginSave() = 0;
   virtual void OnSaveProgress(uint current, uint bound) = 0;
   virtual void OnEndSave() = 0;

   virtual void OnBeginLoad() = 0;
   virtual void OnLoadProgress(uint current, uint bound) = 0;
   virtual void OnEndLoad() = 0;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SAVELOADAPI_H
