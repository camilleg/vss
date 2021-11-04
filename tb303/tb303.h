#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

class tb303Alg : public VAlgorithm
{
	float vco_inc, vco_k;
	float vcf_cutoff, vcf_envmod, vcf_envdecay, vcf_reso, vcf_rescoeff;
	float vcf_e0, vcf_e1;
	float vcf_c0; // c0=e1 on retrigger, c0*=ed every sample
	float vcf_d1, vcf_d2, vcf_envpos;
	int fStarted;
	void recalc();

public:
	void Retrigger()
		{ vcf_c0=vcf_e1; vco_k=-.5; fStarted=1; }
	void setFreq(float);
	void setFilterCutoff(float);
	void setResonance(float);
	void setEnvMod(float);
	void setEnvDecay(float);

	void generateSamples(int);
	tb303Alg();
	~tb303Alg();
};

class tb303Hand : public VHandler
{
	float zFreq;
	float zFilterCutoff;
	float zResonance;
	float zEnvMod;
	float zEnvDecay;
	enum { isetFreq, isetFilterCutoff, isetResonance, isetEnvMod, isetEnvDecay };
	
protected:
	tb303Alg* getAlg() { return (tb303Alg*)VHandler::getAlg(); }
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

	void Retrigger() { getAlg()->Retrigger(); }

	float dampingTime() { return 0.03; }

	tb303Hand(tb303Alg* alg = new tb303Alg);
	~tb303Hand() {}
	int receiveMessage(const char*);
};

class tb303Actor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new tb303Hand(); }
	tb303Actor();
	~tb303Actor() {}
	void sendDefaults(VHandler*);
	int	receiveMessage(const char*);

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
	ostream& dump(ostream& os, int);
};

static inline int CheckFreq(float f) { return f >= 0.01 && f <= globs.SampleRate; }

// clamping, with no error messages
inline void Clamp(float& z, float zmin=0., float zmax=1.)
	{ if (z<zmin) z=zmin; else if (z>zmax) z=zmax; }
static inline int CheckFilterCutoff(float& f) { Clamp(f); return 1; }
static inline int CheckResonance(float& f) { Clamp(f); return 1; }
static inline int CheckEnvMod(float& f) { Clamp(f); return 1; }
static inline int CheckEnvDecay(float& f) { Clamp(f); return 1; }
