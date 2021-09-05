// #include this only from C++, not from C.

#pragma once

#ifdef VSS_WINDOWS
#include <windows.h>
#endif

#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

#include "platform.h"
#include "vssglobals.h"

#define iNil (-1)
#define hNil (-1.f)	// also defined in vssMsg.h

typedef unsigned long ulong;

inline void
FloatCopy( void * dst, const void * src, const int n )
{
	memcpy( dst, src, n * sizeof(float) );
}

inline void
ZeroFloats( void *array, const int n )
{
	memset( array, 0, n * sizeof(float) );
}

// from vssSrv.c++

extern void PingServer(struct sockaddr_in *cl_addr);
extern void Srv_UpdateMasterVolume(float newGain);
extern void Srv_UpdateMidiVolume(float newGain);
extern "C" int actorMessageMM(const void* pv, struct sockaddr_in *cl_addr);
extern "C" int actorMessageHandler(const char* message);

// Only so we can test vss by sending messages to itself.
extern "C" float ClientReturnVal(void);
extern "C" const char * ClientReturnValString(void);

extern "C" void ReturnFloatToClient(float);
extern "C" void ReturnStringToClient(const char*);

extern "C" float* PvzMessageGroupRecentHandle(void);
