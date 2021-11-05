#include "sfile.h"
#include "aiff.h"

FILE* inf = NULL;
extern void err(const char*) {}
extern void warn(const char*) {}
extern int scan_inf ( void );

sfile::sfile(char * dir, char * file) :
	sampleData( NULL ),
	userCount( 0 )
{
	strcpy(fileName, file);
	strcpy(dirName, dir);
	
	char fNameWithPath[1000];
	if ( fileName[0] != '/' )
		sprintf(fNameWithPath, "%s/%s", dirName, fileName);
	else
		sprintf(fNameWithPath, "%s", fileName);
	if (!(inf = fopen(fNameWithPath, "rb")))
		{
		printf("vss error: sfile failed to read file \"%s\".\n", fNameWithPath);
		return;
		}

	fWAV = scan_inf();
	int sampSize = fileSampSize = nh.wdsi;
	fileNumChans = nh.chan;
	fileSRate = nh.rate;
	fileNumFrames = nh.fram;

#if 1
	std::cerr <<"file: "
		 << (fWAV? "WAV, " : "AIFF, ")
		 <<fileSampSize <<" bits per sample, "
		 <<fileNumChans <<" channels, "
		 <<fileSRate <<" SR, "
		 <<fileNumFrames <<" frames.\n";
#endif

//	allocate some space and read up the samples
	int	sampSizeBytes = sampSize / 8;
	sampleData = new char[fileNumFrames * sampSizeBytes * fileNumChans];
	if (!sampleData)
		{
		std::cerr <<"vss error: sampleData allocation failure.\n";
		fclose(inf);
		inf = NULL;
		return;
		}
	
	// fread() may return 0 on some older aiff files.  Edit and resave the file with soundeditor and it'll be fine.
	unsigned int readNumFrames =
		fread(sampleData, sampSizeBytes * fileNumChans, fileNumFrames, inf);
	fclose(inf);
	inf = NULL;
	if (readNumFrames != fileNumFrames )
		{
		printf("vss error: SampleActor failed to read samples from file \"%s\".\n", fNameWithPath );
		delete [] (char*)sampleData;
		sampleData = NULL;
		return;
		}

	// endianness is irrelevant for 8-bit data
	if (fileSampSize > 8 && ((htons(0x1234) != 0x1234) ^ fWAV))
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
}

sfile::~sfile()
{
	if (sampleData)
		delete [] (char*)sampleData;
}
