#if !defined(crowdActor_inc)
#define crowdActor_inc

#include "VActor.h"

// id's should ideally be 32-bit values, but they're passed in
// as part of an array of floats.  So leave them as floats in here.  Sigh.

class CrowdHandler
{
public:
	float h; // handle to SampleHandler actor
	float id;

	void Init(float hArg, float idArg) { h=hArg; id=idArg; }
};

#define iSndMax 50
#define iXYZMax 500
#define cchFileMax 100

class XYZ
{
public:
	float id, x,y,z;
	void Init(float idArg, float xArg, float yArg, float zArg)
		{ id=idArg; x=xArg; y=yArg; z=zArg; }
};

class CrowdActor : public VActor	
{
private:
	VActor* mgDelete;
	VActor* sampActor;
	float hSampActor;
	float hMGDelete;

	int ih;                    // # of handles currently in use (length of rgh)
	int ihMax;                 // max length of rgh
	float dB;                  // amplitude change in decibels
	float zRateMin, zRateMax;  // change of "pitch" (log thereof)
	CrowdHandler rgh[iSndMax]; // list of currently actually playing sounds
	char szFile[cchFileMax];   // soundfile name
	float idMomentary;         // for internally generated id's

	float rgidTopN[iSndMax];     // the Top N id's (these have handlers)
	XYZ rgxyz[iXYZMax];          // id+position's passed in from XYZArray
	int cxyz;                    // length of rgxyz

	float rgidTopNPrev[iSndMax]; // previous CAVE-frame
	XYZ rgxyzPrev[iXYZMax];
	int cxyzPrev;
	int ihPrev;

	int FTopN(float id);
	int FTopNPrev(float id);
	float& PhFromId(float id);
	XYZ& XYZFromId(float id);

public:
	CrowdActor();
	~CrowdActor();

	/*;;;; virtual void act(void);*/
	virtual	int receiveMessage(const char*);

protected:
	void setDirectory(char* sz);
	void setFile(char* sz);
	void setRate(float rMin, float rMax);
	void setMaxSimultaneous(int c);
	void play(float x=0., float y=5., float z=0.);
	void xyzArray(float* rgz, int cz);
	void autostop(float h);
};

#endif
