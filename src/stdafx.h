/*
 *
 *  stdafx.h
 *
 */

#pragma once

// Change these values to use different versions
#define WINVER			_WIN32_WINNT_WIN7
#define _WIN32_WINNT	_WIN32_WINNT_WIN7
#define _WIN32_IE		_WIN32_IE_IE110
#define _RICHEDIT_VER	0x0300

#include <sdkddkver.h>

#include <time.h>

#include <atlbase.h>
#include <atlapp.h>


#include <atlwin.h>

#include <atlstr.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atlmisc.h>
#include <atlctrlx.h>
#include <atldlgs.h>
#include <atlddx.h>
#include <atlcrack.h>
#include <atlgdi.h>

#include <atlsync.h>

#include <ATLComTime.h>

#include <ws2tcpip.h>
#include <in6addr.h>
#include <ws2ipdef.h>
#include <IPTypes.h>

#include <comdef.h>

#include <list>
#include <queue>
#include <deque>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <functional>
#include <memory>
#include <atomic>

#define _USE_MATH_DEFINES
#include <math.h>

#include <regex>

#include <gdiplus.h>

using namespace Gdiplus;

#include <bcrypt.h>

#include <strsafe.h>

#include <process.h>
#include <processthreadsapi.h>

#include <wincodec.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>


extern	CAppModule	_Module;
