///////////////////////////////////////////////////////////////////////////////
// $Id$

// Don't define IID_IUnknown in static builds because a similar definition
// will most likely be linked in via a 'pragma comment(lib, "uuid.lib")' by
// one of the Windows header files
#if (_MSC_VER < 1300) && (!defined(STATIC_BUILD) || !defined(_WIN32))
#define DEFINE_IID_IUNKNOWN
#endif
#define INITGUID
#include "combase.h"
#include "techguids.h"
#include "renderguids.h"
#include "engineguids.h"
#include "gameguids.h"
#include "editorguids.h"
