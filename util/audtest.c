// One-shot tester for .aud files.

#include "vssClient.h"
#include <stdbool.h>
#include <string.h>

bool is_i(const char* s)
{
	return strcmp(s, "-i") == 0;
}

int main(int argc, char* argv[])
{
	if (argc < 2 || argc > 3 || (argc == 2 && is_i(argv[1]))) {
LUsage:
		fprintf(stderr, "Usage: %s [-i] foo.aud\n  (-i lets you type the names of message groups, or return to quit)\n",
			argv[0]);
		return -1;
	}
	
	if (argc==3 && !is_i(argv[1])) {
		printf("Unrecognized flag \"%s\".\n", argv[1]);
		goto LUsage;
		}

	if (!BeginSoundServer()) {
		printf("Error: VSS not running.\n");
		return -1;
	}

	const int handle = AUDinit(argv[argc-1]);
	if (handle < 0) {
		EndSoundServer(); // AUDinit complained.
		return -1;
	}

	if (argc==3 && is_i(argv[1])) {
		for (;;) {
			char command[2000];
			const char* r = fgets(command, sizeof(command)-1, stdin);
			if (!r)
				break;
			// Strip trailing newline.
			command[strlen(command) - 1] = '\0';
			if (!*command)
				break;
			AUDupdate(handle, command, 0, NULL);
		}
	}

	AUDterminate(handle);
	EndSoundServer();
	return 0;
}
