#pragma once
#include "VActor.h"

class logisticActor : public VActor
{
	enum { NORMAL, LINBIN, LOGBIN, SEQ };
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
	logisticActor();
	~logisticActor() {}

	void setMG(char*);
	void goOnce();
	void setState(float);
	void setCtrl(float);
	void setScaleandOffset(float, float);
	void setOutput(float); 
	void setOutputMode(float);
	void setBinNum(float);

	void act();
	int receiveMessage(const char*);

	int CheckState(float f) { return f > 0.0 && f < 1.0; }
	int CheckOutputMode(float f) { return f >= NORMAL && f <= SEQ; }
	int CheckBinNum(float f) { return f >= 1.0; }
};
