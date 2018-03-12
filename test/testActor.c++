//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
//===========================================================================

#include <cstdio>
#include <iostream>

#include "testActor.h"

ACTOR_SETUP(TestActor, TestActor)

//===========================================================================
//		construction
//
TestActor::TestActor()
{
  setTypeName("TestActor");
}

TestActor::~TestActor()
{
}

void print(char* str)
{
  printf("%s\n", str);
  fflush(stdout);
}

/* This receiveMessage should not forward everything up to the Threshold superclass,
   because the superclass may change.
 */
int TestActor::receiveMessage(const char* Message)
{
  CommandFromMessage(Message);
  if (CommandIs("Print"))
    {
      ifS(str, print(str));
      return Uncatch();
    }
  if (CommandIs("PrintL"))
    {
      ifS(str, print(str));
      return Uncatch();
    }

  return baseClass::receiveMessage(Message);
}

