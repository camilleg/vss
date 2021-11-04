#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"
#include "sfile.h"
#include <list>
// Has much in common with gran/gran.h.

// Play AIFF files.
class sampAlg : public VAlgorithm
{
	sfile*	file;
	double	index;	//DZ		// 	into sfile's sampleData
	ulong	startAt;		//	basic playback start and end indices
	ulong 	endAt;			//		(frame indices, actually)
	ulong	startLoopAt;		//	looping start and end indices
	ulong 	endLoopAt;		//		(frame indices, actually)
	float	sampleStep;		//	rate of stepping through samples
	float	sampleScale;		//	for scaling integer samples to floats
	int	loop;			//	flag: loop or not
	
//	for speed, and at great expense to elegance, keep copies of these
//	sfile members, saving extra indirection every sample.
	void* sampleData;
	int	fileNumChans;

//	pointer to member function for indexing sampleData
	float	(sampAlg::*getSampFn)(ulong, int);

public:
	char* getFileName()	const { return file ? file->name() : nullptr; }
	char* getDirectory() const { return file ? file->directory() : nullptr; }
	float getStart() const { return file ? startAt / file->sampleRate() : 0.; }
	float getEnd() const { return file ? endAt / file->sampleRate() : 0.; }
	float getLoopStart() const { return file ? startLoopAt / file->sampleRate() : 0.; }
	float getLoopEnd() const { return file ? endLoopAt / file->sampleRate() : 0.; }
	int	getLoop() const { return loop; }
	float getPosition() const { return file ? index / file->sampleRate() : 0.; }
	float getSRratio() const { return file ? file->sampleRate() * globs.OneOverSR : 0.; }

	void	jumpTo( float );
	void	setFile(sfile *);
	void	resetFileParams();
	void	setBounds( float, float );
	void	setLoop( float, float, int = 1 );
	void	setLoop( int );
	void	setSampleStep( float scale ) { if (file) sampleStep = getSRratio() * scale; }

	float dampingTime()	{ return 0.03; }
	
	//	sampleData access
private:
	float	get8bitSamp( ulong, int );
	float	get16bitSamp( ulong, int );
	float	getSamp( ulong, int = 1 );

public:
		sampAlg();
		~sampAlg();
private:
	//	Has samples, and isn't past the end of its samples.
	int FValidForOutput() { return file != NULL && index <= (float)endAt; }
	void generateSamples(int howMany);
};

// May be told to delete itself when sample playback ends.
class sampHand : public VHandler
{
	int		deleteWhenDone;
	float 	hMGDelete; // message group to inform of self-deletion
	float zhMGDeleteData; // user data for hMGDelete

	// Default directory for loading files.
	char 	directoryName[180];

protected:
	sampAlg* getAlg() { return (sampAlg*)VHandler::getAlg(); }
	
	// Scale the ratio of the vss sample rate to the file's sample rate.
	float sampleStep;
	enum { isetSampleStep };
	
public:
	char* getFileName() { return getAlg()->getFileName(); }
	char* getDirectory() { return getAlg()->getDirectory(); }
	float getStart() { return getAlg()->getStart(); }
	float getEnd() { return getAlg()->getEnd(); }
	float getLoopStart() { return getAlg()->getLoopStart(); }
	float getLoopEnd() { return getAlg()->getLoopEnd(); }
	int	getLoop() { return getAlg()->getLoop(); }

	void	setDeleteWhenDone(int d=1, float h=hNil, float data=0.) { deleteWhenDone = d; hMGDelete=h; zhMGDeleteData=data; }
	
	void SetAttribute(IParam iParam, float z);
	void jumpTo(float);
	void setDirectory(char*);
	void setFile(char*);
	void setBounds(float, float);
	void setLoop(float, float, float = 1.);
	void setLoop(float);
	void setSampleStep(float z, float t = timeDefault)
		{ modulate(isetSampleStep, sampleStep, z, AdjustTime(t)); }

	sampHand(sampAlg * alg = new sampAlg);
	~sampHand() {}
	void act();
	int receiveMessage(const char*);
	
	// Rewind the file.
	void restrike(char*);
};

class sampActor : public VGeneratorActor
{
public:
	sampActor();
	~sampActor();
	VHandler* newHandler() { return new sampHand(); }
	void sendDefaults(VHandler*);
	int receiveMessage(const char*);

	void	playSample(char*);
	void	rewindAll();
	void	setAllDirectory(char*);
	void	setDirectory(char*);
	void	setAllBounds(float, float);
	void	setAllLoop(int);
	void	setAllLoop(float, float, int = 1);
	void	setLoop(float);
	void	setStep(float);
	void	setAllStep(float, float = 0.);

private:
	char 	defaultDirectory[180];
	int	defaultLoopFlag;
	float	defaultSampleStep;

    using SfileList = std::list<sfile*>;
	SfileList fileList;
	
public:
	sfile* loadFile(char*, char*);
	sfile* loadFile(char* fname) { return loadFile(defaultDirectory, fname); }
	void unloadFile(char*, char*);
	void unloadFile(char* fname) { unloadFile(defaultDirectory, fname); }
	void unloadAllFiles(int = 0);
};

static inline int	CheckSampleStep(float f) 	{ return ((f >-1000.0) &&
                                                          (f < 1000.0)); }
