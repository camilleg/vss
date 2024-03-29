#pragma once

#include <math.h>
#include <stdio.h>

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"
#include "cwdio.h"

#define CritBandNum 30
#define BUFSIZE 512
#define TABSIZE 1024
#define TWOPI 8.*atan(1.)
#define NPITCH 22
#define NDYNA 128
#define NGROUP 23
#define MAXNHAR 801
#define NODAMPERKEY 90

// global data class
struct PIANODATA {
  float freqc[9];
  int hkframe[NPITCH];

  float *wavetab, *gmag;
  int sizewave, sizegmag;
  int offsetwave[NPITCH+1];
  int offsetgmag[NPITCH+1];

  float gmaxtab[NPITCH*NDYNA*NGROUP];
  float durtab[128*128];
  float rlsratetab[NODAMPERKEY*128];

  float tabsizef;
  int gmaxtabdim[4], durtabdim[3], rlsratetabdim[3], tabsize, tabsize1, nhar;
  int ngroup[NPITCH];
  int npts[NPITCH];
  float dt[NPITCH];
  float fa[NPITCH];
  float cw[NPITCH][MAXNHAR], phase[NPITCH][MAXNHAR];
  int hfrom[NPITCH][MAXNHAR], hto[NPITCH][MAXNHAR];

  float sr, sampperiod, maxfreq;

  // action noise
  float *attn, attndur;
  float *rlsn, rlsndur;

  bool fValid;

  // reading data file 
  int readcwdcomm(FILE *fp, CWDHDR *cwdHdr, COMMCK *commCk);
  int readcwddata(FILE *fp, DATACK *dataCk, CKID ckID);
  const char* getckname(CKID ckID) const;
  int readdat(const char *filename, int *dim, float *data);
  int readpcm(const char *filename, float *data, int size);

  PIANODATA(float sr);
  ~PIANODATA();
};

class pianoAlg : public VAlgorithm
{
  PIANODATA * pianod;
  int tabsize, tabsize1;
  float tabsizef;

  // pointer to global data used in generateSamples()
  float *wavetab, *gmag1, *gmag2;
  int *offsetwave, *offsetgmag;
  float *attn, *rlsn;
  float attndur; // this is actually local

  // local(notewise) data
  int noteOn, noteDone;
  float noteOffTime;
  int offsetw;
  int offsetg1, offsetg2;
  int hkframe1, hkframe2;

  float gmax1[NGROUP], gmax2[NGROUP];
  float rlstime, rlsratesamp, rlsratesec;
  int ngroup2;
  int npts1, npts2;

  /* tone parameters */
  float dur, smax;
  float sr, sampperiod, freq, pitch, inhrange;
  int key, midiKey, dyna, octave, softPedal;

  /* action noise */
  float attnamp, attnpretime;
  float rlsnamp;
  int attni, rlsni;

  /* synthesis variables */
  float tempgmag[NGROUP];
  float scalegmag1[NGROUP], scalegmag2[NGROUP], scale1[NGROUP], scale2[NGROUP];
  int whichone;
  float synmax, maxinhar;
  float frame1, frame2, ti[NGROUP], t, gmagsi1, gmagsi2;
  float fframe1, fframe2, ftime, tstep[NGROUP], fracfreq;
  int iframe1, iframe2, itime, wi, iInRls;

  // todo: demote these to locals
  int i, j, k;

  static inline float linterp(float f, float a, float b) { return f*(b-a) + a ; }

public:
  void setTstep();
  void setWhichOne();
  void setBoth();

  void setKey(int); 
  void setFreq(float); 
  void setDyna(int);
  void setSoftPedal(int); 
  void setSusPedal(int) { cerr << "pianoAlg::setSusPedal NYI\n"; }
  void setDur(float);
  void setNoteOn(bool);

  int finished() const { return noteDone; }
  void setAttnAmp(float z) { attnamp = z; }
  void setRlsnAmp(float z) { rlsnamp = z; }
  void setInhar(float z) { printf("got %f\t",z); inhrange = z*maxinhar; setTstep(); }

  void setPianoData(PIANODATA * data);

  void	generateSamples(int);
  pianoAlg();
  ~pianoAlg();
};

class pianoHand : public VHandler
{
  pianoAlg* getAlg() { return (pianoAlg*)VHandler::getAlg(); }

public:
  void	setFreq(float);
  void	setKey(float);
  void	setDyna(float);
  void	setAttnAmp(float z) { getAlg()->setAttnAmp(z); }
  void	setRlsnAmp(float);
  void	setInhar(float z) { getAlg()->setInhar(z); }

  void	setNoteOn(float z) { getAlg()->setNoteOn(z); }
  void	setSoftPedal(float z) { getAlg()->setSoftPedal(int(z)); }
  void	setSusPedal(float z) { getAlg()->setSusPedal(int(z)); }

  void setPianoData(PIANODATA* data) { getAlg()->setPianoData(data); }

  void act();

  pianoHand(pianoAlg* alg = new pianoAlg);
  ~pianoHand() {}
  int receiveMessage(const char*);
};

class pianoActor : public VGeneratorActor
{
  PIANODATA* pianod;
  float defaultFreq;
  float defaultDyna;

public:
  VHandler* newHandler() { return new pianoHand(); }
  pianoActor();
  ~pianoActor() { delete pianod; }

  void sendDefaults(VHandler*);
  int receiveMessage(const char*);
};

static inline int	CheckFreq(float f) { return f>=27.5 && f<=3520.; }
static inline int	CheckMIDI(float f) { return f>=0 && f<=127; }
