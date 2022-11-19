/*
 *  dynamic.c -- example of using return value of AUDupdate()
 *
 *  Camille Goudeseune, 5/99
 */

#include "vssClient.h"

int main()
{
	int i;
	float sounds[10];
	if (!BeginSoundServer())
		return -1;
	AUDinit("dynamic.aud");

	/*** STEP 3: Create several sounds via AUDupdate() ***/

	for (i=0; i<10; i++)
		{
		sounds[i] = AUDupdateSimpleFloats("NewX", 1, (float)(300 + 100 * i));
		usleep(30000);
		}
	usleep(250000);

	/*** STEP 4: Modify sounds via AUDupdate() ***/

	for (i=0; i<10; i++)
		{
		AUDupdateSimpleFloats("ChangeFreqX", 2, sounds[i], (float)(80+40*i));
		usleep(50000);
		}
	usleep(500000);

	/*** STEP 4: Delete sounds via AUDupdate() ***/

	for (i=0; i<10; i++)
		{
		AUDupdateSimpleFloats("DeleteX", 1, sounds[i]);
		sounds[i] = hNil; /* prevent "dangling" handles */
		}
	usleep(300000);

	AUDupdateSimple("Done", 0, NULL);
	EndSoundServer();
	return 0;
}
