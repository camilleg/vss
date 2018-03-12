#ifndef _MIXER_H_
#define _MIXER_H_

#include <cmath>

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"
#include "VFloatArray.h"

#define MaxNumInput	8
#define MaxNumInput2 64

//===========================================================================
//		mixerAlg 
//
//	class mixerAlg is a processor algorithm that mixes
//	multiple input sources
//

class mixerAlg : public VAlgorithm
{
private:
//	mixer parameters
	int	channelNum;
	int	numInputs;
	int	matrix;
	float fader[MaxNumInput];		// linear fader value
	float faderm[MaxNumInput][MaxNumInput];
	VAlgorithm*	source[MaxNumInput];

//	access members
public:
	float *	getFaderAmp(void)	{ return fader; }
	float *	getMatrixAmp(void)	{ return faderm[0]; }
	int		getNumInputs(void)	{ return numInputs; }
	int		getMatrix(void)	{ return matrix; }
	int		getCurrentChannel(void)	{ return channelNum; }

//	parameter update members
	void	setMatrixMode(int z=0);
	void	setMatrixAmp(float * z);
	void	setChannelNum(int num)	{ channelNum = num; } 
	void	setOneInput(VAlgorithm * alg) { source[channelNum] = alg; }
	void	setOneFaderAmp(float lin) { fader[channelNum] = lin; }

	void	setNumInputs(int num) { numInputs = num; }
	void	setAllFaderAmp(float * lin)
			{ for (int i=0; i<numInputs; i++)
				if (lin[i]!=1000) fader[i]=lin[i];
			}

//	sample generation
	void	generateSamples(int);

//	construction/destruction
	mixerAlg(void);
	~mixerAlg();

};	// end of class mixerAlg

//===========================================================================
//		mixerHand 
//
//	class mixerHand is a handler class for mixerAlg.
//
class mixerHand : public VHandler
{
//	modulating parameters of mixerAlg
private:
	FloatArray<MaxNumInput, mixerAlg>	allFaderAmp;
	FloatArray<MaxNumInput2, mixerAlg>	allMatrixAmp;
	int	myChannelNum;
	VHandler* myHandlers[MaxNumInput];
	int	numInput;
	int matrix;
	float faderm[MaxNumInput2];

//	Algorithm access:
// 	Define a version of getAlg() that returns a pointer to mixerAlg
protected:
	mixerAlg * getAlg(void)	{ return (mixerAlg *) VHandler::getAlg(); }

//	parameter modulation
public:
	void	setMatrixMode(int z);
	void    setMatrixAmp(int dir, int chan, int num, float * z, float t=0.);

	void	setOneChannelAmp(int num, float hSrc, float z, float t=0.);
	void	setOneChannelGain(int num, float hSrc, float z, float t=0.);

	void	setChannelNum(int num);
	void	setOneFaderAmp(int num, float z, float t = 0.);
	void	setOneFaderGain(int num, float z, float t = 0.);

	void	setNumInputs(int num);
	void	setAllFaderAmp(int num, float * z, float t=0.);
	void	setAllFaderGain(int num, float * z, float t=0.);
	
//	assigning sources for mixing
	void	setOneInput(float);
	void	setOneInput(void);
	void	setAllInputs(int num, float * z);
	void	setAllInputs(void);

//	actor behavior
	virtual void actCleanup(void);

//	construction
	mixerHand(mixerAlg * alg = new mixerAlg);
		
//	destruction
	virtual	~mixerHand() {}

//	message reception
	int receiveMessage(const char * Message);

};	// end of class mixerHand

//===========================================================================
//		mixerActor
//
//	class mixerActor is a processor actor class for mixerAlg
//
class mixerActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new mixerHand(); }

//	construction/destruction
public:
	mixerActor(void);
virtual	~mixerActor() {}

virtual	void	sendDefaults(VHandler *);
virtual int	receiveMessage(const char * Message);

//	parameter setting members
	void	setFaderAmp(float f);
	void	setFaderGain(float f);
	void	setAllFaderAmp(float f, float t = 0.);
	void	setAllFaderGain(float f, float t = 0.);

//	default parameters
protected:
	float	defaultFaderAmp;

};	// end of class mixerActor

//===========================================================================
//	BOUNDS CHECKING IS VITAL TO OUR SURVIVAL!!!!!!!!!!!!!!!!!!!
//
//	Find reasonable bounds and enforce them.
//
static inline int	CheckFaderGain(float f) 	{ return f <= 42. || f == 1000.; }
static inline int	CheckFaderAmp(float f)
				{ return (f >= -128. && f <= 128.) || f == 1000.; }
static inline int	CheckChannelNum(int f)
			 	{ return (f >= 1 && f <= MaxNumInput) || f == -1;}

#endif // ndef _MIXER_H_
