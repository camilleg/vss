/*
 * Copyright (c) 1992 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
	vssMsg.h
		send data over udp packets
*/

#ifndef __MSG_H__
#define __MSG_H__

#include "vssglobals.h"

#define hNil (-1.f)

extern "C" void setAckPrint(int flag);
extern "C" void Msgsend(struct sockaddr_in *paddr, mm* pmm);
extern "C" void clientMessageCall(char* Message);

extern "C" const char* GetVssLibVersion(void); /* from vssBuild.c++ */
extern "C" const char* GetVssLibDate(void);

#endif /* __MSG_H__ */
