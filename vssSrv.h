#pragma once

#ifdef VSS_WINDOWS
#include <windows.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <poll.h> // struct pollfd

#include "platform.h"

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

extern void PingServer(struct sockaddr_in*);
extern void Srv_UpdateMasterVolume(float);
extern int actorMessageMM(const char*, struct sockaddr_in*);
extern int actorMessageHandler(const char*);

// Only to test vss by sending messages to itself.
extern float ClientReturnVal();
#ifdef UNUSED
extern const char * ClientReturnValString();
#endif

extern void ReturnFloatToClient(float);
extern void ReturnStringToClient(const char*);

extern float* PvzMessageGroupRecentHandle();

extern struct pollfd vpfd;
