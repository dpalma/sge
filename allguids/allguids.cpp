///////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(_WIN32)
#define DEFINE_IID_IUNKNOWN
#else
#pragma comment(lib, "uuid")
#endif
#define INITGUID
#include "combase.h"
#include "techguids.h"
#include "renderguids.h"
#include "engineguids.h"
#include "gameguids.h"
#include "editorguids.h"
#include "guiguids.h"
