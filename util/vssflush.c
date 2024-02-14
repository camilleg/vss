// Delete all actors.
 
#include "vssClient.h"

int main(int argc, char* argv[])
{
	if (!BeginSoundServer())
		return 1;

	actorMessage("DeleteAllActors");
	EndSoundServer();
	return 0;
}
