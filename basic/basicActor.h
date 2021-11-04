// BASIC interpreter.

#pragma once
#include "VActor.h"

class BasicActor: public VActor
{
	int fTerminated;
	int preprocess(char*);
	void command(const char*);

public:
	BasicActor();
	~BasicActor();
	void act();
	int receiveMessage(const char*);
	void type_in(char *);
	void type_and_get_floats(float hMG, char* cmd);
};
