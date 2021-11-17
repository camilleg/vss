#include "gran.h"

ACTOR_SETUP(granActor, Granulator)

granActor::granActor() :
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

granActor::~granActor()
{
	unloadAllFiles( /* beFirm = */ true );
}

//	Don't use a default file name because the file is loaded when 
//	the file name is set. Shouldn't allow creation without a file 
//	name really.
void granActor::sendDefaults(VHandler* p)
{
	VGeneratorActor::sendDefaults(p);
	granHand* h = (granHand*)p;
	h->setDirectory(defaultDirectory);
	h->setSampleStep(defaultSampleStep);
}

int granActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

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

void granActor::setDirectory(char* dir)
{
	strcpy( defaultDirectory, dir );
}

void granActor::setAllDirectory(char * dir)
{
	HandlerListIterator< granHand > it;
    for (it = children.begin(); it != children.end(); it++)
		(*it)->setDirectory( dir );
	strcpy( defaultDirectory, dir );
}

#if 0
// Create a new granHand with the specified sample file.
void granActor::playSample(char* fname)
{
	granHand* s = (granHand*)newHandler();
	if (!s)
		return;
	addChild(s);
	sendDefaults(s);
	s->setFile(fname);
	s->setPause(0);
}
#endif

// Load a new file from disk, if needed.
sfile* granActor::loadFile(char* dirName, char* fName)
{
    for (const auto f: fileList)
		if (f->equDirFile(dirName, fName))
			return f; // Already loaded.

	const auto newOne = new sfile(dirName, fName);
	if (!newOne->samples())
	{
		fprintf(stderr, "vss: granActor failed to load sample file %s\n", fName);
		delete newOne;
		return nullptr;
	}
	fileList.push_back(newOne);
	return newOne;
}

// If this file is already in memory, remove it.
void granActor::unloadFile(char* dirName, char* fName)
{
    for (auto it = fileList.begin(); it != fileList.end(); ++it) {
		if ((*it)->equDirFile(dirName, fName)) {
        	if ((*it)->numUsers() == 0) {
        		delete *it;
        		fileList.erase(it);
        	}
        	else
        		fprintf(stderr, "vss: granActor can't unload soundfile %s because it's still in use.\n", fName);
       		return;
       	}
	}

	fprintf(stderr, "vss: granActor did not find soundfile %s to unload.\n", fName);
}

// Unload files not still in use.  If beFirm is true, unload all files
// (may crash, unless from the destructor, when the next event is
// the deletion of the granAlgs that are using these sfiles).

void granActor::unloadAllFiles(int beFirm)
{
    for (auto it = fileList.begin(); it != fileList.end(); ++it) {
        if (beFirm || (*it)->numUsers() == 0) {
        	delete *it;
        	fileList.erase(it--);
        }
        else
        	fprintf(stderr, "vss: granActor can't unload soundfile %s because it's still in use.\n", (*it)->name());
	}
}
