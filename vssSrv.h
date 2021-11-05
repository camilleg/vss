#pragma once

#ifdef VSS_WINDOWS
#include <windows.h>
#endif

#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "platform.h"
#include "vssglobals.h"

#define iNil (-1)
#define hNil (-1.0f)

typedef unsigned long ulong;

inline void FloatCopy(void* dst, const void* src, const int n)
{
	memcpy(dst, src, n * sizeof(float));
}

inline void ZeroFloats(void* dst, const int n)
{
	memset(dst, 0, n * sizeof(float));
}

// vssSrv.c++
extern void PingServer(struct sockaddr_in*);
extern void Srv_UpdateMasterVolume(float);
extern void Srv_UpdateMidiVolume(float);
extern int actorMessageMM(const void*, struct sockaddr_in*);
extern int actorMessageHandler(const char*);

// Only to test vss by sending messages to itself.
extern float ClientReturnVal();
extern const char * ClientReturnValString();

extern void ReturnFloatToClient(float);
extern void ReturnStringToClient(const char*);

extern float* PvzMessageGroupRecentHandle();
