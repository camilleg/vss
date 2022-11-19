/**************************************************
 *                    vssPing.c                   *
 * The simplest possible Audition client.  Open a *
 * a connection to the server and then close it.  *
 **************************************************/
#include "vssClient.h"

int main()
{
	if (BeginSoundServer())
		EndSoundServer();
	return 0;
}
