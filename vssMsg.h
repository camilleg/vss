// Send strings over UDP.

#pragma once
#include "vssglobals.h"

extern void setAckPrint(int);
extern void Msgsend(struct sockaddr_in*, const char*);
extern void clientMessageCall(char*);

extern const char* GetVssLibVersion(); // from vssBuild.c++
extern const char* GetVssLibDate();
