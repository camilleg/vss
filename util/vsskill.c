// Connect to vss and kill it.

#include "vssClient.h"

int main(int argc, char* argv[])
{
	if (BeginSoundServer())
		killSoundServer();
	return 0;
}
