%{
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
static inline int isatty (int desc) { return 0; }

extern char vszFilterCommand[500];
extern int fFiltered;
extern int vfAbsorbText;

extern "C" int yylex(void);
extern "C" void yyerror(const char *msg);

#define MAX_ERRORS 1

#include "parseMsg.h"

void skipComment(void);
void PragmaFilter(void);

%}

Float		-?(([0-9]+\.?)|([0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?)
qstring		\"[^"\n]*["\n]				/* quoted string */
qstring2	\"[^"\n]*\"					/* quoted string, terminated */
string		[a-zA-Z0-9_\.+#%]+			/* non-quoted string */
wh			[\t\r\n ]+					/* whitespace */

%%

[\t\r\n, ]+	;							/* skip whitespace and commas */ 
"//pragma"[\t ]+filter[\t ]+{qstring2}	{ PragmaFilter(); return tPRAGMA; }
"//".*		;							/* skip C++ style comments */
"/*"		{ skipComment(); }			/* skip C style comments */

{Float}		{ strcpy(yylval.string, yytext); return tNUMBER; }

"{"			{ return tOPEN_BRACE; }
"}"			{ return tCLOSE_BRACE; }
"["			{ return tOPEN_SQB; }
"]"			{ return tCLOSE_SQB; }
"="			{ return tEQUAL; }
":"			{ fprintf(stderr, "vss client warning: you mean ';' not ':'\n"); return tSEMICOLON; }
";"			{ return tSEMICOLON; }
"to"		{ return tTO; }
"@"			{ return tAT; }
"*"			{ return tSTAR; }
"?"			{ return tQMARK; }
"$"			{ return tDOLLAR; }
\\\"		{ return tESCAPEDDOUBLEQUOTE; }

"ClientSleep"[\t\r\n ]+{Float} { strcpy(yylval.string, yytext); return tSLEEP; }
[sS]"leep"[\t\r\n ]+{Float}	 { strcpy(yylval.string, yytext); return tSLEEP; }

ClientSetTimeout[\t\r\n ]+{Float} { strcpy(yylval.string, yytext); return tTIMEOUT; }

ClientPrint[\t\r\n ]+{qstring2} { strcpy(yylval.string, yytext); return tPRINT; }

{qstring}	{
		memcpy(yylval.string, yytext+1, yyleng-1);
		if(yylval.string[yyleng-2] != '"')
			fprintf(stderr, "vss client error: Unterminated character string\n");
		else
			yylval.string[yyleng-2] = '\0';	/* remove close quote */
		return tSTRING;
	}

Create		{ yylval.actorMessage = 1; return tCREATE_MESSAGE; }
Delete		{ yylval.actorMessage = 2; return tDELETE_MESSAGE; }
BeginNote		{ yylval.actorMessage = 3; fprintf(stderr, "vss client warning: you mean BeginSound not BeginNote\n"); return tBEGIN_NOTE_MESSAGE; }
BeginSound		{ yylval.actorMessage = 3; return tBEGIN_NOTE_MESSAGE; }
BeginNotePaused		{ yylval.actorMessage = 4; fprintf(stderr, "vss client warning: you mean BeginSoundPaused not BeginNotePaused\n"); return tBEGIN_PAUSED_NOTE_MESSAGE; }
BeginSoundPaused		{ yylval.actorMessage = 4; return tBEGIN_PAUSED_NOTE_MESSAGE; }
{string}	{ strcpy(yylval.string, yytext); return tSTRING; }

.			{ return yytext[0]; }

%%

void PragmaFilter()
{
	strcpy(yylval.string, "");
	/* Do this not more than once. */
	if (*vszFilterCommand)
		return;

	char sz[500];
	strcpy(sz, yytext);
	sz[yyleng-1] = '\0';
	strcpy(vszFilterCommand, strchr(sz, '"') + 1);
}

void yyerror(const char *msg)
{
	extern int numErrors;
	extern char* curFileName;

	// Report no errors as the result of a filtering.
	if (vszFilterCommand && !fFiltered)
		return;

	// Dereferencing curFileName[] causes a coredump, inexplicably.
	fprintf(stderr, "vss client error: File %s, line %d: %s at \"%s\"",
		curFileName && *curFileName ? curFileName : "-",
		yylineno+1, msg, yytext);
	fprintf(stderr, "vss client error: Line %d: %s at \"%s\"",
		yylineno+1, msg, yytext);
	fprintf(stderr, ++numErrors >= MAX_ERRORS ? ", giving up.\n" : "\n");
}

#ifdef VSS_LINUX
#define input() yyinput()
#endif

void skipComment()
{
	char c;
	for(;;)	{
		while((c=yyinput())!='*')
			;
		if((c=yyinput())=='/')
			return;
		else	{
			unput(c);
		}
	}
}
