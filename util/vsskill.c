// Connect to vss and kill it.

#include "vssClient.h"

int main()
{
	if (BeginSoundServer())
		killSoundServer();
	return 0;
}
