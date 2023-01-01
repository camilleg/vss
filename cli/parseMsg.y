%{
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <unistd.h>

extern "C" {
const auto MAX_ERRORS = 1;

int     AUDinit(const char *);
float    AUDupdate(int, char *, int, float *);
float    AUDupdateSimple(char *, int, float *);
float    AUDupdateFloats(int, char *, int, ...);
float    AUDupdateSimpleFloats(char *, int, ...);
void    AUDterminate(int);
void	AUDqueue(int, float *, float);
void	AUDflushQueue(int, char*, int);
void	SetReplyTimeout(float);

int yylex();
extern "C" void yyerror(const char*);
}

#include "symtab.h"

#define	fdMax	100		// Max times a single app can call init
#include "actorMsg.h"
#include "cliMsg.h"

int findActorName(int, char*);
int findNoteName(char*);
void AppendArg(const char*);
void AppendFloat(float);

char curFileName[2000], prevFileName[2000]; // Static storage duration, therefore zero initialized.
extern FILE		*yyin;
int numErrors;
static Symtab symtabActor[fdMax];
static Symtab symtabNote[fdMax];
static int		serverHandles[fdMax];
static char		theArgs[1500];
static float	vhActor;
static char vszhnote[200];
static float vhNote;
static int		argCount;
static int		fd = 0;		// like a "file descriptor"
char*			typeptr;
extern int		currentServerHandle;	// in cliMsg.c
static int		vfPaused;
static int 		vfEnableWarnNoMG = 1;
int fFiltered = 0; // not static
char vszFilterCommand[500];
int vfAbsorbText = 0;

typedef struct
{
	float time;
	int	  size;
	float *farray;
}	audqdata;
#define	qdMax	32	//	Max times a client can call AUDqueue
					//	between calls to AUDflush
static audqdata Qdata[qdMax];
static int 		QdataCount = 0;

extern "C" int yywrap() { return 1; }

extern "C" void AUDEnableNoMessageGroupWarning(int f)
	{ vfEnableWarnNoMG = f; }
%}

%union	{
	char			string[512];
	int		actorMessage;	//xx30 only for vss-builtin msgs
}

%token <string> tOPEN_BRACE tCLOSE_BRACE tSTAR tDOLLAR tQMARK tEQUAL tOPEN_SQB tCLOSE_SQB tSEMICOLON tTO tERROR tSLEEP tAT tTIMEOUT tPRINT tESCAPEDDOUBLEQUOTE
%token <string> tSTRING tNUMBER tPRAGMA
%token <actorMessage> tCREATE_MESSAGE tDELETE_MESSAGE tBEGIN_NOTE_MESSAGE tBEGIN_PAUSED_NOTE_MESSAGE

%type <string> Float NormalFloat NormalFloatArg BeginNoteBody Handle
%type <actorMessage> aBeginNoteMessage

%%

Messages:	tPRAGMA
				{ if (!fFiltered && *vszFilterCommand) { fclose(yyin); yyin = fopen("/dev/null", "r"); vfAbsorbText = 1; } }
	|	MessageSemicolon
	| Messages MessageSemicolon;
MessageSemicolon: Message tSEMICOLON;

Message:	tSTRING tEQUAL tCREATE_MESSAGE tSTRING {
			if (!vfAbsorbText && !symtabActor[fd].FAdd($1, createActor($4)))
				{
				fprintf(stderr, "vss client error: Command \"%s = Create %s\":\n", $1, $4);
				yyerror("Actor name already used, or too many actors in this AUDinit() call.");
				}
		//	else
		//		{
		//		printf("added a guy %s\n", $4);
		//		symtabActor[fd].Dump();
		//		}
		}
	|	BeginNoteBody { *vszhnote = '\0'; }
	|	tSTRING tEQUAL { strcpy(vszhnote, $1); /*printf("bgnnote lvalue is \"%s\"\n", vszhnote);*/ } BeginNoteBody

	|	tDELETE_MESSAGE tSTRING {
				float h = symtabActor[fd].HFromSz($2); /* grab it before we delete */
				if (!vfAbsorbText && (h < 0. || !symtabActor[fd].FDelete($2)))
					{
					h = symtabNote[fd].HFromSz($2);
					if (h < 0. || !symtabNote[fd].FDelete($2))
						{
						fprintf(stderr, "vss client error: Delete: undefined actor \"%s\"\n", $2);
						yyerror("");
						goto LDeleteFinished;
						}
					}
				{
				char szT[1000];
				sprintf(szT, "Delete %f", h);
				if (!vfAbsorbText)
					actorMessage(szT);
				}
			LDeleteFinished: ;
			}
	|   tSTRING {
		if (!vfAbsorbText && symtabActor[fd].FFound($1))
			{
			fprintf(stderr, "vss client error: Syntax error: actor %s\n", $1);
			yyerror("");
			}
		else if (!vfAbsorbText && symtabNote[fd].FFound($1))
			{
			fprintf(stderr, "vss client error: Syntax error: handler %s\n", $1);
			yyerror("");
			}
		else
			{
			/*printf("starting a generic msg \"%s\"\n", $1);*/
			strcpy(theArgs, $1);
			strcat(theArgs, " ");
			}
		}
		Args {
			if(numErrors < MAX_ERRORS)
				{
				if (!vfAbsorbText)
					actorMessage(theArgs);
				}
		}
	|	tSLEEP {
			float z = 0;
			if (!vfAbsorbText && 1 == sscanf($1, "%*s %f", &z) && z > 0. && z < 24*3600.)
				{
				usleep((long)(z * 1000000));
				}
			}
	|	tTIMEOUT {
			float z = 0;
			if (!vfAbsorbText && 1 == sscanf($1, "%*s %f", &z))
				{
				if (z > 0. && z <= 3600.)
					SetReplyTimeout(z);
				else
					fprintf(stderr, "VSS client error: ignoring bogus ClientSetTimeout value %g\n", z);
				}
			}
	|	tPRINT {
			char* pch = strchr($1, '"') + 1;
			*strchr(pch, '"') = '\0'; // erase closing double quote
			if (!vfAbsorbText)
				fprintf(stderr, "%s\n", pch);
			}

	;

aBeginNoteMessage:
		tBEGIN_NOTE_MESSAGE
			{ $$ = $1; vfPaused = 0; }
	|	tBEGIN_PAUSED_NOTE_MESSAGE
			{ $$ = $1; vfPaused = 1; }
	;

BeginNoteBody: aBeginNoteMessage tSTRING {
				if(!vfAbsorbText && numErrors < MAX_ERRORS)
					{
					/* assumption: BeginNoteBody can't be nested. */
					vhActor = symtabActor[fd].HFromSz($2);
					if (vhActor < 0)
						vhActor = symtabNote[fd].HFromSz($2);
					if (vhActor < 0)
						{
						fprintf(stderr, "vss client error: Actor \"%s\" not defined\n", $2);
						yyerror("Undefined actor in .aud file");
						*vszhnote = '\0';
						vhNote = -1;
						}
				sprintf(theArgs, "%s %f ",
					vfPaused ? "BeginSoundPaused" : "BeginSound",
					/*doesn't work... $1==tBEGIN_NOTE_MESSAGE ? "BeginSound" : "BeginSoundPaused", */
					vhActor);
					}
				}
			Args {
				if(!vfAbsorbText && numErrors < MAX_ERRORS)
					{
					if (vszhnote[0])
						vhNote = actorMessageRetval(theArgs);
					else
						{
						vhNote = -1;
						actorMessage(theArgs);
						}

					/* add (hNote,vszhnote) to symbol table. if (!*vszhnote), overwrite prev "" value. */
					if (*vszhnote && !symtabNote[fd].FAdd(vszhnote, vhNote))
						fprintf(stderr, "VSS client error: BeginSound failed during AUDinit\n\t(duplicate handle name \"%s\" for BeginSound?).\n", vszhnote);
					/* clear it, just in case */
					*vszhnote = '\0';
					vhNote = -1;
					}
				};

Args:	/* nothing */		/* this parsing is only for diagnostics.  Strings are passed on to actors for them to sscanf-parse. */
	|   Args Arg
	;

Arg:	Float
	|	tCREATE_MESSAGE { AppendArg("Create "); }
	|	tDELETE_MESSAGE { AppendArg("Delete "); }
	|	tBEGIN_NOTE_MESSAGE { AppendArg("BeginSound "); }
	|	tBEGIN_PAUSED_NOTE_MESSAGE { AppendArg("BeginSoundPaused "); }
	|	Farray
	|	DoubleQuote
	;

Farray:	AtsMaybe RawArray;

RawArray:
		tOPEN_SQB		{ AppendArg("["); }
		Floats
		tCLOSE_SQB		{ AppendArg("]"); }
	|	tOPEN_SQB tCLOSE_SQB { if (!vfAbsorbText) fprintf(stderr, "VSS client warning: empty array.\n"); }
		;

NormalFloat:	tNUMBER	{ /*;;needed?*/ strcpy($$, $1); }
	;

NormalFloatArg:	NormalFloat	{ AppendArg($1); AppendArg(" "); }
	;

Star: tSTAR
	{ AppendArg("*"); }
	;

Dollar: tDOLLAR
	{ AppendArg("$"); }
	;

AtsMaybe: Ats | /**/;

Ats	: At
	| Ats At
	;

At: tAT
	{ AppendArg("@"); }
	;

To: tTO
	{ AppendArg(" to "); }
	;

DoubleQuote: tESCAPEDDOUBLEQUOTE
	{ AppendArg("\""); }
	;

Handle: tSTRING	{
			/* Substitute handles for names, of actors and notes.  Otherwise pass the string through. */
			float h;
			if ((h = symtabActor[fd].HFromSz($1)) >= 0.)
				/* print actor's handle */
				AppendFloat(h);
			else if ((h = symtabNote[fd].HFromSz($1)) >= 0.)
				/* print note's handle */
				AppendFloat(h);
			else
				AppendArg($1);
			AppendArg(" ");
			}
	;

Float:           NormalFloatArg
	|	Star     NormalFloatArg { /*;;needed?*/ strcpy($$, $2); }
	|	Ats      NormalFloatArg { /*;;needed?*/ strcpy($$, $2); }
	|	Ats Star NormalFloatArg { /*;;needed?*/ strcpy($$, $3); }
	|	tSTAR tQMARK { AppendArg("*? "); }
	|	Handle
	;

Floats:	Float
	|	Floats Float
	|	Star NormalFloatArg To Star NormalFloatArg
			{ /* variable-length array */ }
	|	Star NormalFloatArg To Star Dollar
			{ /* variable-length array, to "**" end of array */ }
	;

%%

void AppendArg(const char* sz)
{
	strcat(theArgs, sz);
}

void AppendFloat(float z)
{
	char sz[20];
	sprintf(sz, "%f", z);
	AppendArg(sz);
}

extern int yylineno;
int hAudSimple = -1;

extern "C" int AUDinit(const char *fileName)
{
	if (currentServerHandle < 1)
		{
		fprintf(stderr, "vss client error: not connected to sound server.  Ignoring .aud file '%s'.\n", fileName);
		return -1;
		}

	if (fd >= fdMax)
		{
		fprintf(stderr, "vss client error:  AUDinit() more than %d times without AUDterminate()'ing old ones first.  Ignoring .aud file '%s'.\n", fdMax, fileName);
		return -1;
		}

	*vszFilterCommand = '\0';
	fFiltered = 0;
	vfAbsorbText = 0;

	yyin = fopen(fileName, "r");
	if (!yyin)
		{
		fprintf(stderr, "vss client error: can't read .aud file '%s'.\n", fileName);
		return -1;
		}
	if (!strcmp(fileName, prevFileName))
		{
		fprintf(stderr, "vss client warning: loading .aud file '%s' twice in a row.\n", fileName);
		}

LAgain:
	strcpy(curFileName, fileName);
	strcpy(prevFileName, fileName);
	symtabActor[fd].Reset();
	symtabNote[fd].Reset();
	numErrors = 0;
	yylineno = 0;

	while (!feof(yyin) && numErrors <= MAX_ERRORS)
		{
		argCount = 0;
		int fOK = yyparse() == 0;
		int fGonnaFilter = !fFiltered && *vszFilterCommand;
		if (!fOK && !fGonnaFilter)
			{
			if (yylineno <= 0)
				fprintf(stderr, "vss client warning: possibly empty .aud file '%s'.\n", fileName);
#if 0 // lexMsg.l's yyerror() already complained.
			else
				fprintf(stderr, "vss client error: syntax error in .aud file '%s', line %d\n", fileName, yylineno);
#endif
			}
		if (fGonnaFilter)
			{
			fFiltered = 1;
			fclose(yyin);
			// Munch fileName into a temp file.
			// The muncher, vszFilterCommand, comes from //pragma in the .aud file,
			// and is often /usr/lib/cpp -P, sometimes then piped through sed.
			char szCmd[1500];
			unlink("/tmp/_-vss-_FiLtEr_--_");
			sprintf(szCmd, "cat %s | %s > /tmp/_-vss-_FiLtEr_--_",
				fileName, vszFilterCommand);
			const int r = system(szCmd);
			if (r != 0) {
			  fprintf(stderr, "vss client error: filter '%s' failed on .aud file '%s'.", vszFilterCommand, fileName);
			  unlink("/tmp/_-vss-_FiLtEr_--_"); // Just in case.
			  return -1;
			}
			/*system("cp /tmp/_-vss-_FiLtEr_--_ /tmp/asdf");*/
			yyin = fopen("/tmp/_-vss-_FiLtEr_--_", "r");
			vfAbsorbText = 0;
			goto LAgain; // real programmers aren't afraid to use goto's
			}
		fFiltered = 1; // pragma filter has to be first in the .aud file.
		}
	fclose(yyin);
	unlink("/tmp/_-vss-_FiLtEr_--_");
	if (numErrors > MAX_ERRORS)
		{
		fprintf(stderr, "vss client error: too many errors with .aud file '%s'.\n", fileName);
		return -1;
		}

	// If the aud file was loaded successfully, remember the current server handle.
	serverHandles[fd] = currentServerHandle;
	hAudSimple = fd;
	return fd++;	 	// return the row # of actors allocated
}

static bool vfHush = false;
extern "C" void AUDterminateImplicit() {
	if (hAudSimple >= 0) {
		vfHush = true;
		AUDterminate(hAudSimple);
		vfHush = false;
	}
}

extern "C" void AUDterminate(int fdT)
{
	if (fdT < 0) {
		if (!vfHush)
			fprintf(stderr, "vss client error: invalid handle to .aud file.  Possible error in .aud file.\n");
		return;
	}
	if (serverHandles[fdT] <= 0) {
		if (!vfHush)
			fprintf(stderr, "vss client error: bogus AUDterminate().\n");
		return;
	}

	const auto sendEm = SelectSoundServer(serverHandles[fdT]);
	if (fdT < 0 || fdT >= fdMax) {
		if (!vfHush)
			fprintf(stderr, "vss client error: bogus AUDterminate(%d).\n", fdT);
		return;
	}
	auto& sa = symtabActor[fdT];
	if (sendEm) {
		const auto handles = sa.Handles();
		char szT[500];
		// Inactivate all actors, so they can't send audEvents,
		// in particular to a deleted actor.
		for (auto h: handles) {
			sprintf(szT, "Active %f 0", h);
			actorMessage(szT);
		}
		// Now it's safe to delete all actors.  Order doesn't matter.
		for (auto h: handles) {
			sprintf(szT, "Delete %f", h);
			actorMessage(szT);
		}
	}
	sa.Reset();

	// Clear this server handle.
	serverHandles[fdT] = 0;

	// Reuse array, if the order of AUDterminates matches that of AUDinits.
	if (fdT == fd-1)
		--fd;

	// Suppress spurious warning.
	*prevFileName = '\0';
}

//	Terminate all of this client's server connections.
extern "C" void AUDreset()
{
	for (int fdT = 0; fdT < fdMax; ++fdT)
		if (serverHandles[fdT] > 0)
			AUDterminate(fdT);
	fd = 0;
	QdataCount = 0;
}

extern "C" float AUDupdateSimple(char* szActor, int numFloats, float* floatArray)
{
	if (hAudSimple >= 0)
		return AUDupdate(hAudSimple, szActor, numFloats, floatArray);

	fprintf(stderr, "vss client error: AUDupdateSimple() without preceding AUDinit().\n");
	return hNil;
}

float AUDupdateSimpleFloats(char* szActor, int numFloats, ...)
{
	if (hAudSimple < 0) {
		fprintf(stderr, "vss client error: AUDupdateSimpleFloats() without preceding AUDinit().\n");
		return hNil;
	}

	// Build an array and then pass it to the conventional AUDupdate().
	const int numFloatsMax = 1000;
	static float rgz[numFloatsMax]; // not re-entrant, but that's OK.
	if (numFloats > numFloatsMax) {
		fprintf(stderr, "VSS client warning: AUDupdateFloats() truncated list of %d floats to %d.\n",
			numFloats, numFloatsMax);
		numFloats = numFloatsMax;
	}
	va_list _;
	va_start(_, numFloats);
	for (int i=0; i<numFloats; ++i)
		rgz[i] = (float)va_arg(_, double);
	va_end(_);
	return AUDupdate(hAudSimple, szActor, numFloats, rgz);
}

float AUDupdateFloats(int fdT, char* szActor, int numFloats, ...)
{
	// Build an array and then pass it to the conventional AUDupdate().

	const int numFloatsMax = 1000;
	static float rgz[numFloatsMax]; // not re-entrant, but that's OK.
	if (numFloats > numFloatsMax) {
		fprintf(stderr, "VSS client warning: AUDupdateFloats() truncated list of %d floats to %d.\n",
			numFloats, numFloatsMax);
		numFloats = numFloatsMax;
	}
	va_list _;
	va_start(_, numFloats);
	for (int i=0; i<numFloats; ++i)
		rgz[i] = (float)va_arg(_, double);
	va_end(_);
	return AUDupdate(fdT, szActor, numFloats, rgz);
}

extern "C" void VSS_StripZerosInPlace(char*); // in cliMsg.c

// Send a data array to the message group called szActor at server fdT.
// Complain if there is no such message group at fdT.
extern "C" float AUDupdate(int fdT, char* szActor, int numFloats, float* floatArray)
{
	if (fdT < 0) {
		fprintf(stderr, "vss client error: invalid handle to .aud file.  Possible error in .aud file.\n");
		return hNil;
	}

	if (!SelectSoundServer(serverHandles[fdT]))
		return hNil;

	const auto h = symtabActor[fdT].HFromSz(szActor);
	if (h < 0.0) {
		if (vfEnableWarnNoMG) {
			fprintf(stderr, "vss client error: No message group called '%s'.\n",  szActor);
			symtabActor[fdT].Dump();
		}
		return hNil;
	}

	char szT[5000];
	sprintf(szT, "SendData %f [ ", h);
	for (int i=0; i<numFloats; ++i) {
		char szT2[20];
		sprintf(szT2, "%f ", floatArray[i]);
		strcat(szT, szT2);
	}
	strcat(szT, "]");
	VSS_StripZerosInPlace(szT);
	return actorMessageRetval(szT);
}

// Call AUDupdate for two servers, e.g. for wet and dry localization.
extern "C" void AUDupdateTwo(int vss1, int vss2, char* szActor, int size, float* array)
{
	if (SelectSoundServer(serverHandles[vss1]))
		(void)AUDupdate(vss1, szActor, size, array);
	if (SelectSoundServer(serverHandles[vss2]))
		(void)AUDupdate(vss2, szActor, size, array);
}

// Call AUDupdate for an array of server handles.
extern "C" void AUDupdateMany(int numHandles, int* handleArray, char* actorHandleName, int numFloats, float* floatArray)
{
	for (int i = 0; i < numHandles; ++i) {
		const int fdT = handleArray[i];
		if (SelectSoundServer(serverHandles[fdT]))
			(void)AUDupdate(fdT, actorHandleName, numFloats, floatArray);
	}
}

//	Queue up an array of data to send to a message group with
//	ScheduleData whenever AUDflushQueue is finally called. Complain
//	if AUDqueue is called more than qdMax times before AUDflushQueue
//	is called.
extern "C" void AUDqueue(int size, float* data, float when)
{
	if (QdataCount >= qdMax)
	{
		fprintf(stderr, "vss client error: too much data queued. Flushing needed.\n");
		return;
	}

	Qdata[QdataCount].time = when;
	Qdata[QdataCount].size = size;
// printf("\t\tqueueing %d at %f: ", size, when);
	Qdata[QdataCount].farray = (float *)malloc(size * sizeof(float));
	if (NULL == Qdata[QdataCount].farray)
	{
		fprintf(stderr, "vss client error: out of memory trying to queue data.\n");
		return;
	}

	for (int i=0; i<size; ++i)
	{
		Qdata[QdataCount].farray[i] = data[i];
// printf("%f ", data[i]);
	}
// printf("\n\n");
	++QdataCount;
}

// Send queued data to the message group szActor, at server fdT, using ScheduleData.
// Complain if actor is called szActor.
extern "C" void AUDflushQueue(int fdT, char* szActor, int fPreserveQueueData)
{
	if (fdT < 0) {
		fprintf(stderr, "vss client error: invalid handle to .aud file.  Possible error in .aud file.\n");
		return;
	  }
    if (!SelectSoundServer(serverHandles[fdT]))
		return;

    const auto h = symtabActor[fdT].HFromSz(szActor);
    if (h < 0.0) {
		if (vfEnableWarnNoMG) {
			fprintf(stderr, "vss client error: No message group called '%s'.\n", szActor);
        	symtabActor[fdT].Dump();
		}
        return;
	}

// printf("AUDflushQueue flushing %d\n", QdataCount);
	// Start the message.
    char szT[5000];
	sprintf(szT, "ScheduleData %f [ ", h);

	// Build the time offset array.
	int cqd;
	char szT2[50];
	for (cqd = 0; cqd < QdataCount; ++cqd) {
		sprintf(szT2, "%f ", Qdata[cqd].time);
		strcat(szT, szT2);
	}
	strcat(szT, "]");

	// Add the data arrays.
	for (cqd = 0; cqd < QdataCount; ++cqd) {
		strcat(szT, " [");
		for (int cdata = 0; cdata < Qdata[cqd].size; ++cdata)
		{
			sprintf(szT2, "%f ", Qdata[cqd].farray[cdata]);
			strcat(szT, szT2);
		}
		strcat(szT, "]");
	}

	// Send it to VSS.
    actorMessage(szT);

	if (!fPreserveQueueData)
		QdataCount = 0;
}

// vim: ts=4 sw=4
