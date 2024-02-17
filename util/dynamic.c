// An example using AUDupdate's return value, via AUDupdateWaitForReply.

#include "vssClient.h"

int main(int argc, char* argv[])
{
	int i;
	float sounds[10];
	if (!BeginSoundServer())
		return -1;

	// Ignore the return value, and thus also don't bother to AUDterminate().
	(void)AUDinit("dynamic.aud");

	// Create 10 sounds.
	AUDupdateWaitForReply(1);
	for (i=0; i<10; i++) {
		sounds[i] = AUDupdateSimpleFloats("NewX", 1, (float)(300 + 100 * i));
		usleep(40000);
	}
	AUDupdateWaitForReply(0);
	usleep(250000);

	// Modify them.
	for (i=0; i<10; i++) {
		AUDupdateSimpleFloats("ChangeFreqX", 2, sounds[i], (float)(80+40*i));
		usleep(60000);
	}
	usleep(500000);

	// Delete them.
	for (i=0; i<10; i++) {
		AUDupdateSimpleFloats("DeleteX", 1, sounds[i]);
	}
	usleep(200000);

	AUDupdateSimple("Done", 0, NULL);
	usleep(200000);
	EndSoundServer();
	return 0;
}
