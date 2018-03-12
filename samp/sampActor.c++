#include "samp.h"

ACTOR_SETUP(sampActor, SampleActor)

//===========================================================================
//		sampActor construction
//
//	Initialize defaults to be sent in sendDefaults.
//
sampActor::sampActor(void) :
	VGeneratorActor(),
	defaultLoopFlag( 0 ),
	defaultSampleStep( 1. )
{
	strcpy(defaultDirectory, ".");
	setTypeName("SampActor");
}

//===========================================================================
//		sampActor destruction
//
//	Throw out all the sfiles that were allocated by this actor.
//
sampActor::~sampActor()
{
	unloadAllFiles( /* beFirm = */ true );
}

//===========================================================================
//		sampActor sendDefaults
//
//	Don't use a default file name because the file is loaded when 
//	the file name is set. Shouldn't allow creation without a file 
//	name really.
//
void 
sampActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	sampHand * h = (sampHand *)p;
	h->setDirectory(defaultDirectory);
	h->setLoop( defaultLoopFlag );
	h->setSampleStep( defaultSampleStep );
}

//===========================================================================
//		sampActor receiveMessage
//
int
sampActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	//	sample bounds
	if (CommandIs("SetAllBounds"))
	{
		ifFF( start, end, setAllBounds(start, end) );
		return Uncatch();
	}

	//	looping
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

	//	directory
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
	
	//	rewind
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
	
	return VGeneratorActor::receiveMessage(Message);
}

//===========================================================================
//		sampActor setAllBounds(start, end)
//
void
sampActor::setAllBounds( float start, float end )
{
	HandlerListIterator< sampHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setBounds( start, end );
	}
}

//===========================================================================
//		sampActor setAllLoop(start, end, flag)
//
void
sampActor::setAllLoop( float start, float end, int flag )
{
	HandlerListIterator< sampHand > it;
	for (it = children.begin(); it != children.end(); it++)
		(*it)->setLoop( start, end, flag );

	defaultLoopFlag = flag;
}

//===========================================================================
//		sampActor setAllLoop(flag)
//
void
sampActor::setAllLoop( int flag )
{
	HandlerListIterator< sampHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setLoop(flag);
	}

	defaultLoopFlag = flag;
}

//===========================================================================
//		sampActor setLoop(flag)
//
void
sampActor::setLoop( float flag )
{
	defaultLoopFlag = (int) flag;
}

//===========================================================================
//      sampActor setDirectory
//
void
sampActor::setDirectory(char * dir)
{
	strcpy( defaultDirectory, dir );
}

//===========================================================================
//      sampActor setAllDirectory
//
void
sampActor::setAllDirectory(char * dir)
{
	HandlerListIterator< sampHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setDirectory( dir );
	}

	strcpy( defaultDirectory, dir );
}

//===========================================================================
//      sampActor setStep
//
void
sampActor::setStep(float step)
{
	if (!CheckSampleStep(step))
		printf("vss error: SampleActor got bogus sample step %f.\n", step);
	else
		defaultSampleStep =  step;
}

//===========================================================================
//      sampActor setAllStep
//
void
sampActor::setAllStep(float step, float time)
{
	if (!CheckSampleStep(step))
	{
		printf("vss error: SampleActor got bogus sample step %f.\n", step);
		return;
	}
	
	HandlerListIterator< sampHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setSampleStep( step, time );
	}

	defaultSampleStep =  step;
}

//===========================================================================
//      sampActor rewindAll
//
void
sampActor::rewindAll(void)
{
	HandlerListIterator< sampHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->jumpTo( (*it)->getStart() );
	}
}

//===========================================================================
//      sampActor playSample
//
//	Create a new sampHand with the specified sample file, and 
//	have it deleteWhenDone.
//
void
sampActor::playSample(char * fname)
{
	sampHand * s = (sampHand *) newHandler();
	if (s == NULL) return;

	addChild( s );
	sendDefaults( s );
	
	s->setFile(fname);
	s->setDeleteWhenDone();
	s->setPause(0);
}

//===========================================================================
//      sampActor loadFile
//
//	Look through the list of files already in memory. If the specified
//	file is found, return the corrsponding sfile. Otherwise, create a 
//	new sfile, add it to the list, and return it.
//
sfile *
sampActor::loadFile(char * dirName, char * fName)
{
	for (SfileList::iterator it = fileList.begin(); it != fileList.end(); ++it)
	{
		if ( (*it)->equDirFile( dirName, fName ) )
		{
        		// found it
        		#ifdef DEBUG
        		printf("SampleActor found %s loaded.\n", fName);
        		#endif
	        	return (*it);
		}
	}

	//	not found
	sfile * newOne = new sfile(dirName, fName);
	if (newOne == NULL || newOne->samples() == NULL)
	{
		printf("vss error: SampleActor failed to load sample file %s\n", fName);
		if (newOne != NULL) delete newOne;
		return NULL;
	}
        	
#ifdef DEBUG
	printf("SampleActor loaded %s.\n", fName);
#endif

	fileList.push_back( newOne );
	return newOne;
}

//===========================================================================
//      sampActor unloadFile
//
//	Look through the list of files already in memory. If the specified
//	file is found, remove it.
//
void
sampActor::unloadFile(char * dirName, char * fName)
{
    SfileList::iterator it;
    for (it = fileList.begin(); it != fileList.end(); it++)
	{
		if ( (*it)->equDirFile( dirName, fName ) )
		{
        	// found it
        	#ifdef DEBUG
        	printf("SampleActor found %s , unloading.\n", fName);
        	#endif
        	if ( (*it)->numUsers() == 0 )
        	{
        		delete (*it);
        		fileList.erase( it-- );
        	}
        	else
        		printf("vss error: SampleActor file \"%s\" is still in use.\n",
					fName);
       		return;
       	}
	}
	
	printf("vss warning: SampleActor did not find %s to unload.\n", fName);
}

//===========================================================================
//      sampActor unloadAllFiles
//
//	Run through the list of files already in memory and unload everything
//	that isn't still in use. If beFirm is true, then unload things that
//	are in use too. This is likely to cause core dumps, and is only good
//	for use in the destructor, when the next thing to happen will be the 
//	deletion of all the sampAlgs that are using these sfiles.
//
void
sampActor::unloadAllFiles(int beFirm)
{
	SfileList::iterator it;

	if (beFirm)
		{
		for (it = fileList.begin(); it != fileList.end(); it++)
			{
#ifdef DEBUG
			printf("SampleActor firmly unloading %s.\n", (*it)->name());
#endif
			delete (*it);
			}
		fileList.clear();
		}
	else
		{
		for (it = fileList.begin(); it != fileList.end(); it++)
			{
#ifdef DEBUG
			printf("SampleActor unloading %s.\n", (*it)->name());
#endif
			if ( (*it)->numUsers() == 0 )
				{
				delete (*it);
				fileList.erase( it-- );
				}
			else
				printf("vss error: SampleActor file \"%s\" is still in use.\n",
					(*it)->name());
			}
		}
}
