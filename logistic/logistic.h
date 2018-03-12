#ifndef _LOGISTIC_H_
#define _LOGISTIC_H_

#include "VActor.h"
#include <cmath>

enum { NORMAL, LINBIN, LOGBIN, SEQ };

//===========================================================================
//		logisticActor
//
//	class logisticActor is a generator actor class for dumbfmAlg
//
class logisticActor : public VActor
{
 private:
  int outputMode;
  int binNum;
  float state;
  float ctrl;
  float output;
  float scale;
  float offset;
  int go;
  float zUserFloat;
  char szMG[80];

//	construction/destruction
public:
	logisticActor(void);
virtual	~logisticActor() {}

 void setMG(char *);
 void goOnce();
 void setState(float);
 void setCtrl(float);
 void setScaleandOffset(float, float);
 void setOutput(float); 
 void setOutputMode(float);
 void setBinNum(float);

virtual void act(void);
virtual int	receiveMessage(const char * Message);

};	// end of class logisticActor

//===========================================================================
//	BOUNDS CHECKING IS VITAL TO OUR SURVIVAL!!!!!!!!!!!!!!!!!!!
//
//	Find reasonable bounds and enforce them.
//
static inline int	CheckState(float f) 	{ return (f < 1. && f > 0.); }
static inline int	CheckOutputMode(float f){ return (f <= SEQ && f >= NORMAL); }
static inline int	CheckBinNum(float f){ return (f >= 1.); }

#endif // ndef _LOGISTIC_H_
