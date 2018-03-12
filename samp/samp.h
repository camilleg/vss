#ifndef _SAMP_H_
#define _SAMP_H_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"
#include "sfile.h"
#include <list>

//===========================================================================
//		sampAlg 
//
//	class sampAlg is an algorithm class for playing AIFF sample files.
//
class sampAlg : public VAlgorithm
{
private:
	sfile *	file;
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
	void *	sampleData;
	int	fileNumChans;

//	pointer to member function for indexing sampleData
	float	(sampAlg::*getSampFn)(ulong, int);

//	access members
public:
	char *	getFileName(void)	{ return (file!=NULL) ? file->name() : NULL; }
	char * 	getDirectory(void)	{ return (file!=NULL) ? file->directory() : NULL; }
	float	getStart(void)		{ return (file!=NULL) ? (double) startAt / file->sampleRate() : 0.; }
	float	getEnd(void)		{ return (file!=NULL) ? (double) endAt / file->sampleRate() : 0.; }
	float	getLoopStart(void)	{ return (file!=NULL) ? (double) startLoopAt / file->sampleRate() : 0.; }
	float	getLoopEnd(void)	{ return (file!=NULL) ? (double) endLoopAt / file->sampleRate() : 0.; }
	int	getLoop(void)		{ return loop; }
	float	getPosition(void)	{ return (file!=NULL) ? (double) index / file->sampleRate() : 0.; }
	float	getSRratio(void)	{ return (file!=NULL) ? file->sampleRate() * globs.OneOverSR : 0.; }

//	parameter update members
	void	jumpTo( float );
	void	setFile(sfile *);
	void	resetFileParams(void);
	void	setBounds( float, float );
	void	setLoop( float, float, int = 1 );
	void	setLoop( int );
	void	setSampleStep( float scale ) { if (file != NULL) sampleStep = getSRratio() * scale; }

virtual	float 	dampingTime(void)	{ return 0.03; }
	
//	sampleData access
private:
	float	get8bitSamp( ulong, int );
	float	get16bitSamp( ulong, int );
	float	getSamp( ulong, int = 1 );

public:

//	construction/destruction
		sampAlg(void);
		~sampAlg();

//	sample generation
private:
	//	Has samples, and isn't past the end of its samples.
	int FValidForOutput() { return file != NULL && index <= (float)endAt; }
	void generateSamples(int howMany);

};	// end of class sampAlg

//===========================================================================
//		sampHand 
//
//	class sampHand is a handler class for sampAlg.
//	There are no modulating parameters for sampAlg.
//
class sampHand : public VHandler
{
//	sampHand may be told to delete itself when the sample is
//	done playing
private:
	int		deleteWhenDone;
	float 	hMGDelete; // message group to inform of self-deletion
	float zhMGDeleteData; // user data for hMGDelete

//	keep track of default directory for loading files
	char 	directoryName[180];

//  Algorithm access:
//  Define a version of getAlg() that returns a pointer to sampAlg
protected:
	sampAlg * getAlg(void) { return (sampAlg *) VHandler::getAlg(); }
	
//	the only smoothly modulating parameter is the sampleStep
//	the sampleStep in the algorithm accounts for the ratio
//	of the vss sample rate and the file's sample rate. The handler's
//	sampleStep scales that value.
	float sampleStep;

	enum { isetSampleStep };
	
//	parameter access
public:
	char *	getFileName(void)	{ return getAlg()->getFileName(); }
	char * 	getDirectory(void)	{ return getAlg()->getDirectory(); }
	float	getStart(void)		{ return getAlg()->getStart(); }
	float	getEnd(void)		{ return getAlg()->getEnd(); }
	float	getLoopStart(void)	{ return getAlg()->getLoopStart(); }
	float	getLoopEnd(void)	{ return getAlg()->getLoopEnd(); }
	int	getLoop(void)		{ return getAlg()->getLoop(); }

	void	setDeleteWhenDone(int d=1, float h=hNil, float data=0.) { deleteWhenDone = d; hMGDelete=h; zhMGDeleteData=data; }
	
//	message handling
	void	SetAttribute(IParam iParam, float z);
	void	jumpTo( float );
	void	setDirectory(char *);
	void	setFile(char *);
	void	setBounds( float, float );
	void	setLoop( float, float, float = 1. );
	void	setLoop( float );
	void	setSampleStep( float z, float t = timeDefault )
		{ modulate(isetSampleStep, sampleStep, z, AdjustTime(t)); }

//	construction/destruction
	sampHand(sampAlg * alg = new sampAlg);
virtual	~sampHand() {}

//	actor behavior
	void	act(void);
	int	receiveMessage(const char * Message);
	
//	override restrike to rewind the file
	void	restrike(char * inits_msg);

};	// end of class sampHand

//===========================================================================
//		sampActor
//
//	class sampActor is a generator actor class for sampAlg
//
class sampActor : public VGeneratorActor
{
//	construction/destruction
public:
	sampActor(void);
virtual	~sampActor();

virtual VHandler * newHandler(void) { return new sampHand(); }

virtual	void 	sendDefaults(VHandler *);
virtual int	receiveMessage(const char * Message);

//	message handling members
	void	playSample(char *);
	void	rewindAll(void);
	void	setAllDirectory(char *);
	void	setDirectory(char *);
	void	setAllBounds(float, float);
	void	setAllLoop(int);
	void	setAllLoop(float, float, int = 1);
	void	setLoop(float);
	void	setStep(float);
	void	setAllStep(float, float = 0.);

//	default parameters
private:
	char 	defaultDirectory[180];
	int	defaultLoopFlag;
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

};	// end of class sampActor

//===========================================================================
//	BOUNDS CHECKING IS VITAL TO OUR SURVIVAL!!!!!!!!!!!!!!!!!!!
//
//	Find reasonable bounds and enforce them.
//
static inline int	CheckSampleStep(float f) 	{ return ((f >-1000.0) &&
                                                          (f < 1000.0)); }

#endif // ndef _SAMP_H_
