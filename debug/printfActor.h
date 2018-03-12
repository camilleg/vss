#if !defined(printfActor_inc)
#define printfActor_inc

#include "VActor.h"

//===========================================================================
//		class PrintfActor
//
class PrintfActor : public VActor	
{
public:
	PrintfActor();
	~PrintfActor();

void setColor(int);
void setColor(const char*);
void setFile(const char*);
virtual	int receiveMessage(const char*);
	
private:
	// Colors are from "man xwsh"
	enum { nil = -1, black, red, green, yellow, blue, magenta, cyan, white };
	int color;
	char* munch(char*);
	void flush(int cch) { if (cch>0 && file!=stderr) fflush(file); }
		// simply ignore fprintf's return value, and flush the file after fprintf has completed.
	FILE* file;
};

#endif
