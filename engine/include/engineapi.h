///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENGINEAPI_H
#define INCLUDED_ENGINEAPI_H

#include "enginedll.h"
#include "comtools.h"
#include "readwriteapi.h"
#include "techstring.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

#define kRT_TiXml _T("TiXml")

ENGINE_API tResult EngineRegisterResourceFormats();
ENGINE_API tResult EngineRegisterScriptFunctions();

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMapProperties
//

F_DECLARE_GUID(SAVELOADID_MapProperties);

////////////////////////////////////////

class ENGINE_API cMapProperties
{
   friend class cReadWriteOps<cMapProperties>;

public:
   cMapProperties();
   cMapProperties(const tChar * pszTitle, const tChar * pszAuthor,
      const tChar * pszDescription, int nPlayers);
   cMapProperties(const cMapProperties & other);
   ~cMapProperties();

   const tChar * GetTitle() const;
   void SetTitle(const tChar *);

   const tChar * GetAuthor() const;
   void SetAuthor(const tChar *);

   const tChar * GetDescription() const;
   void SetDescription(const tChar *);

   int GetNumPlayers() const;
   void SetNumPlayers(int numPlayers);

private:
   cStr m_title, m_author, m_description;
   int m_nPlayers;
};

////////////////////////////////////////

template <>
class ENGINE_API cReadWriteOps<cMapProperties>
{
public:
   static tResult Read(IReader * pReader, cMapProperties * pMapProperties);
   static tResult Write(IWriter * pWriter, const cMapProperties & mapProperties);
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENGINEAPI_H
