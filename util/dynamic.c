// An example of using AUDupdate()'s return value.

#include "vssClient.h"

int main()
{
	int i;
	float sounds[10];
	if (!BeginSoundServer())
		return -1;

	// Ignore AUDinit()'s return value, and thus also don't bother to AUDterminate().
	(void)AUDinit("dynamic.aud");

	// Create sounds.
	for (i=0; i<10; i++)
		{
		sounds[i] = AUDupdateSimpleFloats("NewX", 1, (float)(300 + 100 * i));
		usleep(30000);
		}
	usleep(250000);

	// Modify sounds.
	for (i=0; i<10; i++)
		{
		AUDupdateSimpleFloats("ChangeFreqX", 2, sounds[i], (float)(80+40*i));
		usleep(50000);
		}
	usleep(500000);

	// Delete sounds.
	for (i=0; i<10; i++)
		{
		AUDupdateSimpleFloats("DeleteX", 1, sounds[i]);
		sounds[i] = hNil; // Prevent "dangling" handles.
		}
	usleep(300000);

	AUDupdateSimple("Done", 0, NULL);
	EndSoundServer();
	return 0;
}
