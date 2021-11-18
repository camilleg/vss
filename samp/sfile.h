#pragma once
#include "vssSrv.h"
#include <cstring>

// Data for an AIFF audio file.
class sfile
{
	void*	sampleData;		//	interleaved samples
	char	fileName[180];
	char	dirName[180];
	float	fileSRate;		//	sample rate
	ulong	fileNumFrames;
	int		fileNumChans;
	int		fileSampSize;	//	sample size in bits, 8 or 16
	int		fWAV;			//	true iff it's a (little endian) .WAV file
	int		userCount;		// how many algorithms are using this
	 
public:
	char* 	name() { return fileName; }
	char*	directory() { return dirName; }
	ulong	numFrames() const { return fileNumFrames; }
	int		numChannels() const { return fileNumChans; }
	float	sampleRate() const { return fileSRate; }
	int		sampleSize() const { return fileSampSize; }
	void*	samples() const { return sampleData; }
	
	void	addUser(void*) { ++userCount; }
	void	removeUser(void*) { --userCount; }
	bool	unused() const { return userCount == 0; }

	sfile(char*, char*);
	~sfile();

	int	equDirFile(const char* dName, const char* fName) const
		{ return !strcmp(dirName, dName) && !strcmp(fileName, fName); }
};

extern FILE* inf;
