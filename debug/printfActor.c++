#include "printfActor.h"

ACTOR_SETUP(PrintfActor, PrintfActor)

PrintfActor::PrintfActor() :
	color(nil),
	file(stderr)
{
	setTypeName("PrintfActor");
}

PrintfActor::~PrintfActor()
{
	if (file != stderr)
		fclose(file);
}

static const char* rgszColor[9] =
	{ "nil",
	  "black", "red", "green", "yellow", "blue", "magenta", "cyan", "white" };

void PrintfActor::setColor(const char* sz)
{
	for (int i=0; i<9; i++)
		{
		if (!strcmp(sz, rgszColor[i]))
			{
			setColor(i-1);
			return;
			}
		}
	fprintf(stderr, "vss warning: PrintfActor SetColor unrecognized (%s)\n",
		sz);
	setColor(nil);
}

void PrintfActor::setColor(int w)
{
	if (w < -1 || w > white)
		{
		fprintf(stderr, "vss warning: PrintfActor SetColor out of range (%d)\n",
			w);
		w = nil;
		}
	color = w;
}

void PrintfActor::setFile(const char* sz)
{
	file = fopen(sz, "w");
	if (!file)
		{
		fprintf(stderr,
			"vss error: PrintfActor couldn't write to file \"%s\"\n",
			sz);
		file = stderr;
		}
	else
		{
		fprintf(stderr,
			"vss remark: PrintfActor redirected to file \"%s\"\n",
			sz);
		}
}

// Convert '_' to ' ', and trailing "\n" to newline, in place.
// Then append a color-reset string if needed.
char* PrintfActor::munch(char* sz) const
{
	for (char* p = sz; *p; p++)
		{
		if (*p == '_')
			*p = ' ';
		if (!strcmp(p, "\\n"))
			{
			strcpy(p, "\n");
			break;
			}
		}

	// We've got room (1000 characters, from def'n of ifSFFFF etc.)
	// to tack this on the end.
	if (color != nil)
		strcat(sz, "\033[0m"); // reset to default

	return sz;
}

int 
PrintfActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("printf"))
		{
		if (color != nil && file == stderr)
			fprintf(stderr, "\033[3%dm", color); // set text color (4%d is page color, fyi)

		ifSFFFF( s,f1,f2,f3,f4, flush(fprintf(file, munch(s), f1,f2,f3,f4)) );

		ifFFFF( f1,f2,f3,f4, flush(fprintf(file, "%g %g %g %g\n", f1,f2,f3,f4)) );
		ifSFFF( s,f1,f2,f3, flush(fprintf(file, munch(s), f1,f2,f3)) );
		ifFFF( f1,f2,f3, flush(fprintf(file, "%g %g %g\n", f1,f2,f3)) );
		ifSFF( s,f1,f2, flush(fprintf(file, munch(s), f1,f2)) );
		ifFF( f1,f2, flush(fprintf(file, "%g %g\n", f1,f2)) );
		ifSF( s,f1, flush(fprintf(file, munch(s), f1)) );
		ifF( f1, flush(fprintf(file, "%g\n", f1)) );
		ifS( s, flush(fprintf(file, "%s", munch(s))) );
		return Uncatch();
		}

	if (CommandIs("SetColor"))
		{
		ifD( w, setColor(w) );
		ifS( s, setColor(s) );
		ifNil( setColor(nil) );
		}

	if (CommandIs("SetFile"))
		{
		ifS( s, setFile(s) );
		return Uncatch();
		}

	return VActor::receiveMessage(Message);
}
