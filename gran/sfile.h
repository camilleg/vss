#if 0
#ifndef _SFILE_H_
#define _SFILE_H_

#include "vssSrv.h"
#include <cstring>

//===========================================================================
//		Sfile 
//
//	class Sfile holds data for an AIFF samples file. 
//
class sfile
{
private:
	void *	sampleData;		//	interleaved samples
	char	fileName[180];
	char	dirName[180];
	float	fileSRate;		//	sample rate of the file
	ulong	fileNumFrames;	//	total sample frame count for the file
	int		fileNumChans;	//	number of audio channels in the file
	int		fileSampSize;	//	size in bits of the samples, 8 or 16
	 
public:
	char * 	name(void)			{ return fileName; }
	char *	directory(void)		{ return dirName; }
	ulong	numFrames(void)		{ return fileNumFrames; }
	int		numChannels(void)	{ return fileNumChans; }
	float	sampleRate(void)	{ return fileSRate; }
	int		sampleSize(void)	{ return fileSampSize; }
	void *	samples(void)		{ return sampleData; }
	
//	keep track of how many algorithms are using this sfile.
private:
	int		userCount;
public:
	void	addUser(void *)		{ userCount++; }
	void	removeUser(void *)	{ userCount--; }
	int		numUsers(void)		{ return userCount; }

//	construction
		sfile(char *, char *);
	 	~sfile();
	 	
//	comparing
	int	equDirFile(const char * dirName, const char * fName)
			{ return !strcmp(name(), fName) && !strcmp(directory(), dirName); }
	 	
private:
		sfile(void);	//	not allowed
	
};	// end of class Sfile

extern FILE* inf;

#endif	// ndef _SFILE_H_
#endif
