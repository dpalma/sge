///////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(_WIN32)
#define DEFINE_IID_IUNKNOWN
#else
#pragma comment(lib, "uuid")
#endif
#define INITGUID
#include "tech/combase.h"
#include "editorguids.h"
#include "engine/engineguids.h"
#include "gui/guiguids.h"
#include "network/netguids.h"
#include "platform/platformguids.h"
#include "render/renderguids.h"
#include "script/scriptguids.h"
#include "sound/soundguids.h"
#include "tech/techguids.h"
