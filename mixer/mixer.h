#pragma once

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"
#include "VFloatArray.h"

const auto MaxNumInput = 8;
const auto MaxNumInput2 = 64;

// Processor algorithm that mixes multiple inputs.
class mixerAlg: public VAlgorithm
{
	int	channelNum;
	int	numInputs;
	bool matrix;
	float fader[MaxNumInput]; // linear fader value
	float faderm[MaxNumInput][MaxNumInput];
	VAlgorithm*	source[MaxNumInput];

public:
	float*	getFaderAmp() { return fader; }
	float*	getMatrixAmp() { return faderm[0]; }
	int		getNumInputs() const { return numInputs; }
	bool	getMatrix() const { return matrix; }
	int		getCurrentChannel() const { return channelNum; }

	void	setMatrixMode(bool z=false);
	void	setMatrixAmp(float* z);
	void	setChannelNum(int num) { channelNum = num; }
	void	setOneInput(VAlgorithm* alg) { source[channelNum] = alg; }
	void	setOneFaderAmp(float lin) { fader[channelNum] = lin; }

	void	setNumInputs(int num) { numInputs = num; }
	void	setAllFaderAmp(float* lin)
			{ for (int i=0; i<numInputs; i++)
				if (lin[i]!=1000) fader[i]=lin[i];
			}

	void generateSamples(int);
	mixerAlg();
	~mixerAlg();
};

// Handler for mixerAlg.
class mixerHand : public VHandler
{
//	modulating parameters of mixerAlg
	FloatArray<MaxNumInput, mixerAlg>	allFaderAmp;
	FloatArray<MaxNumInput2, mixerAlg>	allMatrixAmp;
	int	myChannelNum;
	VHandler* myHandlers[MaxNumInput];
	int	numInput;
	bool matrix;
	float faderm[MaxNumInput2];

protected:
	mixerAlg* getAlg() { return (mixerAlg*)VHandler::getAlg(); }

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

	void actCleanup();

	mixerHand(mixerAlg* alg = new mixerAlg);
	~mixerHand() {}
	int receiveMessage(const char*);
};

// Processor actor for mixerAlg.
class mixerActor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new mixerHand(); }

	mixerActor();
	~mixerActor() {}

	void sendDefaults(VHandler*);
	int receiveMessage(const char*);

	void	setFaderAmp(float f);
	void	setFaderGain(float f);
	void	setAllFaderAmp(float f, float t = 0.);
	void	setAllFaderGain(float f, float t = 0.);

protected:
	float	defaultFaderAmp;
};

static inline int	CheckFaderGain(float f) 	{ return f <= 42. || f == 1000.; }
static inline int	CheckFaderAmp(float f)
				{ return (f >= -128. && f <= 128.) || f == 1000.; }
static inline int	CheckChannelNum(int f)
			 	{ return (f >= 1 && f <= MaxNumInput) || f == -1;}
