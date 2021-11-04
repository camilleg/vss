#pragma once
#include "VActor.h"

class PrintfActor : public VActor	
{
public:
	PrintfActor();
	~PrintfActor();

	void setColor(int);
	void setColor(const char*);
	void setFile(const char*);
	int receiveMessage(const char*);
	
private:
	// Colors are from "man xwsh"
	enum { nil = -1, black, red, green, yellow, blue, magenta, cyan, white };
	int color;
	char* munch(char*) const;
	void flush(int cch) const { if (cch>0 && file!=stderr) fflush(file); }
		// simply ignore fprintf's return value, and flush the file after fprintf has completed.
	FILE* file;
};
