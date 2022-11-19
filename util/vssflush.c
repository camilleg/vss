/***************************************
 *              vssFlush.c             *
 * Delete all actors in server.        *
 ***************************************/
 
#include "vssClient.h"

int main()
{
	if (!BeginSoundServer())
		return 1;

	actorMessage("DeleteAllActors");
	EndSoundServer();
	return 0;
}
