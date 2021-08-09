#include "basicActor.h"

ACTOR_SETUP(BasicActor, BasicActor)

extern void BASICinit(void);
extern int BASICstep(const char*);
extern void BASICterm(void);
extern void BASICflushoutput(void);
extern const char* BASICoutput(void);

BasicActor::BasicActor() :
	VActor(),
	fTerminated(0)
{
	setTypeName("BasicActor");
	BASICinit();
}

BasicActor::~BasicActor()
{
	BASICterm();
}

void BasicActor::act()
{
	VActor::act();
}

int BasicActor::preprocess(char* cmd)
{
	if (fTerminated)
		{
		fprintf(stderr, "vss error: BasicActor BASIC interpreter exited.\n");
		return 0;
		}

	if (isDebug())
		fprintf(stderr, "BASIC> %s\n", cmd);
	return 1;
}

extern int BASICfprintvss(void);
extern void BASICflushprintvss(void);

void BasicActor::command(const char* cmd)
{
	if (!BASICstep(cmd))
		fTerminated = 1;
	if (BASICfprintvss())
		{
		// Must make a local copy of BASICoutput(), because these commands
		// themselves could indirectly cause a printvss and call this function!
		// In other words, this function must be re-entrant.
		char sz[5001];
		strcpy(sz, BASICoutput());

		// Newlines delimit (actually, terminate) VSS commands.
		// Don't use strtok() because it's not reentrant.
		char* pchPrev = sz;
		char* pch = strchr(pchPrev, '\n');
		for (;  pch;  pch = strchr(pchPrev=pch+1, '\n'))
			{
			*pch = '\0';
			if (isDebug())
				fprintf(stderr, "BASIC_VSS> %s\n", pchPrev);
			actorMessageHandler(pchPrev);
			}
		BASICflushoutput();
		BASICflushprintvss();
		}
}

void BasicActor::type_in(char* cmd)
{
	if (!preprocess(cmd))
		return;
	command(cmd);
}

void BasicActor::type_and_get_floats(float hMG, char* cmd)
{
	if (!preprocess(cmd))
		return;

	BASICflushoutput();
	command(cmd);

	char szCmd[200];
	sprintf(szCmd, "SendData %f [%s]", hMG, BASICoutput());
	BASICflushoutput();
	actorMessageHandler(szCmd);
}

//===========================================================================
//		receiveMessage
//
int 
BasicActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("Do"))
	{
		ifM( msg, type_in(msg) );
		return Uncatch();
	}

	if (CommandIs("SendFloats"))
	{
		ifFM( hMG, msg, type_and_get_floats(hMG, msg) );
		return Uncatch();
	}

	return VActor::receiveMessage(Message);
}
