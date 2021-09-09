//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1997.
//===========================================================================
//	actually, this code was written by Camille, he should have his own banner.

#pragma once
#include <fstream>
#include <iostream>
#include <cstring>
#include "VActor.h"

//	sscanf-parsing stuff

// Does "msg" start with command "cmd"?
// A common thing to do in sscanf-style parsing.
//;; only in messageGroup.c++.  use stuff below instead.
static inline int SzStart(const char* msg, const char* cmd)
{
	int cch = strlen(cmd);
	return (msg[cch] == ' ' || msg[cch] == '\0') &&
		!strncmp(msg, cmd, cch);
}

extern "C" float* VrgzMG(void);
extern "C" int SscanfFloats(int cz, float* rgz, const char* sz);
extern "C" int SscanfInts(int cw, int* rgw, const char* sz);

extern "C" char sscanf_cmd[1000];
extern "C" const char* sscanf_msg;
#define CommandIs(_) (!strcmp(sscanf_cmd, _))

#ifdef DEBUG
#define CATCHES // Needs #define DEBUG.
#endif

#ifdef CATCHES
inline int VActor::Catch(void)   { cerr <<typeName() <<": <" <<sscanf_cmd <<sscanf_msg <<">\n"; return 1; }
inline int VActor::Uncatch(void) { cerr<<"msg missed (unexpected args) by " <<typeName() <<"\n\t<" <<sscanf_cmd <<sscanf_msg <<">\n"; return 0; }
inline int Catch(void) { cerr<<"msg caught\n\t<" << sscanf_cmd << sscanf_msg << ">\n"; return 1; }
inline int Uncatch(void) { cerr<<"msg missed (unexpected args)\n\t<" <<sscanf_cmd <<sscanf_msg <<">\n"; return 0; }
#else
inline int VActor::Catch(void)		{ return 1; }
inline int VActor::Uncatch(void)	{ return 0; }
inline int Catch(void)				{ return 1; }
inline int Uncatch(void)			{ return 0; }
#endif

//         ifXYZ(x,y,z, foo(x,y,z));
// means:
// If the arglist for this command begins with args x y z of type X Y Z respectively,
// call the function foo with those args (x,y,z), note that we caught the message
// (if CATCHES is #defined), and return.
// After one or more of these, put the statement
//         return Uncatch();
// to note that we should have caught this message, but missed (because its arg list
// didn't match anything we expected).  ifNil() is a special case - it expects the empty
// arg list (which we can't really test for), so it always succeeds.  A "return Uncatch();"
// after an ifNil() will be unreachable code, so in this case don't bother putting it in.
//
// It is deliberate that the last arg of an ifXYZ macro can only be
// a single statement (i.e., expression), not a { block; of; statements; }.
// This enforces some degree of modularity in your command parser by putting
// such a block in a separate function which has the clearly defined purpose of
// handling that particular message.

#define Do_(_)                  { _; return Catch(); }

#define ifNil(_) { Do_(_) }

#define ifTypeArg1Sz(type,a1,_,sz) \
 { int cch; type a1; if (1 == sscanf(sscanf_msg, sz, &a1,&cch)) Do_(_) }
#define ifTypeArg2Sz(type,a1,a2,_,sz) \
 { int cch; type a1,a2; if (2 == sscanf(sscanf_msg, sz, &a1,&a2,&cch)) Do_(_) }
#define ifTypeArg3Sz(type,a1,a2,a3,_,sz) \
 { int cch; type a1,a2,a3; if (3 == sscanf(sscanf_msg, sz, &a1,&a2,&a3,&cch)) Do_(_) }
#define ifTypeArg4Sz(type,a1,a2,a3,a4,_,sz) \
 { int cch; type a1,a2,a3,a4; if (4 == sscanf(sscanf_msg, sz, &a1,&a2,&a3,&a4,&cch)) Do_(_) }
#define ifTypeArg5Sz(type,a1,a2,a3,a4,a5,_,sz) \
 { int cch; type a1,a2,a3,a4,a5; if (5 == sscanf(sscanf_msg, sz, &a1,&a2,&a3,&a4,&a5,&cch)) Do_(_) }
#define ifTypeArg6Sz(type,a1,a2,a3,a4,a5,a6,_,sz) \
 { int cch; type a1,a2,a3,a4,a5,a6; if (6 == sscanf(sscanf_msg, sz, &a1,&a2,&a3,&a4,&a5,&a6,&cch)) Do_(_) }
#define ifTypeArg7Sz(type,a1,a2,a3,a4,a5,a6,a7,_,sz) \
 { int cch; type a1,a2,a3,a4,a5,a6,a7; if (7 == sscanf(sscanf_msg, sz, &a1,&a2,&a3,&a4,&a5,&a6,&a7,&cch)) Do_(_) }
#define ifTypeArg8Sz(type,a1,a2,a3,a4,a5,a6,a7,a8,_,sz) \
 { int cch; type a1,a2,a3,a4,a5,a6,a7,a8; if (8 == sscanf(sscanf_msg, sz, &a1,&a2,&a3,&a4,&a5,&a6,&a7,&a8,&cch)) Do_(_) }
#define ifTypeArg9Sz(type,a1,a2,a3,a4,a5,a6,a7,a8,a9,_,sz) \
 { int cch; type a1,a2,a3,a4,a5,a6,a7,a8,a9; if (9 == sscanf(sscanf_msg, sz, &a1,&a2,&a3,&a4,&a5,&a6,&a7,&a8,&a9,&cch)) Do_(_) }

#define ifF(a,_)							ifTypeArg1Sz(float,a,_,"%f %n")
#define ifFF(a,b,_)							ifTypeArg2Sz(float,a,b,_,"%f %f %n")
#define ifFFF(a,b,c,_)						ifTypeArg3Sz(float,a,b,c,_,"%f %f %f %n")
#define ifFFFF(a,b,c,d,_)					ifTypeArg4Sz(float,a,b,c,d,_,"%f %f %f %f %n")
#define ifFFFFF(a,b,c,d,e,_)				ifTypeArg5Sz(float,a,b,c,d,e,_,"%f %f %f %f %f %n")
#define ifFFFFFF(a,b,c,d,e,f,_)				ifTypeArg6Sz(float,a,b,c,d,e,f,_,"%f %f %f %f %f %f %n")
#define ifFFFFFFF(a,b,c,d,e,f,g,_)			ifTypeArg7Sz(float,a,b,c,d,e,f,g,_,"%f %f %f %f %f %f %f %n")
#define ifFFFFFFFF(a,b,c,d,e,f,g,h,_)		ifTypeArg8Sz(float,a,b,c,d,e,f,g,h,_,"%f %f %f %f %f %f %f %f %n")
#define ifFFFFFFFFF(a,b,c,d,e,f,g,h,i,_)	ifTypeArg9Sz(float,a,b,c,d,e,f,g,h,i,_,"%f %f %f %f %f %f %f %f %f %n")

#define ifD(a,_)							ifTypeArg1Sz(int,a,_,"%d %n")
#define ifDD(a,b,_)							ifTypeArg2Sz(int,a,b,_,"%d %d %n")
#define ifDDD(a,b,c,_)						ifTypeArg3Sz(int,a,b,c,_,"%d %d %d %n")
#define ifDDDD(a,b,c,d,_)					ifTypeArg4Sz(int,a,b,c,d,_,"%d %d %d %d %n")
#define ifDDDDD(a,b,c,d,e,_)				ifTypeArg5Sz(int,a,b,c,d,e,_,"%d %d %d %d %d %n")
#define ifDDDDDD(a,b,c,d,e,f,_)				ifTypeArg6Sz(int,a,b,c,d,e,f,_,"%d %d %d %d %d %d %n")
#define ifDDDDDDD(a,b,c,d,e,f,g,_)			ifTypeArg7Sz(int,a,b,c,d,e,f,g,_,"%d %d %d %d %d %d %d %n")
#define ifDDDDDDDD(a,b,c,d,e,f,g,h,_)		ifTypeArg8Sz(int,a,b,c,d,e,f,g,h,_,"%d %d %d %d %d %d %d %d %n")
#define ifDDDDDDDDD(a,b,c,d,e,f,g,h,i,_)	ifTypeArg9Sz(int,a,b,c,d,e,f,g,h,i,_,"%d %d %d %d %d %d %d %d %d %n")

// Nasty: DF will parse "3.1416" as "int 3; float .1416",
// in the naive sscanf.  Workaround: replace "%d %f" with "%d%*[ \t\r\n]%f".

// all length-2 nonhomogeneous combinations of D and F

#define ifDF(a,b,_)			{ int cch; int a; float b;  if (2 == sscanf(sscanf_msg, "%d%*[ \t\r\n]%f %n", &a,&b,&cch)) Do_(_) }
#define ifFD(a,b,_)			{ int cch; int b; float a;  if (2 == sscanf(sscanf_msg, "%f %d %n", &a,&b,&cch)) Do_(_) }

// all length-3 nonhomogeneous combinations of D and F

#define ifDDF(a,b,c,_)		{ int cch; int a,b; float c; if (3 == sscanf(sscanf_msg, "%d %d%*[ \t\r\n]%f %n", &a,&b,&c,&cch)) Do_(_) }
#define ifDFD(a,b,c,_)		{ int cch; int a,c; float b; if (3 == sscanf(sscanf_msg, "%d%*[ \t\r\n]%f %d %n", &a,&b,&c,&cch)) Do_(_) }
#define ifDFF(a,b,c,_)		{ int cch; int a; float b,c; if (3 == sscanf(sscanf_msg, "%d%*[ \t\r\n]%f %f %n", &a,&b,&c,&cch)) Do_(_) }
#define ifFDD(a,b,c,_)		{ int cch; int b,c; float a; if (3 == sscanf(sscanf_msg, "%f %d %d %n", &a,&b,&c,&cch)) Do_(_) }
#define ifFDF(a,b,c,_)		{ int cch; int b; float a,c; if (3 == sscanf(sscanf_msg, "%f %d%*[ \t\r\n]%f %n", &a,&b,&c,&cch)) Do_(_) }
#define ifFFD(a,b,c,_)		{ int cch; int c; float a,b; if (3 == sscanf(sscanf_msg, "%f %f %d %n", &a,&b,&c,&cch)) Do_(_) }

// all length-4 nonhomogeneous combinations of D and F

#define ifDDDF(a,b,c,d,_)	{ int cch; int a,b,c; float d; if (4 == sscanf(sscanf_msg, "%d %d %d%*[ \t\r\n]%f %n", &a,&b,&c,&d,&cch)) Do_(_) }
#define ifDDFD(a,b,c,d,_)	{ int cch; int a,b,d; float c; if (4 == sscanf(sscanf_msg, "%d %d%*[ \t\r\n]%f %d %n", &a,&b,&c,&d,&cch)) Do_(_) }
#define ifDDFF(a,b,c,d,_)	{ int cch; int a,b; float c,d; if (4 == sscanf(sscanf_msg, "%d %d%*[ \t\r\n]%f %f %n", &a,&b,&c,&d,&cch)) Do_(_) }
#define ifDFDD(a,b,c,d,_)	{ int cch; int a,c,d; float b; if (4 == sscanf(sscanf_msg, "%d%*[ \t\r\n]%f %d %d %n", &a,&b,&c,&d,&cch)) Do_(_) }
#define ifDFDF(a,b,c,d,_)	{ int cch; int a,c; float b,d; if (4 == sscanf(sscanf_msg, "%d%*[ \t\r\n]%f %d%*[ \t\r\n]%f %n", &a,&b,&c,&d,&cch)) Do_(_) }
#define ifDFFD(a,b,c,d,_)	{ int cch; int a,d; float b,c; if (4 == sscanf(sscanf_msg, "%d%*[ \t\r\n]%f %f %d %n", &a,&b,&c,&d,&cch)) Do_(_) }
#define ifDFFF(a,b,c,d,_)	{ int cch; int a; float b,c,d; if (4 == sscanf(sscanf_msg, "%d%*[ \t\r\n]%f %f %f %n", &a,&b,&c,&d,&cch)) Do_(_) }
#define ifFDDD(a,b,c,d,_)	{ int cch; int b,c,d; float a; if (4 == sscanf(sscanf_msg, "%f %d %d %d %n", &a,&b,&c,&d,&cch)) Do_(_) }
#define ifFDDF(a,b,c,d,_)	{ int cch; int b,c; float a,d; if (4 == sscanf(sscanf_msg, "%f %d %d%*[ \t\r\n]%f %n", &a,&b,&c,&d,&cch)) Do_(_) }
#define ifFDFD(a,b,c,d,_)	{ int cch; int b,d; float a,c; if (4 == sscanf(sscanf_msg, "%f %d%*[ \t\r\n]%f %d %n", &a,&b,&c,&d,&cch)) Do_(_) }
#define ifFDFF(a,b,c,d,_)	{ int cch; int b; float a,c,d; if (4 == sscanf(sscanf_msg, "%f %d%*[ \t\r\n]%f %f %n", &a,&b,&c,&d,&cch)) Do_(_) }
#define ifFFDD(a,b,c,d,_)	{ int cch; int c,d; float a,b; if (4 == sscanf(sscanf_msg, "%f %f %d %d %n", &a,&b,&c,&d,&cch)) Do_(_) }
#define ifFFDF(a,b,c,d,_)	{ int cch; int c; float a,b,d; if (4 == sscanf(sscanf_msg, "%f %f %d%*[ \t\r\n]%f %n", &a,&b,&c,&d,&cch)) Do_(_) }
#define ifFFFD(a,b,c,d,_)	{ int cch; int d; float a,b,c; if (4 == sscanf(sscanf_msg, "%f %f %f %d %n", &a,&b,&c,&d,&cch)) Do_(_) }

// a few length-5 nonhomogeneous combinations of D and F, as needed

#define ifDDFFF(a,b,c,d,e,_)	{ int cch; int a,b; float c,d,e; if (5 == sscanf(sscanf_msg, "%d%*[ \t\r\n]%d%*[ \t\r\n]%f %f %f %n", &a,&b,&c,&d,&e,&cch)) Do_(_) }

// Parsing messages with other messages as arguments inside them requires a call to ParseMunge().

#define ifM(b,_)			{ int cch; char b[1000]; if (1 == sscanf(sscanf_msg, "%[^\001]%n", b,&cch)) Do_(_) }
#define ifFM(a,b,_)			{ int cch; float a; char b[1000]; if (2 == sscanf(sscanf_msg, "%f %[^\001]%n", &a, b,&cch)) Do_(_) }
#define ifFFM(a,b,c,_)		{ int cch; float a,b; char c[1000];  if (3 == sscanf(sscanf_msg, "%f %f %[^\001]%n", &a, &b, c,&cch)) Do_(_) }
#define ifDFM(a,b,c,_)		{ int cch; int a; float b; char c[1000]; if (3 == sscanf(sscanf_msg, "%d%*[ \t\r\n]%f %[^\001]%n", &a, &b, c,&cch)) Do_(_) }
#define ifDDFM(a,b,c,d,_)	{ int cch; int a,b; float c; char d[1000]; if (4 == sscanf(sscanf_msg, "%d %d%*[ \t\r\n]%f %[^\001]%n", &a, &b, &c, d,&cch)) Do_(_) }

#define ifS(a,_)  			{ int cch; char a[1000]; if (1 == sscanf(sscanf_msg, "%s %n", a,&cch)) Do_(_) }
#define ifFS(a,b,_)  		{ int cch; float a; char b[1000]; if (2 == sscanf(sscanf_msg, "%f %s %n", &a, b,&cch)) Do_(_) }
#define ifSF(s,f,_)			{ int cch; char s[1000]; float f; if (2 == sscanf(sscanf_msg, "%s %f %n", s, &f,&cch)) Do_(_) }
#define ifSFF(s,f,g,_)		{ int cch; char s[1000]; float f,g; if (3 == sscanf(sscanf_msg, "%s %f %f %n", s, &f,&g,&cch)) Do_(_) }
#define ifSFFF(s,f,g,h,_)	{ int cch; char s[1000]; float f,g,h; if (4 == sscanf(sscanf_msg, "%s %f %f %f %n", s, &f,&g,&h,&cch)) Do_(_) }
#define ifSFFFF(s,f,g,h,i,_){ int cch; char s[1000]; float f,g,h,i; if (5 == sscanf(sscanf_msg, "%s %f %f %f %f %n", s, &f,&g,&h,&i,&cch)) Do_(_) }
#define ifDS(a,b,_)  		{ int cch; int a; char b[1000]; if (2 == sscanf(sscanf_msg, "%d %s %n",   &a, b,&cch)) Do_(_) }
#define ifSD(s,i,_)		  	{ int cch; int i; char s[1000]; if (2 == sscanf(sscanf_msg, "%s %d %n",   s, &i,&cch)) Do_(_) }
#define ifDSD(a,b,c,_)  		{ int cch; int a,c; char b[1000]; if (3 == sscanf(sscanf_msg, "%d %s %d %n",   &a, b, &c, &cch)) Do_(_) }
#define ifDDSF(i1,i2,s,f,_)	{ int cch; int i1,i2; char s[1000]; float f; if (4 == sscanf(sscanf_msg, "%d %d %s %f %n", &i1, &i2, s, &f,&cch)) Do_(_) }

#define ifSS(a,b,_) 		{ int cch; char a[500],b[500]; if (2 == sscanf(sscanf_msg, "%s %s %n", a,b,&cch)) Do_(_) }
#define ifSSS(a,b,c,_) 		{ int cch; char a[300],b[300],c[300]; if (3 == sscanf(sscanf_msg, "%s %s %s %n", a,b,c,&cch)) Do_(_) }
#define ifSSSS(a,b,c,d,_)	{ int cch; char a[250],b[250],c[250],d[250]; if (4 == sscanf(sscanf_msg, "%s %s %s %s %n", a,b,c,d,&cch)) Do_(_) }

#define ifSSF(a,b,f,_) 		{ int cch; char a[500], b[500]; float f; if (3 == sscanf(sscanf_msg, "%s %s %f %n", a,b,&f,&cch)) Do_(_) }
#define ifSSFF(a,b,f,g,_) 		{ int cch; char a[500],b[500]; float f,g; if (4 == sscanf(sscanf_msg, "%s %s %f %f %n", a,b,&f,&g,&cch)) Do_(_) }
#define ifSSFFF(a,b,f,g,h,_) 		{ int cch; char a[500],b[500]; float f,g,h; if (5 == sscanf(sscanf_msg, "%s %s %f %f %f %n", a,b,&f,&g,&h,&cch)) Do_(_) }

#define ifSSSF(a,b,c,f,_) 		{ int cch; char a[300],b[500],c[300]; float f; if (4 == sscanf(sscanf_msg, "%s %s %s %f %n", a,b,c,&f,&cch)) Do_(_) }
#define ifSSSFF(a,b,c,f,g,_) 		{ int cch; char a[300],b[300],c[300]; float f,g; if (5 == sscanf(sscanf_msg, "%s %s %s %f %f %n", a,b,c,&f,&g,&cch)) Do_(_) }
#define ifSSSFFF(a,b,c,f,g,h,_) 		{ int cch; char a[300],b[300],c[300]; float f,g,h; if (6 == sscanf(sscanf_msg, "%s %s %s %f %f %f %n", a,b,c,&f,&g,&h,&cch)) Do_(_) }

#define ifSSM(a,b,c,_) 		{ int cch; char a[300],b[300],c[500]; if (3 == sscanf(sscanf_msg, "%s %s %[^\001]%n", a,b,c,&cch)) Do_(_) }
#define ifSSSM(a,b,c,d,_) 		{ int cch; char a[300],b[300],c[300],d[500]; if (4 == sscanf(sscanf_msg, "%s %s %s %[^\001]%n", a,b,c,d,&cch)) Do_(_) }

// It's OK for array 'a' to vanish after an ifFAFAFA,
// because when modulate() passes it to VModulatorPool::insert,
// its contents are copied by VFloatArray's constructor.

#define MaxArrayLen (500)

#define ifIntArray(a, count, _) { \
	int a[MaxArrayLen]; int count = -1; \
	if (0 <= (count = SscanfInts(MaxArrayLen, a, sscanf_msg))) \
		Do_(_) }

#define ifFloatArray(a, count, _) { \
	float a[MaxArrayLen]; int count = -1; \
	if (0 <= (count = SscanfFloats(MaxArrayLen, a, sscanf_msg))) \
		Do_(_) }

#define ifFloatArrayFloat(a, count, f, _) { \
	float a[MaxArrayLen], f; int count = -1; \
	if (0 <= (count = SscanfFloats(MaxArrayLen, a, sscanf_msg)) && \
		1 == sscanf(strchr(sscanf_msg, ']')+1, "%f", &f)) \
		Do_(_) }

#define ScanArray(a) (SscanfFloats(MaxArrayLen, a, strchr(sscanf_msg, '[')))

#define ifFloatFloatArray(f, a, count, _) { \
	float f, a[MaxArrayLen]; int count = -1; \
	if (1 == sscanf(sscanf_msg, "%f [", &f) && \
		0 <= (count = ScanArray(a))) \
		Do_(_) }

#define ifIntFloatArray(b, a, count, _) { \
	int b; float a[MaxArrayLen], f; int count = -1; \
	if (1 == sscanf(sscanf_msg, "%d [", &b) && \
		0 <= (count = ScanArray(a))) \
		Do_(_) }

#define ifIntIntFloatArray(b, c, a, count, _) { \
	int b, c; float a[MaxArrayLen]; int count = -1; \
	if (2 == sscanf(sscanf_msg, "%d %d [", &b, &c) && \
		0 <= (count = ScanArray(a))) \
		Do_(_) }

#define ifFloatFloatArrayFloat(f, a, f2, count, _) { \
	float f, a[MaxArrayLen], f2; int count = -1; \
	if (1 == sscanf(sscanf_msg, "%f [", &f) && \
		0 <= (count = ScanArray(a)) && \
		1 == sscanf(strchr(sscanf_msg, ']')+1, "%f", &f2)) \
		Do_(_) }

extern "C" void CommandFromMessage(const char* message, int fGenActor=0);
