#include "vssClient.h"
#include <iostream>
#include <iomanip>

const char* GetVssLibVersion(void);
const char* GetVssLibDate(void);

main()
{
	int handle; char in = NULL; int i,rev = 0;

	if (!BeginSoundServer())
	{
		cout << "UDP failed." << endl; exit(2);
	}
	else cout << "Sound Server connected." << endl;
/*	
	cout << setw(20) << setiosflags(ios::left) << "Lib version: " << GetVssLibVersion() << endl;
	cout << setw(20) << setiosflags(ios::left) << "Lib date: " << GetVssLibDate() << endl;
*/
	handle = AUDinit("foo.aud");
	if (handle < 0) {cout << "Failed to load foo.aud" << endl; exit(3);}
	else cout << "foo.aud opened." << endl;

/*	while (in != 'q')
	{	scanf("%c",&in);
		rev = !rev;
		AUDupdate(handle, "msg", 1, (float *)&rev);
	}
*/
	scanf("%c",&in);

	AUDterminate(handle);
	EndSoundServer();
}
