//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
//===========================================================================

#include "gran.h"

//===========================================================================
//		granAlg constructor
//
granAlg::granAlg(void) :
	file( NULL ),
	index( 0. ),
	startAt( 0L ),
	endAt( 0L ),
	dur ( 0L ),
	sampleStep(1.),
	sampleScale(1.),
	slope(0),
	rampDown(0),
	fStartedRampDown( 0 ),
	rebound(0.),
	spread(0.05),
	sampleData( NULL ),
	fileNumChans( 0 ),
	getSampFn( NULL )
{
	// printf("Nchan 1 fer shur\n");;;;
	Nchan(1/*fileNumChans*/);;;;
}

//===========================================================================
//		granAlg destructor
//
granAlg::~granAlg(void)
{
	if (file != NULL) file->removeUser(this);
}

//===========================================================================
//		sampleData access
//
inline float
granAlg::get8bitSamp( ulong frame, int chan )
{
	return (float) ((char *)sampleData)[chan + (frame * fileNumChans)];
}

inline float
granAlg::get16bitSamp( ulong frame, int chan )
{
	return (float) ((short *)sampleData)[chan + (frame * fileNumChans)];
}

inline float
granAlg::getSamp( ulong frame, int chan )
{
#ifdef DEBUG
	if ( getSampFn == NULL )
	{
		fprintf(stderr, "granAlg has NULL getSampFn!!\n");
		return 0.;
	}

	if (chan > fileNumChans)
	{
		fprintf(stderr, "vss: granActor tried to access more channels than this file has.\n");
		return 0.;
	}
#endif

	return (this->*getSampFn)(frame, chan);
}

//===========================================================================
//		outputSamples
//
//	# of output channels is the # of channels in the input file.
//
void
granAlg::generateSamples(int howMany)
{
	Nchan(/*fileNumChans>0 ? fileNumChans :*/ 1);;;;

	for (int j = 0; j < howMany; j++)
		{

//	if we run off the end of the sample, just copy zeros
//	into the buffer. No need to update the index any more.
//	also if there's no sample data yet, copy zeros.

		if (!fStartedRampDown && index > rampDown)
			{
			fStartedRampDown = 1;
//fprintf(stderr, "rampdown! %f\n", rampDown);;

			// Note that we're not updating the parent VHandler object
			// about this change of amplitude in us, the algorithm.
			// Normally we'd do VHandler::setAmp(0, time);
			// but there's no pointer back to the VHandler object.
			// So we just do this and leave the VHandler object in the dark:

			VAlgorithm::setAmp(0, slope);

			// And what that means to you, gentle reader, is that
			// you shouldn't send messages to a grain after it's
			// started playing.  Adjust future grains instead, you doofus.
			}

		if (index > endAt)
			{
			// we're past the end, we'll never be before it again,
			// so fill the rest of the buffer with zeros.
			// (The handler may have been deleted, also.)
			for (int i=j; i<howMany; i++)
				ClearSample(i);
			break;
			}

//	perform linear interpolation on the samples
//	the index is a float for improved playback rate accuracy		
		float w2 = sampleScale * (index - ulong(index));
		float w1 = sampleScale - w2;

		for (int chan=0; chan < fileNumChans; chan++)
			{
			float gran = w1 * getSamp((ulong)index    , chan) + 
						 w2 * getSamp((ulong)index + 1, chan);
			Output(gran, j, chan);
			}

		//	update the index
		index += sampleStep;
		}	
}

//===========================================================================
//		granAlg setStart
//
//	Start the grain at the specified fraction 0..1 into the source file.
//	Keep the duration intact, by adjusting the ending time of the grain
//	to match.
//
void
granAlg::setStart( float time )
{

	if (!file || file->sampleRate() == 0)
		{
		fprintf(stderr, "vss error: granAlg::setStart called with no setFile before it!\n");
		return;
		}

	//  input time: controlMin..controlMax.
	// output time: 0..1.

//fprintf(stderr, "\n\n\tgranAlg::setStart\nnow time is %g\n", time);;

	time = (time - controlMin) / (controlMax - controlMin);


//fprintf(stderr, "now time is %g     (ctrl %g %g)\n", time, controlMin, controlMax);;

	time += spread * (drand48() * 2. - 1.);

//fprintf(stderr, "time %g, rebound %g, spread %g\n", time, rebound, spread);;

	// If out of range, clamp and also rebound back a little bit.
	if (time < 0.)
		{
		time = drand48() * rebound;
		}
	else if (time > 1.)
		{
		time = 1. - drand48() * rebound;
		}

//fprintf(stderr, "now time is %g     (0..1 clamp)\n", time );;

	//  input time: 0..1.
	// output time: jumpMin..jumpMax.
	time = jumpMin + (jumpMax-jumpMin) * time;

//fprintf(stderr, "now time is %g     (jump %g %g)\n", time, jumpMin, jumpMax);;

	//  input time: jumpMin..jumpMax (which is a subset of 0..1)
	// output time: seconds // (well, samples actually).
	time *= (float)fileEnd / file->sampleRate();

//fprintf(stderr, "now time is %g    (fileEnd %g sec)\n\n", time, (float)fileEnd / file->sampleRate());;

	if (time < 0.)
		fprintf(stderr, "vss: granAlg cannot jump to a time before the beginning of the file.\n");
	else if (file == NULL)
		fprintf(stderr, "vss: granAlg has no file to jumpto in.\n");
	else
	{
		startAt = (ulong)(index = (time * file->sampleRate()) + 0.5);
//fprintf(stderr, "index = %g in granAlg::setStart;   dur=%g\n", index, (float)dur);;
		endAt = startAt + dur;
		rampDown = endAt - ((slope * file->sampleRate()) + 0.5);
		if (endAt > fileEnd)
			{
			fprintf(stderr, "vss: granAlg shifting grain back within source file %g = %g - %g samples\n",
				(float)(endAt - fileEnd), (float)endAt, (float)fileEnd);
			startAt -= endAt - fileEnd;
			index = startAt;
//fprintf(stderr, "index = %g in granAlg::setStart #2 (fileend %g)\n", index, (float)fileEnd);;
			endAt = fileEnd;
			rampDown = endAt - ((slope * file->sampleRate()) + 0.5);
			}
//fprintf(stderr, "\ngranAlg::setStart t=%f (%d,%d)\n\n\n", time, startAt, endAt);;
	}
}

//	Select a new AIFF file.
void granAlg::setFile(sfile * newFile)
{
	if ( newFile == NULL )
	{
		fprintf(stderr, "vss: ignoring granAlg::setFile(NULL)\n");
		return;
	}
	
	resetFileParams();
	
//	select a sample indexing function
	switch ( newFile->sampleSize() )
	{
		case 8:
			getSampFn = &granAlg::get8bitSamp;
			sampleScale = 1. / 128.;
			break;
		case 16:
			getSampFn = &granAlg::get16bitSamp;
			sampleScale = 1. / 32767.;
			break;
		default:
			fprintf(stderr,  "vss: granActor: file %s has an unrecognized sample size.\n\n", 
					newFile->name() );
			return;
	}
	
//	set parameters
	file = newFile;
	sampleData = file->samples();
	fileNumChans = file->numChannels();
	Nchan(/*fileNumChans>0 ? fileNumChans :*/ 1);;;;
	index = 0.;
	startAt = 0;
	endAt = fileEnd = file->numFrames() - 1;
	rampDown = endAt - ((slope * file->sampleRate()) + 0.5);
	sampleStep = getSRratio();
	file->addUser(this);
}

//===========================================================================
//		granAlg resetFileParams
//
//	Clear out all the file parameters and dump any samples in memory.
//	Don't clear parameters that aren't related to a particular file.
//
void
granAlg::resetFileParams(void)
{
	if (file != NULL) file->removeUser(this);
	file = NULL;
	index = 0.;
//fprintf(stderr, "index and dur reset in granAlg::resetFileParams\n");;
	startAt = endAt = dur = 0L;
	rampDown = endAt;
	//fprintf(stderr, "granAlg::resetFileParams (%d,%d)\n\n", startAt, endAt);;
	sampleStep = 1.;
	getSampFn = NULL;
	sampleData= NULL;
	fileNumChans = 0;
	Nchan(1/*fileNumChans*/);;;;
}

//===========================================================================
//		granAlg setInterval( begin, end )
//
//	Set the begin and end times.  Times are specified in seconds.
//
void
granAlg::setInterval(float begin, float end)
{
	if (begin >= end)
	{
		fprintf(stderr, "vss: granActor's interval start cannot be later than interval end.\n");
		return;
	}
	
	startAt = (ulong)(index =
		max(0., (begin * file->sampleRate()) + 0.5 /* cheap rounding */));
	endAt = (ulong)(min(file->numFrames() - 1., (end * file->sampleRate()) + 0.5));
	rampDown = endAt - ((slope * file->sampleRate()) + 0.5);
	dur = endAt - startAt;
	//fprintf(stderr, "granAlg::setInterval (%d,%d)\n\n", startAt, endAt);;
//fprintf(stderr, "\n\n\n\nindex = %g in granAlg::setInterval\n", index);;
}

void
granAlg::setDur(float durArg /* in seconds */)
{
	if (durArg <= 0.0)
	{
		fprintf(stderr, "vss: granActor's interval duration (%g) must be positive.\n", durArg);
		return;
	}

	dur = (ulong)((durArg * file->sampleRate()) + 0.5);
	endAt = (ulong)(min(file->numFrames() - 1.,
		startAt + (durArg * file->sampleRate()) + 0.5));
	rampDown = endAt - ((slope * file->sampleRate()) + 0.5);
	//fprintf(stderr, "granAlg::setDur (%d,%d) %d\n\n", startAt, endAt, dur);;
}

void
granAlg::setSlope(float slopeArg)
{
	if (slopeArg < 0. || slopeArg > 10.)
		{
		fprintf(stderr, "vss: granActor's slope out of range (%g), overriding to zero.\n", slopeArg);
		slopeArg = 0.;
		}
	slope = slopeArg;
	rampDown = endAt - ((slope * file->sampleRate()) + 0.5);
//fprintf(stderr, "rampDown := %g\n", rampDown);;
}

void
granAlg::setRanges(float controlMin_, float controlMax_, float jumpMin_, float jumpMax_)
{
	controlMin = controlMin_;
	controlMax = controlMax_;
	jumpMin = jumpMin_;
	jumpMax = jumpMax_;
	//fprintf(stderr, "granAlg::setRanges controlMin %g\n", controlMin);;

}

static float Clamp01(float _) { return _<0. ? 0. : _>1. ? 1. : _; }

void
granAlg::setRebound(float _)
{
	rebound = Clamp01(_);
}

void
granAlg::setSpread(float _)
{
	spread = Clamp01(_);
}
