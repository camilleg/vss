#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"
#include "../samp/sfile.h"
#include <list>
// Has much in common with samp/samp.h.

// Play grains from AIFF files.
class granAlg : public VAlgorithm
{
	sfile*	file;
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
	void*	sampleData;
	int		fileNumChans;

//	pointer to member get8bitSamp or get16bitSamp.
	float	(granAlg::*getSampFn)(ulong, int);

public:
	char* getFileName() const { return file ? file->name() : nullptr; }
	char* getDirectory() const { return file ? file->directory() : nullptr; }
	float getIntervalStart() const { return file ? startAt / file->sampleRate() : 0.; }
	float getIntervalEnd() const { return file ? endAt / file->sampleRate() : 0.; }
	float getPosition() const { return file ? index / file->sampleRate() : 0.; }
	float getSRratio() const { return file ? file->sampleRate() * globs.OneOverSR : 0.; }

	void	setStart( float );
	void	setDur( float );
	void	setSlope( float );
	void	setFile(sfile *);
	void	resetFileParams();
	void	setInterval( float, float );
	void	setSampleStep(float scale = 1.) { if (file) sampleStep = getSRratio() * scale; }
	void	setRanges(float controlMin_, float controlMax_, float jumpMin_, float jumpMax_);
	void	setRebound(float);
	void	setSpread(float);

	float dampingTime() { return 0.03; }
	
private:
	float	get8bitSamp( ulong, int );
	float	get16bitSamp( ulong, int );
	float	getSamp( ulong, int = 1 );

	// Has samples, and isn't past the end of its samples.
	int FValidForOutput() { return file != NULL && index <= endAt; }

	bool noFile() const;
	void generateSamples(int);

public:
	granAlg();
	~granAlg();
};

class granHand : public VHandler
{
	float myDur;
	float mySlope;
	float controlMin,controlMax,jumpMin,jumpMax;
	void setRanges();

	// Default directory for loading files.
	char 	directoryName[180];

protected:
	granAlg* getAlg() { return (granAlg*)VHandler::getAlg(); }
	
//	the sampleStep in the algorithm accounts for the ratio
//	of the vss sample rate and the file's sample rate. The handler's
//	sampleStep scales that value.
	float sampleStep;

public:
	char* getFileName() { return getAlg()->getFileName(); }
	char* getDirectory() { return getAlg()->getDirectory(); }
	float getIntervalStart() { return getAlg()->getIntervalStart(); }
	float getIntervalEnd() { return getAlg()->getIntervalEnd(); }

	void setDirectory(char*);
	void setFile(char*);
	void setStart(float);
	void setDur(float);
	void setInterval(float, float);
	void setSlope(float);
	void setRebound(float);
	void setSpread(float);
	void setSampleStep(float);

	granHand(granAlg* alg = new granAlg);
	~granHand() {}
	void act();
	int receiveMessage(const char*);
};

class granActor : public VGeneratorActor
{
public:
	granActor();
	~granActor();
	VHandler* newHandler() { return new granHand(); }
	void sendDefaults(VHandler*);
	int receiveMessage(const char*);

	// message handling
	void rewindAll();
	void setAllDirectory(char *);
	void setDirectory(char *);
	void setStep(float);

private:
	// default parameters
	float	myDur, myStart, mySlope;
	float	controlMin,controlMax,jumpMin,jumpMax;
	float	rebound;
	float	spread;
	char 	defaultDirectory[180];
	float	defaultSampleStep;
	
	using SfileList = std::list<sfile*>;
	SfileList fileList;
	
public:
	sfile*	loadFile(char*, char*);
	sfile*	loadFile(char* fname) { return loadFile(defaultDirectory, fname); }
	void	unloadFile(char*, char*);
	void	unloadFile(char* fname) { unloadFile(defaultDirectory, fname); }
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

	float Start() const { return myStart; }
	float Dur() const { return myDur; }
	float Slope() const { return mySlope; }
	float ControlMin() const { return controlMin; }
	float ControlMax() const { return controlMax; }
	float JumpMin() const { return jumpMin; }
	float JumpMax() const { return jumpMax; }
	float Rebound() const { return rebound; }
	float Spread() const { return spread; }
};

static	inline	int	CheckSampleStep(float f) 	{ return f > 0. && f < 1000.; }
