#pragma once

#include "mm.h"

#pragma warning(disable: 4996) 
// ^ _close
// Uninitialized variables:
#pragma warning(disable: 4703) 
#pragma warning(disable: 4700) 
#pragma comment( lib, "OpenGL32.lib" ) 
#pragma comment( lib, "glu32.lib" ) 
#pragma comment( lib, "ws2_32.lib" ) 

#define strncasecmp strnicmp
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
#include <assert.h>
#define _NOT_IMPLEMENTED assert(0);

#define dprintf(...) 
//#define dprintf(...) printf(__VA_ARGS__)

