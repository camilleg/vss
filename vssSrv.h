#ifndef __SRV_H__
#define __SRV_H__

/* vssSrv.h  --  #include this only from C++, not from C! */

#ifdef VSS_WINDOWS
#include <windows.h> // This has to be included before most other stuff.
#endif

using namespace std;

#include "platform.h"
#include "vssglobals.h"

#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>


#define iNil (-1)
#define hNil (-1.f)	/* this is also defined in vssMsg.h */

typedef unsigned long ulong;

// booleans
#ifndef true
#define false   0
#define true    1
#endif

//===========================================================================
//	handy inlines
//
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

// Prototypes from vssSrv.c++

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

#endif /* __SRV_H__ */
