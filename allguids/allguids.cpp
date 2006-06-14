///////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(_WIN32)
#define DEFINE_IID_IUNKNOWN
#else
#pragma comment(lib, "uuid")
#endif
#define INITGUID
#include "combase.h"
#include "editorguids.h"
#include "engineguids.h"
#include "guiguids.h"
#include "netguids.h"
#include "platformguids.h"
#include "renderguids.h"
#include "scriptguids.h"
#include "soundguids.h"
#include "techguids.h"
