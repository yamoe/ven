#pragma once

#define WIN32_LEAN_AND_MEAN

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

#ifndef _WINSOCKAPI_
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>
#endif
#include <process.h>

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

#include "type.h"
#include "cpu.h"
#include "elapsed_timer.h"
#include "non_copyable.h"
#include "singleton.h"
#include "str.h"
#include "thread.h"
#include "tls.h"

#include "lock.h"
#include "auto.h"
#include "pool_ptr.h"

#include "next.h"
#include "lfstack.h"
#include "slist.h"
#include "slist_locker.h"

