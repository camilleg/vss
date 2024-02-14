// Is VSS running?

#include "vssClient.h"

int main(int argc, char* argv[])
{
  if (!BeginSoundServer())
    return 1;
  EndSoundServer();
  return 0;
}
