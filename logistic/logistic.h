#pragma once

#include "VActor.h"

enum { NORMAL, LINBIN, LOGBIN, SEQ };

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
};

static inline int	CheckState(float f) 	{ return (f < 1. && f > 0.); }
static inline int	CheckOutputMode(float f){ return (f <= SEQ && f >= NORMAL); }
static inline int	CheckBinNum(float f){ return (f >= 1.); }
