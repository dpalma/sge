////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_STDHDR_H
#define INCLUDED_STDHDR_H

#include "tech/techtypes.h"
#include "tech/techassert.h"
#include "tech/techlog.h"
#include "tech/combase.h"

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////

extern tResult ServerInit(int argc, tChar * argv[]);
extern void ServerTerm();
extern tResult ServerFrame();

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_STDHDR_H
