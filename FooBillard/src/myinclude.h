#include "mm.h"

#define USE_SDL
#define USE_SOUND
#define __WIN32__
#define WIN32
#define _CRT_SECURE_NO_WARNINGS
#define _MBCS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h> // including before windows.h prevents it from including winsock1
#define NOMINMAX
#include <Windows.h>
#define _USE_MATH_DEFINES
#include <math.h>
