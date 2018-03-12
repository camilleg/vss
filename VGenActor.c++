//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1997.
//===========================================================================

#include "VGenActor.h"
#include "VHandler.h"
#include "VAlgorithm.h" // just for ScalarFromdB()
#include <ctype.h> // for isalpha()

//===========================================================================
//      constructor
//
VGeneratorActor::VGeneratorActor(void):
	VActor(),
	zAmpl(1.),
	zScaleAmp(1.),
	pan(0.),
	elev(0.),
	distance(0.),					// cave coords
	distanceHorizon(70.),			// cave coords
	xPos(0.), yPos(5.3), zPos(0.),	// cave coords
	fLinearEnv(0),
	fDying(false)
{
}

//===========================================================================
//      destructor
//
VGeneratorActor::~VGeneratorActor()
{
  	fDying = true; // Prevent removeChild from updating children, which would invalidate our HandlerListIterator (2011).
	for (HandlerListIterator<VHandler> it = children.begin(); it != children.end(); it++)
		if (*it) {
		  delete *it;
		}
}

//===========================================================================
//      addChild
//
//	When a child is created a a result of a BeginSound message being received, 
//	newHandler() is called, and if it returns successfully, the new child 
//	(handler) is added to this generator actor's list of progeny using 
//	addChild().
//
void
VGeneratorActor::addChild(VHandler * newGuy)
{
	pair< HandlerList::iterator, bool > result = children.insert( newGuy );
	if ( !result.second )
		fprintf(stderr, "vss internal error: VGeneratorActor::addChild() failed.\n\n");
	newGuy->setParent( handle() );
}

//===========================================================================
//		removeChild
//
//  When a handler is deleted, it informs the parent generator actor so 
//  that the latter's children list can be kept up to date.
//
void
VGeneratorActor::removeChild(VHandler * h)
{
	if (fDying)
		return;
//  if the child is found in the list of children (and
//  it had better be), remove it from the list.
	HandlerListIterator< VHandler > it = children.find( h );
	if ( it == children.end() )
		fprintf(stderr, "vss internal error: VGeneratorActor::removeChild() failed.\n\n");
	else
		children.erase( it );
}

//===========================================================================
//		sendDefaults
//
//	Derived classes that have parameters of their own should override
//	this member to send the default values to a handler instance.
//	DON'T FORGET TO CALL THE PARENT'S SENDDEFAULTS() SO THAT INHERITED
//	DEFAULTS ARE ALSO SENT.
//
void
VGeneratorActor::sendDefaults(VHandler * h)
{
	if (!h)
		{
		fprintf(stderr, "vss error: VGeneratorActor::sendDefaults failing, null VHandler*\n");
		return;
		}
	if (!h->getAlg())
		{
		// getAlg() already printed something when it returned NULL.
		fprintf(stderr, "\tVGeneratorActor::sendDefaults failing because of NULL getAlg().\n");
		return;
		}

	h->setMute(false);
	h->setLinear(fLinearEnv);

	// Start at zero, immediately.
	h->setPause(true);
	h->setAmp(0.);
	h->scaleAmp(0.);

	h->setPan(pan);
	h->setElev(elev);
	h->setDistance(distance);
	h->setDistanceHorizon(distanceHorizon);

	h->setPause(false);		// so the notes don't "click" on, if dampingTime>0.
	h->setAmp(zAmpl);
	h->scaleAmp(zScaleAmp);

}

//===========================================================================
//		receiveMessage
//
//	Derived classes should override this member if they receive messages.
//	DON'T FORGET TO CALL THE PARENT'S RECEIVEMESSAGE() FOR MESSAGES YOU 
//	DON'T HANDLE.
//

int
VGeneratorActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

//	handler creation

	if (CommandIs("BeginNote"))
	{
		fprintf(stderr, "vss: you mean BeginSound not BeginNote.\n");
		strcpy(sscanf_cmd, "BeginSound");
		goto LBeginNote;
	}

	if (CommandIs("BeginNotePaused"))
	{
		fprintf(stderr, "vss: you mean BeginSoundPaused not BeginNotePaused.\n");
		strcpy(sscanf_cmd, "BeginSoundPaused");
		goto LBeginNote;
	}

	if (CommandIs("BeginSound") || CommandIs("BeginSoundPaused"))
	{
LBeginNote:
		VHandler * phandler = newHandler();
		if (phandler == NULL)
		{
			printf("vss internal error: generator actor couldn't create a new handler.\n");
			return Catch();
		}

		addChild( phandler );
		sendDefaults( phandler );

		//	save this because CommandIs() doesn't seem
		//	to work after parseInitializers().
		int fPause = CommandIs("BeginSoundPaused");
		phandler->setPause(1);
		parseInitializers(sscanf_msg, phandler);
		phandler->setPause(fPause);

		// Return handle to client.
		ReturnFloatToClient(phandler->handle());
		return Catch();
	}

	if (CommandIs("InvertAmp"))
	{
		ifD( f, invertAmp(f) );
		return Uncatch();
	}

	if (CommandIs("InvertAllAmp"))
	{
		ifD( f, invertAllAmp(f));
		return Uncatch();
	}

	if (CommandIs("SetInputAmp"))
	{
		ifF( z, setInputAmp(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllInputAmp"))
	{
		ifFF( z, time, setAllInputAmp(z, time));
		ifF( z, setAllInputAmp(z));
		return Uncatch();
	}

	if (CommandIs("SetInputGain"))
	{
		ifF( z, setInputGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllInputGain"))
	{
		ifFF( z, time, setAllInputGain(z, time));
		ifF( z, setAllInputGain(z));
		return Uncatch();
	}

	if (CommandIs("SetAmpl"))
	{
		fprintf(stderr, "vss: I assume you meant SetAmp not SetAmpl.\n");
		goto LSetAmp;
	}

	if (CommandIs("ScaleAmp"))
	{
		ifF( z, scaleAmp(z) );
		ifFF( z, time,
			fprintf(stderr, "vss error: scaleAmp with duration works only with handlers, not actors such as %s:\n\t\"%s\"",
				typeName(), Message) );
		return Uncatch();
	}

	if (CommandIs("ScaleAllAmp"))
	{
		ifFF( z, time, scaleAllAmp(z, time));
		ifF( z, scaleAllAmp(z));
		return Uncatch();
	}

	if (CommandIs("SetAmp"))
	{
LSetAmp:
		ifF( z, setAmp(z) );
		ifFF( z, time,
			fprintf(stderr, "vss error: setAmp with duration works only with handlers, not actors such as %s:\n\t\"%s\"",
				typeName(), Message) );
		return Uncatch();
	}

	if (CommandIs("SetAllAmp"))
	{
		ifFF( z, time, setAllAmp(z, time));
		ifF( z, setAllAmp(z));
		return Uncatch();
	}

	if (CommandIs("SetGain"))
	{
		ifF( z, setGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllGain"))
	{
		ifFF( z, time, setAllGain(z, time));
		ifF( z, setAllGain(z));
		return Uncatch();
	}

	if (CommandIs("SetPan"))
	{
		ifF( z, setPan(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllPan"))
	{
		ifFF( z, time, setAllPan(z, time));
		ifF( z, setAllPan(z) );
		return Uncatch();
	}

	if (CommandIs("SetElev"))
	{
		ifF( z, setElev(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllElev"))
	{
		ifFF( z, time, setAllElev(z, time));
		ifF( z, setAllElev(z) );
		return Uncatch();
	}

	if (CommandIs("SetDistance"))
	{
		ifF( d, setDistance(d) );
		return Uncatch();
	}

	if (CommandIs("SetAllDistance"))
	{
		ifFF( d, time, setAllDistance(d, time));
		ifF( d, setAllDistance(d) );
		return Uncatch();
	}

	if (CommandIs("SetXYZ"))
	{
		ifFFF( x,y,z, setXYZ(x,y,z) );
		return Uncatch();
	}

	if (CommandIs("SetAllXYZ"))
	{
		ifFFFF( x,y,z, time, setAllXYZ(x,y,z, time));
		ifFFF( x,y,z, setAllXYZ(x,y,z) );
		return Uncatch();
	}

	if (CommandIs("SetDistanceHorizon"))
	{
		ifF( d, setDistanceHorizon(d) );
		return Uncatch();
	}

	if (CommandIs("SetAllDistanceHorizon"))
	{
		ifF( d, setAllDistanceHorizon(d) );
		return Uncatch();
	}

	if (CommandIs("SetLinearEnv"))
	{
		ifD( lin, setLinear(lin) );
		return Uncatch();
	}

	return VActor::receiveMessage(Message);

}	// end of receiveMessage

//===========================================================================
//		parseInitializers
//
//	Called when a BeginSound is received with a list of initializers.
//
void
VGeneratorActor::parseInitializers(const char * inits_msg, VHandler * phandler)
{
	// parse this: list of { sz z, or sz sz }
	// z is [*]tNUMBER -- use sscanf(%f)
	// sz is tSTRING -- use sscanf(%s)
	// receiveMessage($1 hHandler $2);

	const char* pch = inits_msg;
	int cch = 0;
	int ichMax = strlen(pch);
	//printf("parseInitializers() gets <%s>, %d chars\n", inits_msg, ichMax);;
	char sz0[100];
	char sz1[500];
	char sz2[50];
	char sz3[50];
	char szCommand[700];
	while (pch-inits_msg < ichMax)
	{
		//printf("\t\tso far: <%s>, ich=%d\n", pch, pch-inits_msg);;
		if (1 != sscanf(pch, "%s %n", sz0, &cch))	// keyword string
			break;
		if (!isalpha(sz0[0]))
			{
			fprintf(stderr, "vss warning: expected a keyword, at \"%s\" in \"%s\"\n",
				sz0, inits_msg);
			break;
			}
		pch += cch;
		if (1 != sscanf(pch, "%s %n", sz1, &cch))	// value, string or float
		{
			fprintf(stderr, "vss warning: keyword '%s' not followed by its value in \"%s\"\n",
				sz0, inits_msg);
			break;
		}
		pch += cch;
		if (!strcmp(sz0, "SetXYZ"))
			{
			if (2 != sscanf(pch, "%s %s %n", sz2, sz3, &cch))	// 2 more floats
				{
				fprintf(stderr, "vss syntax error: incorrect x-y-z value for \"SetXYZ %s\" in \"%s\"\n", sz1, inits_msg);
				break;
				}
			pch += cch;
			strcat(sz1, " ");
			strcat(sz1, sz2);
			strcat(sz1, " ");
			strcat(sz1, sz3);
			}
#if 0
		// This is dangerous.  Some ifFF's (samp/sampHand.c++, chant/chantHand.c++) do NOT have the second F meaning a duration of modulation.
		strcat(sz1, " 0"); // Do it immediately not in dampingTime() seconds.
#endif

		// got a pair, so send it to receiver
		// fprintf(stderr, "got a pair <%s> <%s>\n", sz0, sz1);

		sprintf(szCommand, "%s %f %s", sz0, phandler->handle(), sz1);
		phandler->receiveMessage(szCommand);
	}
}	// end of parseInitializers


//===========================================================================
//		setXXX
//
void VGeneratorActor::setInputAmp(float a)
{
	zInputAmpl = a;
}

void VGeneratorActor::setInputGain(float a)
{
	zInputAmpl = ScalarFromdB(a);
}

void VGeneratorActor::invertAmp(int fInvert)
{
	fInvertAmp = fInvert;
}

void VGeneratorActor::setAmp(float a)
{
	zAmpl = a;
}

void VGeneratorActor::scaleAmp(float a)
{
	zScaleAmp = a;
}

void VGeneratorActor::setGain(float a)
{
	zAmpl = ScalarFromdB(a);
}

void VGeneratorActor::setPan(float a)
{
	pan = a;
}

void VGeneratorActor::setElev(float a)
{
	elev = a;
}

void VGeneratorActor::setDistance(float a)
{
	distance = a;
}

void VGeneratorActor::setDistanceHorizon(float a)
{
	distanceHorizon = a;
}

void VGeneratorActor::setXYZ(float x, float y, float z)
{
	xPos = x; yPos = y; zPos = z;
}

void VGeneratorActor::setLinear(int fLin)
{
	fLinearEnv = fLin;
}

//===========================================================================
//		setAllXXX
//
#define LoopHandlers(statement) \
	HandlerListIterator< VHandler > it; \
	for (it = children.begin(); it != children.end(); it++) \
		(*it)->statement


void VGeneratorActor::setAllInputAmp(float a, float t)
{
	LoopHandlers(setInputAmp(a, t));
	setInputAmp(a);
}

void VGeneratorActor::setAllInputGain(float a, float t)
{
	LoopHandlers(setInputGain(a, t));
	setInputGain(a);
}

void VGeneratorActor::invertAllAmp(int fInvert)
{
	LoopHandlers(invertAmp(fInvert));
	invertAmp(fInvert);
}

void VGeneratorActor::setAllAmp(float a, float t)
{
	LoopHandlers(setAmp(a, t));
	setAmp(a);
}

void VGeneratorActor::scaleAllAmp(float a, float t)
{
	LoopHandlers(scaleAmp(a, t));
	scaleAmp(a);
}

void VGeneratorActor::setAllGain(float a, float t)
{
	LoopHandlers(setGain(a, t));
	setGain(a);
}

void VGeneratorActor::setAllPan(float a, float t)
{
	LoopHandlers(setPan(a, t));
	setPan(a);
}

void VGeneratorActor::setAllElev(float a, float t)
{
	LoopHandlers(setElev(a, t));
	setElev(a);
}

void VGeneratorActor::setAllDistance(float a, float t)
{
	LoopHandlers(setDistance(a, t));
	setDistance(a);
}

void VGeneratorActor::setAllDistanceHorizon(float a)
{
	LoopHandlers(setDistanceHorizon(a));
	setDistanceHorizon(a);
}

void VGeneratorActor::setAllXYZ(float x, float y, float z, float t)
{
	LoopHandlers(setXYZ(x,y,z, t));
	setXYZ(x,y,z);
}

//===========================================================================
//		dump
//
//	Print biographical info on os.
//
ostream & 
VGeneratorActor::dump(ostream &os, int tabs)
{
	VActor::dump(os, tabs);

	indent(os, tabs) << "     Amp: " << zAmpl << endl;
//	indent(os, tabs) << "ScaleAmp: " << zScaleAmp << endl;
	return os;
}
