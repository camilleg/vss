#ifndef _switch_h_
#define _switch_h_

#include "VActor.h"
#include "../thresh/threshActor.h"

class SwitchActor : public ThresholdActor
{
public:
	// Constructor, destructor
	SwitchActor(void);
	virtual ~SwitchActor() {}

	// Actor behavior
	virtual int receiveMessage(const char * Message);

};

#endif
