#include "gran.h"

ACTOR_SETUP(granActor, Granulator)

//===========================================================================
//		granActor construction
//
//	Initialize defaults to be sent in sendDefaults.
//
granActor::granActor(void) :
	VGeneratorActor(),
	myDur(0),
	myStart(0),
	mySlope(0),
	controlMin(0),
	controlMax(1),
	jumpMin(0),
	jumpMax(1),
	rebound(0.),
	spread(0.05),
	defaultSampleStep(1.)
{
	strcpy(defaultDirectory, ".");
	setTypeName("Granulator");
}

//===========================================================================
//		granActor destruction
//
//	Throw out all the sfiles that were allocated by this actor.
//
granActor::~granActor()
{
	unloadAllFiles( /* beFirm = */ true );
}

//===========================================================================
//		granActor sendDefaults
//
//	Don't use a default file name because the file is loaded when 
//	the file name is set. Shouldn't allow creation without a file 
//	name really.
//
void 
granActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	granHand * h = (granHand *)p;
	h->setDirectory(defaultDirectory);
	h->setSampleStep(defaultSampleStep);
}

//===========================================================================
//		granActor receiveMessage
//
int
granActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	//	directory
	if (CommandIs("SetAllDirectory")) //;;;; nuke this
	{
		ifS( dir, setAllDirectory(dir) );
		return Uncatch();
	}

	if (CommandIs("SetDirectory"))
	{
		ifS( dir, setDirectory(dir) );
		return Uncatch();
	}
	
	//	loading and unloading
	if (CommandIs("LoadFile"))
	{
		ifS( fname, loadFile(fname) );
		return Uncatch();
	}
	
	if (CommandIs("UnloadFile"))
	{
		ifS( fname, unloadFile(fname) );
		return Uncatch();
	}
	
	if (CommandIs("UnloadAllFiles"))
	{
		ifNil( unloadAllFiles() );
		// return Uncatch();
	}

	if (CommandIs("SetPlaybackRate"))
	{
		ifF( step, setStep( step ) );
		return Uncatch();
	}

	if (CommandIs("SetDur"))
	{
		ifF( _, setDur( _ ) );
		return Uncatch();
	}

	if (CommandIs("SetStart"))
	{
		ifF( _, setStart( _ ) );
		return Uncatch();
	}

	if (CommandIs("SetSlope"))
	{
		ifF( _, setSlope( _ ) );
		return Uncatch();
	}

	if (CommandIs("SetControlMin"))
	{
		ifF( _, setControlMin( _ ) );
		return Uncatch();
	}

	if (CommandIs("SetControlMax"))
	{
		ifF( _, setControlMax( _ ) );
		return Uncatch();
	}

	if (CommandIs("SetJumpMin"))
	{
		ifF( _, setJumpMin( _ ) );
		return Uncatch();
	}

	if (CommandIs("SetJumpMax"))
	{
		ifF( _, setJumpMax( _ ) );
		return Uncatch();
	}

	if (CommandIs("Rebound"))
	{
		ifF( _, setRebound( _ ) );
		return Uncatch();
	}

	if (CommandIs("Spread"))
	{
		ifF( _, setSpread( _ ) );
		return Uncatch();
	}

	return VGeneratorActor::receiveMessage(Message);
}

void granActor::setDur(float t)
{
	myDur = t;
}

void granActor::setStart(float t)
{
	myStart = t;
}

void granActor::setSlope(float t)
{
	mySlope = t;
}

void granActor::setStep(float step)
{
	if (!CheckSampleStep(step))
		fprintf(stderr, "vss error: granActor got bogus playback rate %f.\n", step);
	else
		defaultSampleStep =  step;
}

static float Clamp01(float _, const char* szError)
{
	if (_>=0. && _<=1.)
		return _;
	cerr << "vss granActor error: " << szError << " out of range 0 to 1 (" << _ << ")\n";
	return _<0. ? 0. : 1.;
}

void granActor::setControlMin(float _)
{
	controlMin = _;
}

void granActor::setControlMax(float _)
{
	controlMax = _;
}

void granActor::setJumpMin(float _)
{
	jumpMin = Clamp01(_, "SetJumpMin");
}

void granActor::setJumpMax(float _)
{
	jumpMax = Clamp01(_, "SetJumpMax");
}

void granActor::setSpread(float _)
{
	spread = Clamp01(_, "SetSpread");
}

void granActor::setRebound(float _)
{
	rebound = Clamp01(_, "SetRebound");
}

void granActor::setInterval(float start, float end)
{
	myStart = start;
	myDur = end - start;
}

//===========================================================================
//      granActor setDirectory
//
void
granActor::setDirectory(char * dir)
{
	strcpy( defaultDirectory, dir );
}

//===========================================================================
//      granActor setAllDirectory
//
void
granActor::setAllDirectory(char * dir)
{
	HandlerListIterator< granHand > it;
    for (it = children.begin(); it != children.end(); it++)
		(*it)->setDirectory( dir );
	strcpy( defaultDirectory, dir );
}

#if 0
//===========================================================================
//      granActor playSample
//
//	Create a new granHand with the specified sample file.
//
void
granActor::playSample(char * fname)
{
	granHand * s = (granHand *) newHandler();
	if (s == NULL) return;

	addChild( s );
	sendDefaults( s );
	
	s->setFile(fname);
	s->setPause(0);
}
#endif

//===========================================================================
//      granActor loadFile
//
//	Look through the list of files already in memory. If the specified
//	file is found, return the corrsponding sfile. Otherwise, create a 
//	new sfile, add it to the list, and return it.
//
sfile *
granActor::loadFile(char * dirName, char * fName)
{
	SfileList::iterator it;
    for (it = fileList.begin(); it != fileList.end(); it++)
	{
		if ( (*it)->equDirFile( dirName, fName ) )
		{
        	// found it
        	#ifdef DEBUG
        	//fprintf(stderr, "granActor found %s loaded.\n", fName);
        	#endif
        	return (*it);
        }
	}

	//	not found
	sfile * newOne = new sfile(dirName, fName);
	if (newOne == NULL || newOne->samples() == NULL)
	{
		fprintf(stderr, "vss: granActor failed to load sample file %s\n", fName);
		if (newOne != NULL) delete newOne;
		return NULL;
	}
        	
#ifdef DEBUG
	//fprintf(stderr, "granActor loaded %s.\n", fName);
#endif

	fileList.push_back( newOne );
	return newOne;
}

//===========================================================================
//      granActor unloadFile
//
//	Look through the list of files already in memory. If the specified
//	file is found, remove it.
//
void
granActor::unloadFile(char * dirName, char * fName)
{
    SfileList::iterator it;
    for (it = fileList.begin(); it != fileList.end(); it++)
	{
		if ( (*it)->equDirFile( dirName, fName ) )
		{
        	// found it
        	#ifdef DEBUG
        	//fprintf(stderr, "granActor found %s , unloading.\n", fName);
        	#endif
        	if ( (*it)->numUsers() == 0 )
        	{
        		delete (*it);
        		fileList.erase( it-- );
        	}
        	else
        		fprintf(stderr, "vss: granActor can't unload soundfile %s because it's still in use.\n", fName);
       		return;
       	}
	}
	
	fprintf(stderr, "vss: granActor did not find soundfile %s to unload.\n", fName);
}

//===========================================================================
//      granActor unloadAllFiles
//
//	Run through the list of files already in memory and unload everything
//	that isn't still in use. If beFirm is true, then unload things that
//	are in use too. This is likely to cause core dumps, and is only good
//	for use in the destructor, when the next thing to happen will be the 
//	deletion of all the granAlgs that are using these sfiles.
//
void
granActor::unloadAllFiles(int beFirm)
{
	SfileList::iterator it;
    for (it = fileList.begin(); it != fileList.end(); it++)
    {
        #ifdef DEBUG
        //fprintf(stderr, "granActor found %s , unloading.\n", (*it)->name());
        #endif
        if ( beFirm || (*it)->numUsers() == 0 )
        {
        	delete (*it);
        	fileList.erase( it-- );
        }
        else
        	fprintf(stderr, "vss: granActor can't unload soundfile %s because it's still in use.\n",
        			(*it)->name());
	}
}
