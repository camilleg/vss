#if !defined(basicActor_inc)
#define basicActor_inc

#include "VActor.h"

// BASIC-language interpreter.

class BasicActor : public VActor
{
private:
	int fTerminated;
	int preprocess(char* cmd);
	void command(const char* cmd);

public:
	BasicActor();
	~BasicActor();

	virtual void act(void);
	virtual	int receiveMessage(const char*);

	void type_in(char *);
	void type_and_get_floats(float hMG, char* cmd);
};

#endif
