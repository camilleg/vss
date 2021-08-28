// Send data over UDP packets.

#pragma once
#include "vssglobals.h"
#define hNil (-1.f)

extern "C" void setAckPrint(int);
extern "C" void Msgsend(struct sockaddr_in *, mm*);
extern "C" void clientMessageCall(char*);

extern "C" const char* GetVssLibVersion(); // from vssBuild.c++
extern "C" const char* GetVssLibDate();
