#ifndef _REVERB_H_
#define _REVERB_H_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//===========================================================================
//	Non-interpolating delay line, copied from stk/DLineN.c++
//
class DLineNcopy
{
  protected:  
	long inPoint;
	long outPoint;
	long length;
	long lengthm1;
	float * inputs;
	float lastOutput;

  public:
	DLineNcopy(long len);  
	~DLineNcopy();  
	void clear();
	void setDelay(float lagSec);
	float tick(float sample);
	float lastOut() { return lastOutput; }
	int isprime(int val);
};

enum { MaxEarlyRefNum = 6, MaxCombNum = 6, MaxAllPassNum = 3 };
enum { DEFAULT, SMALLROOM, HALL, ECHO1, CANYON, MAXPRE };
#define ParaNum 44

//===========================================================================
//		reverbAlg 
//

class reverbAlg : public VAlgorithm
{
private:
//	synthesis parameters
	DLineNcopy *delayLineEarlyRef[MaxEarlyRefNum];
	DLineNcopy *delayLineComb[MaxCombNum];
	DLineNcopy *delayLineAllPass[MaxAllPassNum];
	float revMix;
	float revGain;
	float pole;
	float BW;
	float lastin1, lastin2;
	float lastout1, lastout2;
	int earlyRefNum;
	float earlyRefDelay[MaxEarlyRefNum];
	float earlyRefCoeff[MaxEarlyRefNum];
	int combNum;
	float MaxT60, t60;
	float dampRatio;
	float combDelay[MaxCombNum]; 
	float combCoeff[MaxCombNum];
	float combDamp[MaxCombNum];
	float combLastOut[MaxCombNum];
	int allPassNum;
	float allPassDelay[MaxAllPassNum];
	float allPassCoeff[MaxAllPassNum];
	int idle;

public:
//	access members
	void clear();
	int getEarlyRefNum(void) { return earlyRefNum; }
	float * getEarlyRefDelay(void) { return earlyRefDelay; }
	float * getEarlyRefCoeff(void) { return earlyRefCoeff; } 
	int getCombNum(void) { return combNum; }
	float * getCombDelay(void) { return combDelay; }
	int getAllPassNum(void) {return allPassNum; }
	float * getAllPassDelay(void) {return allPassDelay; }

//	parameter update members
	void setPreset(float z);
	void setPara(float * p);
	void setRevMix(float mix) { revMix = mix; }
	void setRevGain(float g) { revGain = g; }
	void setPole(float p) { pole = p; }
	void setBW(float bw) { BW = bw; }
	void setEarlyRefNum(float n) { earlyRefNum = int(n); }
	void setEarlyRefDelay(float * delay);
	void setEarlyRefCoeff(float * coeff);
	void setCombNum(float n) { combNum = int(n); }
	void setComb(void);
	void setCombDelay(float * delay);
	void setT60(float t) { t60 = t*MaxT60; this->setComb(); }
	void setDampRatio(float dr) { dampRatio = dr; this->setComb(); }
	void setAllPassNum(float n) { allPassNum = int(n); }
	void setAllPassDelay(float * delay);
	void setIdle(int i) { idle = i; }

//	sample generation
	void generateSamples(int);
	float tick(float input);
	int FValidForOutput() { return source != NULL; }

//	construction/destruction
	reverbAlg(void);
	~reverbAlg();

};	// end of class reverbAlg

//===========================================================================
//		reverbHand 
//
//	class reverbHand is a handler class for reverbAlg.
//
class reverbHand : public VHandler
{
//	modulating parameters of reverbAlg
private:
	float revMix;
	float revGain;
	float t60; // revTime
	float BW; // revBright
	float pole; // revPole
	float dampRatio;

	enum {
		isetRevMix,
		isetRevGain,
		isetRevTime,
		isetRevBright,
		isetRevPole,
		isetDampRatio };

//	Algorithm access:
// 	Define a version of getAlg() that returns a pointer to reverbAlg.
protected:
	reverbAlg * getAlg(void) { return (reverbAlg *) VHandler::getAlg(); }

//	parameter modulation
public:
	void SetAttribute(IParam iParam, float z);

	void setRevMix(float z, float t = timeDefault)
		{ modulate(isetRevMix, revMix, z, AdjustTime(t)); }
	void setRevGain(float z, float t = timeDefault)
		{ modulate(isetRevGain, revGain, z, AdjustTime(t)); }
	void setRevTime(float z, float t = timeDefault)
		{ modulate(isetRevTime, t60, z, AdjustTime(t)); }
	void setRevBright(float z, float t = timeDefault)
		{ modulate(isetRevBright, BW, z, AdjustTime(t)); }
	void setRevPole(float z, float t = timeDefault)
		{ modulate(isetRevPole, pole, z, AdjustTime(t)); }
	void setDampRatio(float z, float t = timeDefault)
		{ modulate(isetDampRatio, dampRatio, z, AdjustTime(t)); }
	
//	parameter setting
	void setIdle(float z);
	void setPreset(char * pre);
	void setPresetFile(char * pre);
	void setPresetNum(int pre);
	void setEarlyRefNum(int z);
	void setEarlyRefMix(float z);
	void setEarlyRefDelay(int cz, float* rgz);
	void setEarlyRefCoeff(int cz, float* rgz);
	void setCombNum(int z);
	void setCombDelay(int cz, float* rgz);
	void setAllPassNum(int z);
	void setAllPassDelay(int cz, float* rgz);

	virtual void actCleanup(void);

//	damp amplitude changes
	float	dampingTime(void) { return 0.03; }

//	construction
	reverbHand(reverbAlg * alg = new reverbAlg);
		
//	destruction
virtual	~reverbHand() {}

	int receiveMessage(const char * Message);

};	// end of class reverbHand

//===========================================================================
//		reverbActor
//
class reverbActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new reverbHand(); }

//	construction/destruction
public:
	reverbActor(void);
virtual	~reverbActor() {}

virtual	void sendDefaults(VHandler *);
virtual int receiveMessage(const char * Message);

//	parameter setting members
	void setRevTime(float f);
	void setAllRevTime(float f, float t = 0.);
	void setRevMix(float f);
	void setAllRevMix(float f, float t = 0.);
	void setRevGain(float f);
	void setAllRevGain(float f, float t = 0.);
	void setRevBright(float f);
	void setAllRevBright(float f, float t = 0.);
	void setRevPole(float f);
	void setAllRevPole(float f, float t = 0.);
	void setDampRatio(float f);
	void setAllDampRatio(float f, float t = 0.);

//	default parameters
protected:
	float defaultRevTime;
	float defaultRevMix;
	float defaultRevBright;
	float defaultRevGain;
	float defaultRevPole;
	float defaultDampRatio;

};	// end of class reverbActor

//===========================================================================
//	BOUNDS CHECKING IS VITAL TO OUR SURVIVAL!!!!!!!!!!!!!!!!!!!
//
//	Find reasonable bounds and enforce them.
//
//	The following places have bounds information:
//		The following Check*
//		Error messages giving valid range in set* in reverbHand
//		Parameter table in documentation
//		Actor/handler messages in documentation
//		Preset, default, delayline values in reverbAlg if needed
//		Preset files if needed
 
static inline int CheckRevMix(float f) { return ((f >= 0.) && (f <= 1.)); }
static inline int CheckRevGain(float f) { return ((f >= 0.) && (f <= 10.)); }
static inline int CheckRevTime(float f) { return ((f > 0.) && (f <= 1.)); }
static inline int CheckRevBright(float f) { return ((f > 0.) && (f <= 1.)); }
static inline int CheckRevPole(float f) { return ((f >= -.5) && (f <= .5)); }
static inline int CheckDampRatio(float f) { return ((f >= 1.) && (f <= 30.)); }

static inline int CheckEarlyRefNum(int f) 
	{ return ((f >= 0) && (f <= MaxEarlyRefNum)); }
static inline int CheckEarlyRefDelay(float f) 
	{ return (f >= 0.1 && f <= 500.); }
static inline int CheckEarlyRefCoeff(float f) 
	{ return (f >= 0. && f <= 1.); }

static inline int CheckCombNum(int f) 
	{ return ((f >= 0) && (f <= MaxCombNum)); }
static inline int CheckCombDelay(float f) 
	{ return ((f >= 0.1) && (f <= 100.)); }

static inline int CheckAllPassNum(int f)
	{ return ((f >= 0) && (f <= MaxAllPassNum)); }
static inline int CheckAllPassDelay(float f) 
	{ return ((f >= 0.1) && (f <= 10.)); }

#endif // ndef _REVERB_H_
