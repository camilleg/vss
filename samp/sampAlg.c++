#include "samp.h"

sampAlg::sampAlg() :
	file( NULL ),
	index( 0. ),
	startAt( 0L ),
	endAt( 0L ),
	startLoopAt( 0L ),
	endLoopAt( 0L ),
	sampleStep( 1. ),
	sampleScale(1.),
	loop( 0 ),
	sampleData( NULL ),
	fileNumChans( 0 ),
	getSampFn( NULL )
{
}

sampAlg::~sampAlg()
{
#if 0
	// Too dangerous.  "file" may have been already deleted
	// but not zero'd in sampActor::unloadAllFiles().
	if (file)
		file->removeUser(this);
#endif
}

// sampleData access.
float sampAlg::get8bitSamp(ulong frame, int chan) {
	return (((unsigned char *)sampleData)[chan + (frame * fileNumChans)]-128);
}
float sampAlg::get16bitSamp(ulong frame, int chan) {
	return ((short *)sampleData)[chan + (frame * fileNumChans)];
}

float sampAlg::getSamp(ulong frame, int chan) {
#ifdef DEBUG
	if (!getSampFn) {
		printf("vss error: sampAlg has NULL getSampFn!!\n");
		return 0.0;
	}
	if (chan > fileNumChans) {
		printf("vss error: SampleActor tried to access more channels than this file has.\n");
		return 0.0;
	}
#endif
	return (this->*getSampFn)(frame, chan);
}

//	Number of output channels is the width of the input file, so this could
//	change during the life of a single actor if it loads different files.
void sampAlg::generateSamples(int howMany) {
	Nchan(fileNumChans);
	if (index > (float)endAt)
		{
		// Unless the file itself has a negative sample rate,
		// sampleStep>=0, so in the for-loop we'll hit the first if()
		// and stay stuck in it.  So let's optimize this case.
		//
		// ;; Ideally, take all the if's out of that for-loop!
		//
		ClearBuffer(howMany);
		return;
		}

	for (int s = 0; s < howMany; s++)
		{
		if (index > (float)endAt)
			{
			//	If we run off the end of the sample, just copy zeros
			//	into the buffer. No need to update the index any more.
			//	Also if there's no sample data yet, copy zeros.
			ClearSample(s);
			continue;
			}
		
		//  perform linear interpolation on the samples
		//  the index is a float for improved playback rate accuracy
		double w2 = sampleScale * (index - (double)ulong(index)); //DZ
		double w1 = sampleScale - w2; //DZ

		//  patch file channels to output channels
		float samp[MaxNumChannels];
		for (int chan = 0; chan < fileNumChans; chan++)
			{
			samp[chan] = w1 * getSamp((ulong)index    , chan) +
						 w2 * getSamp((ulong)index + 1, chan);
			}
		OutputNchan(samp, s);

		//  update the index, test for looping
		index += (double)sampleStep; //DZ

		//NEW DZ 2-2004
		if(sampleStep>0.0)
		{
  			if (loop && (index > (float)endLoopAt))
     				index -= (float)(endLoopAt - startLoopAt +1);
		}
		else //negative sample step
		{
 			if(loop && (index < (float)startLoopAt))
   	    			index = (float)(endLoopAt);
		}
	
	}
}

// Move the index to the specified time, in seconds.
void
sampAlg::jumpTo( float time )
{
	if (time < 0.)
		printf("vss error: SampleActor cannot JumpTo a time before the beginning of the file.\n");
	else if (file == NULL)
		printf("vss error: SampleActor cannot JumpTo since no file has been loaded.\n");
	else
	{
		index = (time * file->sampleRate()) + 0.5;
		if (index > (float)endAt)
			printf("vss remark: sampAlg jumped past sample end.\n");
		if (index > (float)endLoopAt)
			printf("vss remark: sampAlg jumped past loop end.\n");
	}
}

// Select a new AIFF file.  Reset the loop parameters.
void
sampAlg::setFile(sfile * newFile)
{
	if (!newFile)
		{
		printf("vss error: SampleActor tried to set file to NULL in sampAlg::setFile().\n");
		return;
		}
	
	resetFileParams();
	
//	select a sample indexing function
	switch ( newFile->sampleSize() )
	{
		case 8:
			getSampFn = &sampAlg::get8bitSamp;
			sampleScale = 1. / 128.;
			break;
		case 16:
			getSampFn = &sampAlg::get16bitSamp;
			sampleScale = 1. / 32767.;
			break;
		default:
			printf( "vss error: SampleActor file \"%s\" has an unrecognized sample size.\n\n", 
					newFile->name() );
			return;
	}
	
//	set parameters
	file = newFile;
	sampleData = file->samples();
	fileNumChans = file->numChannels();
	endLoopAt = endAt = file->numFrames() - 1;
	sampleStep = getSRratio();
	file->addUser(this);
}

//	Clear out all the file parameters and dump any samples in memory.
//	Don't clear parameters like the loop that aren't related to a 
//	particular file.
void
sampAlg::resetFileParams(void)
{
	if (file != NULL) file->removeUser(this);
	file = NULL;
	index = 0.;
	startAt = endAt = 0L;
//	startLoopAt = endLoopAt = 0L;	// not related to a particular file
//	loop = 0;			// not related to a particular file
	sampleStep = 1.;
	getSampFn = NULL;
	sampleData= NULL;
	fileNumChans = 0;
}

// Set the begin and end times for playback of a sample, in seconds.
void
sampAlg::setBounds(float begin, float end)
{
	if (!file)
		return;
	if (begin >= end)
	{
		printf("vss error: SampleActor playback start cannot be the same as or later than end.\n");
		return;
	}
	startAt = std::max(0., (begin * file->sampleRate()) + 0.5 /* cheap rounding */);
	endAt = std::min(file->numFrames() - 1., (end * file->sampleRate()) + 0.5);
	//	if endAt has been set back to before current index, 
	//	push index back to the end sample
	if ((float)endAt < index)
		index = (float)endAt;
}

//	Set the begin and end times for a sample loop, in seconds.
void
sampAlg::setLoop(float begin, float end, int flag)
{
	if (!file)
		return;
	if (begin >= end)
	{
		printf("vss error: in SampleActor SetLoop, loop start (%g) cannot be the same as or later than loop end (%g).\n",
			begin, end);
		return;
	}
	startLoopAt = std::max(0., (begin * file->sampleRate()) + 0.5 /* cheap rounding */);
	endLoopAt = std::min(file->numFrames() - 1., (end * file->sampleRate()) + 0.5);
	loop = flag;
}

void
sampAlg::setLoop(int flag)
{
	loop = flag;
}
