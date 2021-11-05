#include "gran.h"

granAlg::granAlg() :
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
	Nchan(1/*fileNumChans*/);;;;
}

granAlg::~granAlg()
{
	if (file)
		file->removeUser(this);
}

inline float granAlg::get8bitSamp(ulong frame, int chan)
{
	return ((char*)sampleData)[chan + (frame * fileNumChans)];
}

inline float granAlg::get16bitSamp(ulong frame, int chan)
{
	return ((short*)sampleData)[chan + (frame * fileNumChans)];
}

inline float granAlg::getSamp(ulong frame, int chan)
{
	if (!getSampFn) {
		fprintf(stderr, "vss: granAlg's getSampFn is uninitialized.\n");
		return 0.0;
	}
	if (chan > fileNumChans) {
		fprintf(stderr, "vss: granAlg's file has only %d channels, not %d.\n", fileNumChans, chan);
		return 0.0;
	}
	return (this->*getSampFn)(frame, chan);
}

//	# of output channels is the # of channels in the input file.
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

bool granAlg::noFile() const {
	if (!file || file->sampleRate() == 0) {
		fprintf(stderr, "vss: granAlg has no file.");
		return true;
	}
	return false;
}

//	Start the grain at the specified fraction 0..1 into the source file.
//	Keep the duration intact, by adjusting the ending time of the grain
//	to match.
void granAlg::setStart(float time)
{
	if (noFile())
		return;

	//  input time: controlMin..controlMax.
	// output time: 0..1.
	time = (time - controlMin) / (controlMax - controlMin);
	time += spread * (drand48() * 2. - 1.);

	// If out of range, clamp and also rebound back a little bit.
	if (time < 0.)
		time = drand48() * rebound;
	else if (time > 1.)
		time = 1. - drand48() * rebound;

	//  input time: 0..1.
	// output time: jumpMin..jumpMax.
	time = jumpMin + (jumpMax-jumpMin) * time;

	//  input time: jumpMin..jumpMax (which is a subset of 0..1)
	// output time: seconds // (well, samples actually).
	time *= (float)fileEnd / file->sampleRate();

	if (time < 0.0) {
		fprintf(stderr, "vss: granAlg ignoring jump before the start of the file.\n");
		return;
	}

	startAt = index = (time * file->sampleRate()) + 0.5;
	endAt = startAt + dur;
	if (endAt > fileEnd)
		{
		fprintf(stderr, "vss: granAlg shifting grain back %g = %g - %g samples\n",
			(float)(endAt - fileEnd), (float)endAt, (float)fileEnd);
		startAt -= endAt - fileEnd;
		index = startAt;
		endAt = fileEnd;
		}
	rampDown = endAt - ((slope * file->sampleRate()) + 0.5);
}

//	Select a new AIFF file.
void granAlg::setFile(sfile * newFile)
{
	if (!newFile)
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

// Clear parameters related to this file.  Free any samples in memory.
void granAlg::resetFileParams()
{
	if (file)
		file->removeUser(this);
	file = NULL;
	index = 0.;
	startAt = endAt = dur = 0L;
	rampDown = endAt;
	sampleStep = 1.;
	getSampFn = NULL;
	sampleData= NULL;
	fileNumChans = 0;
	Nchan(1/*fileNumChans*/);;;;
}

void granAlg::setInterval(float begin, float end /* in seconds */)
{
	if (begin >= end)
	{
		fprintf(stderr, "vss: granAlg ignoring reversed interval.\n");
		return;
	}
	if (noFile())
		return;
	
	startAt = index = std::max(0., (begin * file->sampleRate()) + 0.5 /* cheap rounding */);
	endAt = std::min(file->numFrames() - 1., (end * file->sampleRate()) + 0.5);
	rampDown = endAt - ((slope * file->sampleRate()) + 0.5);
	dur = endAt - startAt;
}

void
granAlg::setDur(float durArg /* in seconds */)
{
	if (durArg <= 0.0) {
		fprintf(stderr, "vss: granAlg ignoring nonpositive duration %g.\n", durArg);
		return;
	}
	if (noFile())
		return;
	dur = (durArg * file->sampleRate()) + 0.5;
	endAt = std::min(file->numFrames() - 1.,
		startAt + (durArg * file->sampleRate()) + 0.5);
	rampDown = endAt - ((slope * file->sampleRate()) + 0.5);
}

void
granAlg::setSlope(float slopeArg)
{
	if (slopeArg < 0. || slopeArg > 10.) {
		fprintf(stderr, "vss: granAlg zeroing out of range slope %g.\n", slopeArg);
		slopeArg = 0.;
		}
	if (noFile())
		return;
	slope = slopeArg;
	rampDown = endAt - ((slope * file->sampleRate()) + 0.5);
}

void
granAlg::setRanges(float controlMin_, float controlMax_, float jumpMin_, float jumpMax_)
{
	controlMin = controlMin_;
	controlMax = controlMax_;
	jumpMin = jumpMin_;
	jumpMax = jumpMax_;
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
