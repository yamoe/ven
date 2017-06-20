#pragma once

#define WIN32_LEAN_AND_MEAN


#ifndef _WINSOCKAPI_
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>
#endif

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

#include <process.h>

#pragma warning(disable : 4091)
#include <DbgHelp.h>
#pragma warning(default : 4091)
#pragma comment(lib,"DbgHelp.Lib")

#include <stdint.h>
#include <string>
#include <deque>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <atomic>
#include <conio.h>

#include "cpu.h"
#include "elapsed_timer.h"
#include "non_copyable.h"
#include "singleton.h"
#include "func.h"
#include "thread.h"
#include "tls.h"

#include "lock.h"
#include "auto.h"
#include "pool_ptr.h"

#include "next.h"
#include "lfstack.h"
#include "slist.h"
#include "slist_locker.h"


#include "char_array.h"
#include "file.h"
#include "stack_trace.h"
#include "crash_dump.h"

#include "time.h"
#include "logger.h"

#include "win_service.h"

