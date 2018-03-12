#ifndef _SAMP_H_
#define _SAMP_H_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"
#include "../samp/sfile.h"
#include <list>

//===========================================================================
//		granAlg 
//
//	class granAlg is an algorithm class for playing samples from AIFF files.
//
class granAlg : public VAlgorithm
{
private:
	sfile *	file;
	float	index;			// 	into sfile's sampleData
	ulong	startAt;		//	start and end indices into sampleData
	ulong 	endAt;			//		(frame indices, actually)
	ulong 	dur;			//		duration (frame index, also)
	ulong	fileEnd;		//		end of file
	float	sampleStep;		//	rate of stepping through samples
	float	sampleScale;	//	for scaling integer samples to floats
	float	slope;			//	linear ramp up/down envelope
	float	rampDown;
	int 	fStartedRampDown;
	float	controlMin,controlMax,jumpMin,jumpMax;
	float	rebound;	// % to jump back, when input is out of range
	float	spread;		// random amount to tweak startAt by
	
//	for speed, and at great expense to elegance, keep copies of these
//	sfile members, saving on extra indirection every sample.
	void *	sampleData;
	int		fileNumChans;

//	pointer to member function for indexing sampleData
	float	(granAlg::*getSampFn)(ulong, int);

//	access members
public:
	char *	getFileName(void)	{ return (file!=NULL) ? file->name() : NULL; }
	char * 	getDirectory(void)	{ return (file!=NULL) ? file->directory() : NULL; }
	float	getIntervalStart(void)	{ return (file!=NULL) ? (double) startAt / file->sampleRate() : 0.; }
	float	getIntervalEnd(void)	{ return (file!=NULL) ? (double) endAt / file->sampleRate() : 0.; }
	float	getPosition(void)	{ return (file!=NULL) ? (double) index / file->sampleRate() : 0.; }
	float	getSRratio(void)	{ return (file!=NULL) ? file->sampleRate() * globs.OneOverSR : 0.; }

//	parameter update members
	void	setStart( float );
	void	setDur( float );
	void	setSlope( float );
	void	setFile(sfile *);
	void	resetFileParams(void);
	void	setInterval( float, float );
	void	setSampleStep(float scale = 1.) { if (file != NULL) sampleStep = getSRratio() * scale; }
	void	setRanges(float controlMin_, float controlMax_, float jumpMin_, float jumpMax_);
	void	setRebound(float);
	void	setSpread(float);

virtual	float 	dampingTime(void)	{ return 0.03; }
	
//	sampleData access
private:
	float	get8bitSamp( ulong, int );
	float	get16bitSamp( ulong, int );
	float	getSamp( ulong, int = 1 );

//	sample generation
private:
	// Has samples, and isn't past the end of its samples.
	int FValidForOutput() { return file != NULL && index <= endAt; }
	void generateSamples(int);

public:
//	construction/destruction
		granAlg(void);
		~granAlg();


};	// end of class granAlg

//===========================================================================
//		granHand 
//
//	class granHand is a handler class for dumbfmAlg.
//	There are no modulating parameters for granAlg.
//
class granHand : public VHandler
{
//	granHand may be told to delete itself when the sample is
//	done playing
private:
	float myDur;
	float mySlope;
	float	controlMin,controlMax,jumpMin,jumpMax;
	void	setRanges( void );

//	keep track of default directory for loading files
	char 	directoryName[180];

//  Algorithm access:
//  Define a version of getAlg() that returns a pointer to granAlg
protected:
    granAlg * getAlg(void) { return (granAlg *) VHandler::getAlg(); }
	
//	the sampleStep in the algorithm accounts for the ratio
//	of the vss sample rate and the file's sample rate. The handler's
//	sampleStep scales that value.
	float sampleStep;

//	parameter access
public:
	char *	getFileName(void)	{ return getAlg()->getFileName(); }
	char * 	getDirectory(void)	{ return getAlg()->getDirectory(); }
	float	getIntervalStart(void)	{ return getAlg()->getIntervalStart(); }
	float	getIntervalEnd(void)	{ return getAlg()->getIntervalEnd(); }

//	message handling
	void	setDirectory(char *);
	void	setFile(char *);
	void	setStart( float );
	void	setDur( float );
	void	setInterval( float, float );
	void	setSlope( float );
	void	setRebound(float);
	void	setSpread(float);
	void	setSampleStep(float);

//	construction/destruction
		granHand(granAlg * alg = new granAlg);
virtual	~granHand() {}

//	actor behavior
	void	act(void);
	int		receiveMessage(const char * Message);
	
};	// end of class granHand

//===========================================================================
//		granActor
//
//	class granActor is a generator actor class for granAlg
//
class granActor : public VGeneratorActor
{
//	construction/destruction
public:
	granActor(void);
virtual	~granActor();

virtual VHandler * newHandler(void) { return new granHand(); }

virtual	void 	sendDefaults(VHandler *);
virtual int		receiveMessage(const char * Message);

//	message handling members
	void	rewindAll(void);
	void	setAllDirectory(char *);
	void	setDirectory(char *);
	void	setStep(float);

//	default parameters
private:
	float	myDur, myStart, mySlope;
	float	controlMin,controlMax,jumpMin,jumpMax;
	float	rebound;
	float	spread;
	char 	defaultDirectory[180];
	float	defaultSampleStep;
	
//	maintain a list of sample files in memory
	typedef list<sfile *> SfileList;
	SfileList	fileList;
	
public:
	sfile *	loadFile(char *, char *);
	sfile *	loadFile(char * fname)		{ return loadFile(defaultDirectory, fname); }
	void	unloadFile(char *, char *);
	void	unloadFile(char * fname)	{ unloadFile(defaultDirectory, fname); }
	void	unloadAllFiles(int = 0);

	void	setStart( float );
	void	setDur( float );
	void	setInterval( float, float );
	void	setSlope( float );
	void	setControlMin( float );
	void	setControlMax( float );
	void	setJumpMin( float );
	void	setJumpMax( float );
	void	setRebound(float);
	void	setSpread(float);

	float Start(void) { return myStart; }
	float Dur(void) { return myDur; }
	float Slope(void) { return mySlope; }
	float ControlMin(void) { return controlMin; }
	float ControlMax(void) { return controlMax; }
	float JumpMin(void) { return jumpMin; }
	float JumpMax(void) { return jumpMax; }
	float Rebound(void) { return rebound; }
	float Spread(void) { return spread; }

};	// end of class granActor

//	Bounds checking.
//
static	inline	int	CheckSampleStep(float f) 	{ return f > 0. && f < 1000.; }

#endif // ndef _SAMP_H_
