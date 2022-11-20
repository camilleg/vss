#include "vssClient.h"

int main()
{
  if (!BeginSoundServer())
    return 1;
  EndSoundServer();
  return 0;
}
