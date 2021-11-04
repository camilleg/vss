#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//	Non-interpolating delay line, copied from stk/DLineN.c++
class DLineNcopy
{
	long inPoint;
	long outPoint;
	long length;
	long lengthm1;
	float* inputs;
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

class reverbAlg : public VAlgorithm
{
	DLineNcopy* delayLineEarlyRef[MaxEarlyRefNum];
	DLineNcopy* delayLineComb[MaxCombNum];
	DLineNcopy* delayLineAllPass[MaxAllPassNum];
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
	void clear();
	int getEarlyRefNum() const { return earlyRefNum; }
	int getCombNum() const { return combNum; }
	int getAllPassNum() const { return allPassNum; }
	const float* getEarlyRefDelay() const { return earlyRefDelay; }
	const float* getEarlyRefCoeff() const { return earlyRefCoeff; } 
	const float* getCombDelay() const { return combDelay; }
	const float* getAllPassDelay() const { return allPassDelay; }

	void setPreset(float);
	void setPara(float*);
	void setRevMix(float mix) { revMix = mix; }
	void setRevGain(float g) { revGain = g; }
	void setPole(float p) { pole = p; }
	void setBW(float bw) { BW = bw; }
	void setEarlyRefNum(float n) { earlyRefNum = n; }
	void setEarlyRefDelay(float*);
	void setEarlyRefCoeff(float*);
	void setCombNum(float n) { combNum = n; }
	void setComb();
	void setCombDelay(float*);
	void setT60(float t) { t60 = t*MaxT60; setComb(); }
	void setDampRatio(float dr) { dampRatio = dr; setComb(); }
	void setAllPassNum(float n) { allPassNum = n; }
	void setAllPassDelay(float * delay);
	void setIdle(int i) { idle = i; }

	void generateSamples(int);
	float tick(float input);
	int FValidForOutput() { return source != nullptr; }

	reverbAlg();
	~reverbAlg();
};

class reverbHand : public VHandler
{
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

protected:
	reverbAlg* getAlg() { return (reverbAlg*)VHandler::getAlg(); }

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
	
	void setIdle(float);
	void setPreset(char*);
	void setPresetFile(char*);
	void setPresetNum(int);
	void setEarlyRefNum(int);
	void setEarlyRefMix(float);
	void setEarlyRefDelay(int, float*);
	void setEarlyRefCoeff(int, float*);
	void setCombNum(int);
	void setCombDelay(int, float*);
	void setAllPassNum(int);
	void setAllPassDelay(int, float*);

	void actCleanup();
	float dampingTime() { return 0.03; }

	reverbHand(reverbAlg* alg = new reverbAlg);
	~reverbHand() {}
	int receiveMessage(const char*);
};

class reverbActor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new reverbHand(); }
	reverbActor();
	~reverbActor() {}
	void sendDefaults(VHandler*);
	int receiveMessage(const char*);

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

protected:
	float defaultRevTime;
	float defaultRevMix;
	float defaultRevBright;
	float defaultRevGain;
	float defaultRevPole;
	float defaultDampRatio;
};

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
