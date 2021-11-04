#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"
#include "VFloatParam.h"

// STK base classes
#include "Stk.h"
using namespace stk;
#include "Instrmnt.h"

#define MAX_FREQ (globs.SampleRate * 0.45)
#define NUM_SHAKER 12 // number of types of Shakers. May increase in newer STK

static inline int CheckFreq(float f) 	{ return f > 0. && f < MAX_FREQ; }
static inline int CheckMIDIVal(float f)	{ return f >= 0. && f <= 127.; }
static inline int CheckShakerType(int i){ return i >= 0 && i <= NUM_SHAKER; }
// int CheckInstruNum(int i) is in stkHand.c++

class stkAlg : public VAlgorithm
{
  int controlNum;
  Instrmnt* pAlgInstru;

 public:
  void generateSamples(int);
  void setInstru(Instrmnt *instru);
  void setCtrlNum(int iValue);
  void setCtrlValue(float fValue);
  void noteOn(float freq, float amp);
  void noteOff(float amp);

  int FValidForOutput() { return pAlgInstru != nullptr; }

  stkAlg();
  ~stkAlg();
};

class stkHand : public VHandler
{
  FloatParam<stkAlg> fValue;
  int instruNum; // indicate what instrument it is 

 protected:
  stkAlg* getAlg() { return (stkAlg*)VHandler::getAlg(); }

 public:
  Instrmnt* pHandInstru;

  int receiveMessage(const char*);

  void setInstru(char * sInstruName);
  void setInstruNum(int iNum);
  void setControl(int iCtrl, float fNewValue, float t=0.);
  void setNoteOn(float freq, float amp=1.);
  void setNoteOff(float amp=0.);
  void setShakerType(int iType);

  stkHand(stkAlg* alg = new stkAlg);
  ~stkHand() = default;
};

class stkActor : public VGeneratorActor
{
 public:
  VHandler* newHandler() { return new stkHand(); }
  void sendDefaults(VHandler*);
  int receiveMessage(const char*);
  stkActor();
  ~stkActor() = default;
};
