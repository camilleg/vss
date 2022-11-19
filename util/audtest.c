/*
 *  audTest.c -- one-shot tester for .aud files
 *
 *  Original author unknown
 *  Revamped by Camille Goudeseune, 5/20/98
 */

#include "vssClient.h"
#include <string.h>

int main(int argc, char *argv[])
{
	char	command[80];
	int		handle;
	
	if (argc != 2 && argc != 3)	{
LUsage:
		fprintf(stderr, "\
Usage:  %s [-i] audfilename\n\
(-i means you then type in message-group names, <CR> by itself to quit)\n\n",
			argv[0]);
		return -1;
	}
	
	if (argc==3 && strcmp(argv[1], "-i"))
		{
		printf("Unrecognized flag \"%s\"\n", argv[1]);
		goto LUsage;
		}

/*** Open communications to server ***/

	if (!BeginSoundServer())	{
		printf("Error: couldn't find a running copy of VSS.\n");
		return -1;
	}
	
/*** Set up the interface actor(s) ***/ 

	handle = AUDinit(argv[argc-1]);
	if (handle < 0)
		{
		printf("Syntax error in .aud file \"%s\"\n", argv[argc-1]);
		EndSoundServer();
		return -1;
		}

	if (argc==3 && !strcmp(argv[1], "-i"))
		for (;;)
			{
			fgets(command, sizeof(command)-1, stdin);
			if (!*command)
				break;
			/* erase final newline character */
			command[strlen(command) - 1] = '\0';
			AUDupdate(handle, command, 0, NULL);
			}
	
/*** Clean up ***/	

	AUDterminate(handle);

/*** Break communication ***/	

	EndSoundServer();
	return 0;
}
