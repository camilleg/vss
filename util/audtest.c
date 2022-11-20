/* One-shot tester for .aud files. */

#include "vssClient.h"
#include <string.h>

int main(int argc, char *argv[])
{
	if (argc != 2 && argc != 3) {
LUsage:
		fprintf(stderr, "\
Usage:  %s [-i] audfilename\n\
(-i means you then type in message-group names, <CR> by itself to quit)\n\n",
			argv[0]);
		return -1;
	}
	
	if (argc==3 && strcmp(argv[1], "-i")) {
		printf("Unrecognized flag \"%s\"\n", argv[1]);
		goto LUsage;
		}

	if (!BeginSoundServer()) {
		printf("Error: couldn't find a running copy of VSS.\n");
		return -1;
	}

	const int handle = AUDinit(argv[argc-1]);
	if (handle < 0) {
		printf("Syntax error in .aud file \"%s\"\n", argv[argc-1]);
		EndSoundServer();
		return -1;
	}

	if (argc==3 && !strcmp(argv[1], "-i")) {
		for (;;) {
			char command[2000];
			const char* r = fgets(command, sizeof(command)-1, stdin);
			if (!r || !*command)
				break;
			// Strip trailing newline.
			command[strlen(command) - 1] = '\0';
			AUDupdate(handle, command, 0, NULL);
		}
	}

	AUDterminate(handle);
	EndSoundServer();
	return 0;
}
