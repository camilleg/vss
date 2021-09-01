#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

class analogAlg : public VAlgorithm
{
private:
	float vco_inc, vco_k, vco_freq;
	float vcf_cutoff, vcf_envmod, vcf_envdecay, vcf_reso, vcf_rescoeff;
	float vcf_e0, vcf_e1;
	float vcf_c0; // c0=e1 on retrigger, c0*=ed every sample
	float vcf_a, vcf_b, vcf_c, vcf_d1, vcf_d2, vcf_envpos;
	float balance;

	void recalc(void);
	void vibrate(void);

public:
	void setFreq(float);
	void setFilterCutoff(float);
	void setResonance(float);
	void setEnvMod(float);
	void setEnvDecay(float);

	void generateSamples(int);

	analogAlg(void);
	~analogAlg();
};

class analogHand : public VHandler
{
private:
	float zFreq;
	float zFilterCutoff;
	float zResonance;
	float zEnvMod;
	float zEnvDecay;

	enum {
		isetFreq,
		isetFilterCutoff,
		isetResonance,
		isetEnvMod,
		isetEnvDecay };
	
protected:
	analogAlg * getAlg(void)	{ return (analogAlg *) VHandler::getAlg(); }

public:
	void SetAttribute(IParam iParam, float z);
	void setFreq(float z, float t=timeDefault)
		{ modulate(isetFreq, zFreq, z, AdjustTime(t)); }
	void setFilterCutoff(float z, float t=timeDefault)
		{ modulate(isetFilterCutoff, zFilterCutoff, z, AdjustTime(t)); }
	void setResonance(float z, float t=timeDefault)
		{ modulate(isetResonance, zResonance, z, AdjustTime(t)); }
	void setEnvMod(float z, float t=timeDefault)
		{ modulate(isetEnvMod, zEnvMod, z, AdjustTime(t)); }
	void setEnvDecay(float z, float t=timeDefault)
		{ modulate(isetEnvDecay, zEnvDecay, z, AdjustTime(t)); }

	float dampingTime(void) { return 0.03; }

	analogHand(analogAlg * alg = new analogAlg);
	virtual ~analogHand() {}

	int receiveMessage(const char * Message);

};

class analogActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new analogHand(); }

	analogActor(void);
	virtual ~analogActor() {}

	virtual void sendDefaults(VHandler *);
	virtual int	receiveMessage(const char * Message);

	void setFreq(float f);
	void setFilterCutoff(float z);
	void setResonance(float z);
	void setEnvMod(float z);
	void setEnvDecay(float z);
	void setAllFreq(float f, float t=0.);
	void setAllFilterCutoff(float z, float t=0.);
	void setAllResonance(float z, float t=0.);
	void setAllEnvMod(float z, float t=0.);
	void setAllEnvDecay(float z, float t=0.);

protected:
	float defaultFreq, defaultFilterCutoff, defaultResonance, defaultEnvMod, defaultEnvDecay;

	virtual ostream &dump(ostream &os, int tabs);
};

static inline int CheckFreq(float f) { return f >= 0.01 && f <= globs.SampleRate; }

//static inline int CheckFilterCutoff(float f) { return f >= 0. && f <= 1.; }
//static inline int CheckResonance(float f) { return f >= 0. && f <= 1.; }
//static inline int CheckEnvMod(float f) { return f >= 0. && f <= 1.; }
//static inline int CheckEnvDecay(float f) { return f >= 0. && f <= 1.; }

inline void Clamp(float& z, float zmin=0., float zmax=1.)
	{ if (z<zmin) z=zmin; else if (z>zmax) z=zmax; }

// clamping, with no error messages
static inline int CheckFilterCutoff(float& f) { Clamp(f); return 1; }
static inline int CheckResonance(float& f) { Clamp(f); return 1; }
static inline int CheckEnvMod(float& f) { Clamp(f); return 1; }
static inline int CheckEnvDecay(float& f) { Clamp(f); return 1; }
