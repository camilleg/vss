/*
from p2c file HP/include/asm.h written by Dave Gillespie
p2c.shar01:XFuncMacro   asm_iand(a,b) = (a & b)
p2c.shar01:XFuncMacro   asm_ior(a,b) = (a | b)

from p2c file examples/p2crc
p2c.shar01:XFuncMacro   hpm_new(p,n) = (*p = Malloc(n))
p2c.shar01:XFuncMacro   hpm_dispose(p,n) = Free(*p)

from p2c file HP/import/hpib_3.imp
p2c.shar01:XFuncMacro   misc_getioerrmsg(s,io) = \
	sprintf(s, "I/O Error %d", (int)io)
p2c.shar01:XFuncMacro   misc_printerror(er,io) = \
	printf("Error %d/%d!\n", (int)er, (int)io)
*/

#define   asm_iand(a,b)	(a & b)
#define   asm_ior(a,b)	(a | b)

#define   hpm_new(p,n)	(*p = malloc(n))
#define   hpm_dispose(p,n)	(free(*p))

#define   misc_getioerrmsg(s,io) sprintf(s, "I/O Error %d", (int)io)
#define   misc_printerror(er,io) printf("Error %d/%d!\n", (int)er, (int)io)

#define	  gotoxy(X,Y)	(0)

