#include "samp.h"

//===========================================================================
//		sampHand construction
//
sampHand::sampHand(sampAlg * a) :
	VHandler(a),
	deleteWhenDone( 0 ),
	hMGDelete( hNil ),
	zhMGDeleteData( 0. ),
	sampleStep( 1. )
{
	setTypeName("sampHand");
	directoryName[0] = '\0';
}

//===========================================================================
//		sampHand act
//
//	Check for deleteWhenDone and delete yourself if necessary.
//
void
sampHand::act(void)
{
	VHandler::act();	
	if ( deleteWhenDone && 
		// make sure it is past the end and not looping
		! getAlg()->getLoop() && 
		getAlg()->getPosition() >= getAlg()->getEnd() )
		{
		if (hMGDelete != hNil)
			{
			char szCmd[80];
			sprintf(szCmd, "SendData %f [%f %f]",
				hMGDelete, handle(), zhMGDeleteData);
			actorMessageHandler(szCmd);
			}
		// the sample has played past the end and isn't looping
		delete this;
		}
}

//===========================================================================
//		sampHand receiveMessage
//
int	
sampHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);
	
	if (CommandIs("SetBounds"))
	{
		ifFF( start, end, setBounds(start, end) );
		return Uncatch();
	}
	
	if (CommandIs("SetLoop"))
	{
		ifFFF( start, end, flag, setLoop(start, end, flag) );
		ifFF( start, end, setLoop(start, end) );
		ifF( flag, setLoop(flag) );
		return Uncatch();
	}
	
	if (CommandIs("SetFile"))
	{
		ifS( fname, setFile(fname) );
		return Uncatch();
	}
	
	if (CommandIs("SetDirectory"))
	{
		ifS( dname, setDirectory(dname) );
		return Uncatch();
	}
	
	if (CommandIs("SetPlaybackRate"))
	{
		ifFF( step, time, setSampleStep( step, time ) );
		ifF( step, setSampleStep( step ) );
		return Uncatch();
	}
	
	if (CommandIs("JumpTo"))
	{
		ifF( time, jumpTo( time ) );
		return Uncatch();
	}

	if (CommandIs("DeleteWhenDone"))
	{
		ifD( d, setDeleteWhenDone(d) );
		ifNil( setDeleteWhenDone() );
		// return Uncatch();
	}

	if (CommandIs("DeleteWhenDoneMG"))
	{
		ifFF( z1,z2, setDeleteWhenDone(1,z1,z2) );
		ifF( z, setDeleteWhenDone(1,z) );
		// return Uncatch();
	}

	if (CommandIs("Rewind"))
	{
		ifNil( jumpTo( getStart() ) );
		// return Uncatch();
	}
	
	return VHandler::receiveMessage(Message);
}

//===========================================================================
//		sampHand setDirectory
//
void
sampHand::setDirectory(char * dir)
{
	if (dir[0] != 0)
		strcpy(directoryName, dir);
	else
		strcpy(directoryName, ".");
}

//===========================================================================
//		sampHand setFile
//
//	Duh, and also set the sample step, because the algorithm resets it
//	to the sample rate ratio when it reads a new file.
//
void
sampHand::setFile(char * fileName)
{
	sampActor * myParent = (sampActor *)getParent();
	if (myParent == NULL)
	{
		printf("vss internal error: sampHand found itself with NULL parent!!\n");
		return;
	}

	sfile * newFile = myParent->loadFile(directoryName, fileName);
	if (newFile == NULL)
		return;
		
	getAlg()->setFile(newFile);
	getAlg()->setSampleStep( sampleStep );
}

//===========================================================================
//		sampHand setBounds(start, end)
//
void
sampHand::setBounds( float start, float end )
{
	getAlg()->setBounds( start, end );
}

//===========================================================================
//		sampHand setLoop(start, end, flag)
//
void
sampHand::setLoop( float start, float end, float flag )
{
	getAlg()->setLoop( start, end, (int) flag );
}

//===========================================================================
//		sampHand setLoop(flag)
//
void
sampHand::setLoop( float flag )
{
	getAlg()->setLoop( (int) flag );
}

//===========================================================================
//		sampHand jumpTo
//
void
sampHand::jumpTo( float time )
{
	getAlg()->jumpTo( time );
}

//===========================================================================
//		sampHand setSampleStep
//
void sampHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case isetSampleStep:
			if (!CheckSampleStep(z))
				printf("sampHand got bogus sample step %f.\n", z);
			else
				getAlg()->setSampleStep(sampleStep = z);
			break;
		default:
			printf("vss error: sampHand got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: sampHand got bogus element-of-float-array-index %d.\n", iParam.i);
}

//	In addition to calling VHandler's restrike, which resets the parameters,
//	rewind the file, so that restriking without specifying a new file rewinds
//	the current file.
void sampHand::restrike(const char* inits_msg)
{
	jumpTo( getStart() );
	VHandler::restrike(inits_msg);
}
