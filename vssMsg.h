// Send data over UDP packets in "mm" format.

#pragma once
#include "vssglobals.h"

extern void setAckPrint(int);
extern void Msgsend(struct sockaddr_in*, mm*);
extern void clientMessageCall(char*);

extern const char* GetVssLibVersion(); // from vssBuild.c++
extern const char* GetVssLibDate();
