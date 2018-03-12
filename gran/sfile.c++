#if 0
#include "sfile.h"
#include "aiff.h"

FILE* inf = NULL;
extern void err(char*) {}
extern void warn(char*) {}
extern void scan_inf ( void );


//===========================================================================
//		sfile constructor
//
sfile::sfile(char * dir, char * file) :
	sampleData( NULL ),
	userCount( 0 )
{
// printf("constructing sfile %s/%s\n", dir, file);

	strcpy(fileName, file);
	strcpy(dirName, dir);
	
//	construct the full file name with path
	char fNameWithPath[200];
	if ( fileName[0] != '/' )
		sprintf(fNameWithPath, "%s/%s", dirName, fileName);
	else
		sprintf(fNameWithPath, "%s", fileName);
		
//	try to open the new file
	if (!(inf = fopen(fNameWithPath, "rb")))
		{
		printf("Failed to open file: %s.\n", fNameWithPath );
		return;
		}

//	get some info about the file
	int sampSize;

	scan_inf();
	fileSampSize = sampSize = nh.wdsi;
	fileNumChans = nh.chan;
	fileSRate = (float)nh.rate;
	fileNumFrames = nh.fram;

#if 0
	cerr <<"file: "
		 <<fileSampSize <<" samples, "
		 <<fileNumChans <<" channels, "
		 <<fileSRate <<" SR, "
		 <<fileNumFrames <<" frames.\n";
#endif

//	allocate some space and read up the samples
	int	sampSizeBytes = sampSize / 8;
	sampleData = new char[fileNumFrames * sampSizeBytes * fileNumChans];
	if (sampleData == NULL)
		{
		cerr <<"vss error: sampleData allocation failure.\n";
		fclose(inf);
		return;
		}
	
	if (fread(sampleData, sampSizeBytes * fileNumChans, fileNumFrames, inf)
		!= fileNumFrames )
		{
		printf("vss error: SampleActor failed to read samples from file \"%s\".\n", fNameWithPath );
		delete [] (char*)sampleData;
		sampleData = NULL;
		fclose(inf);
		return;
		}

	if (htons(0x1234) != 0x1234)
		{
		// swap bytes because aiff doesn't match endianness of cpu (e.g. aiff from irix, running on intel linux)
		char* pb = (char*)sampleData;
		for (unsigned int i=0; i<sampSizeBytes * fileNumChans * fileNumFrames; i+=2)
			{
			char tmp = pb[i+1];
			pb[i+1] = pb[i];
			pb[i] = tmp;
			}
		}

// clean up
	fclose(inf);
}


//===========================================================================
//		sfile destructor
//
sfile::~sfile()
{
// printf("destructing sfile %s/%s\n", dirName, fileName);
	if (sampleData != NULL)
		delete [] (char*)sampleData;
}
#endif
