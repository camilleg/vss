#include "stk-vss.h"

ACTOR_SETUP(stkActor, StkActor)

stkActor::stkActor() : VGeneratorActor() {
  setTypeName("StkActor");
}

void stkActor::sendDefaults(VHandler * p) {
  VGeneratorActor::sendDefaults(p);
}

int stkActor::receiveMessage(const char* Message) {
  CommandFromMessage(Message);
  return VGeneratorActor::receiveMessage(Message);
}
