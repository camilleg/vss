#include "samp.h"
ACTOR_SETUP(sampActor, SampleActor)

sampActor::sampActor() :
	VGeneratorActor(),
	defaultLoopFlag( 0 ),
	defaultSampleStep( 1. )
{
	strcpy(defaultDirectory, ".");
	setTypeName("SampActor");
}

sampActor::~sampActor()
{
	unloadAllFiles( /* beFirm = */ true );
}

//	Don't use a default file name because the file is loaded when 
//	the file name is set. Shouldn't allow creation without a file 
//	name really.
void sampActor::sendDefaults(VHandler* p)
{
	VGeneratorActor::sendDefaults(p);
	sampHand* h = (sampHand*)p;
	h->setDirectory(defaultDirectory);
	h->setLoop( defaultLoopFlag );
	h->setSampleStep( defaultSampleStep );
}

int sampActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetAllBounds"))
	{
		ifFF( start, end, setAllBounds(start, end) );
		return Uncatch();
	}

	if (CommandIs("SetAllLoop"))
	{
		ifFFD( start, end, flag, setAllLoop(start, end, flag) );
		ifFF( start, end, setAllLoop(start, end) );
		ifD(flag, setAllLoop(flag) );
		return Uncatch();
	}
	if (CommandIs("SetLoop"))
	{
		ifF(flag, setLoop(flag) );
		return Uncatch();
	}

	if (CommandIs("SetAllDirectory"))
	{
		ifS( dir, setAllDirectory(dir) );
		return Uncatch();
	}
	if (CommandIs("SetDirectory"))
	{
		ifS( dir, setDirectory(dir) );
		return Uncatch();
	}

	//	sample step (like pitch changing)
	if (CommandIs("SetAllPlaybackRate"))
	{
		ifFF( step, time, setAllStep( step, time ) );
		ifF( step, setAllStep( step ) );
		return Uncatch();
	}
	if (CommandIs("SetPlaybackRate"))
	{
		ifF( step, setStep( step ) );
		return Uncatch();
	}

	if (CommandIs("RewindAll"))
	{
		ifNil( rewindAll() );
		// return Uncatch();
	}

	//	play-and-delete
	if (CommandIs("PlaySample"))
	{
		ifS( fname, playSample( fname ) );
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

	return VGeneratorActor::receiveMessage(Message);
}

void sampActor::setAllBounds(float start, float end) {
	HandlerListIterator<sampHand> it;
	for (it = children.begin(); it != children.end(); ++it)
		(*it)->setBounds( start, end );
}

void sampActor::setAllLoop(float start, float end, int flag) {
	HandlerListIterator<sampHand> it;
	for (it = children.begin(); it != children.end(); ++it)
		(*it)->setLoop( start, end, flag );
	defaultLoopFlag = flag;
}

void sampActor::setAllLoop(int flag) {
	HandlerListIterator<sampHand> it;
	for (it = children.begin(); it != children.end(); ++it)
		(*it)->setLoop(flag);
	defaultLoopFlag = flag;
}

void sampActor::setLoop(float flag) {
	defaultLoopFlag = flag;
}

void sampActor::setDirectory(char* dir) {
	strcpy( defaultDirectory, dir );
}

void sampActor::setAllDirectory(char* dir) {
	HandlerListIterator<sampHand> it;
	for (it = children.begin(); it != children.end(); ++it)
		(*it)->setDirectory( dir );
	strcpy(defaultDirectory, dir);
}

void sampActor::setStep(float step) {
	if (!CheckSampleStep(step))
		printf("vss error: SampleActor got bogus sample step %f.\n", step);
	else
		defaultSampleStep = step;
}

void sampActor::setAllStep(float step, float time) {
	if (!CheckSampleStep(step)) {
		printf("vss error: SampleActor got bogus sample step %f.\n", step);
		return;
	}
	HandlerListIterator<sampHand> it;
	for (it = children.begin(); it != children.end(); ++it)
		(*it)->setSampleStep( step, time );
	defaultSampleStep =  step;
}

void sampActor::rewindAll() {
	HandlerListIterator<sampHand> it;
	for (it = children.begin(); it != children.end(); ++it)
		(*it)->jumpTo( (*it)->getStart() );
}

// Create a new sampHand with the specified sample file.
void sampActor::playSample(char* fname)
{
	sampHand* s = (sampHand*)newHandler();
	addChild(s);
	sendDefaults(s);
	s->setFile(fname);
	s->setDeleteWhenDone();
	s->setPause(0);
}

// Load a new file from disk, if needed.
sfile* sampActor::loadFile(char* dirName, char* fName) {
	for (const auto& f: fileList)
		if (f->equDirFile(dirName, fName))
			return f; // Already loaded.

	const auto newOne = new sfile(dirName, fName);
	if (!newOne->samples()) {
		printf("vss error: SampleActor failed to load sample file %s\n", fName);
		delete newOne;
		return nullptr;
	}
	fileList.push_back(newOne);
	return newOne;
}

// If this file is already in memory, remove it.
void sampActor::unloadFile(char* dirName, char* fName) {
    for (auto it = fileList.begin(); it != fileList.end(); ++it) {
		if ((*it)->equDirFile(dirName, fName)) {
        	if ((*it)->unused()) {
        		delete *it;
        		fileList.erase(it);
			} else {
				fprintf(stderr, "vss: SampleActor file \"%s\" is still in use.\n", fName);
			}
       		return;
       	}
	}
	fprintf(stderr, "vss: SampleActor did not find %s to unload.\n", fName);
}


// Unload files not still in use.  If beFirm, unload all files
// (may crash, unless from the destructor, which then deletes the
// VAlgorithms that are using these sfiles).
void sampActor::unloadAllFiles(int beFirm) {
	if (beFirm) {
		for (auto f: fileList) delete f;
		fileList.clear();
		return;
	}
    for (auto it = fileList.begin(); it != fileList.end();) {
		if ((*it)->unused()) {
			delete *it;
			it = fileList.erase(it); // works with either list or vector
		} else {
			fprintf(stderr, "vss: SampleActor file %s is still in use.\n", (*it)->name());
			++it;
		}
	}
}
