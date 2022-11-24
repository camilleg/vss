#include "logistic.h"

ACTOR_SETUP(logisticActor, LogisticActor)
  
logisticActor::logisticActor(void) : 
  VActor(),
  outputMode(0),
  binNum(10),
  state(0.6),
  ctrl(3),
  scale(1),
  offset(0),
  go(0),
  zUserFloat(0.)
{
  setTypeName("LogisticActor");
}

void
logisticActor::act()
{
  VActor::act();
  if (!*szMG) return;
  if (!go) return;
  go = 0;
  state = ctrl * state * (1.-state);
  // This switch() only sets output, but that's immediately overwritten!
  // This is so in every version I have, back to 1999 Aug 3.
  switch (outputMode) {
  case LINBIN:
      output = floor(state*float(binNum));
      if (output >= binNum)
	output -= 1;
      break;
  case LOGBIN:
      output = (pow(10.f, state)-1.f)/9.f;
      output = floor(output*float(binNum));
      if (output >= binNum)
	output -= 1;
      break;
  case SEQ:
  default:
      break;
  }

  output = scale * state + offset;
  if (isDebug()) 
    printf("state %f, ctrl %f, output %f, scale %f, offset %f, mg %s, userfloat %f\n",
	   state, ctrl, output, scale, offset, szMG, zUserFloat);

  char szCmd[256];
  sprintf(szCmd, "SendData %s [%f %f]", szMG, output, zUserFloat);
  actorMessageHandler(szCmd);
}

int
logisticActor::receiveMessage(const char * Message)
{
  CommandFromMessage(Message);
  
  if (CommandIs("SetMessageGroup"))
    {
      ifS( szName, setMG(szName) ); // strncpy might truncate char szName[1000] to char szMG[80].
      return Uncatch();
    }
  
  if (CommandIs("GoOnce"))
    {
      goOnce();
      return Catch();
    }
  
  if (CommandIs("SetState"))
    {
      ifF(z, setState(z) );
      return Uncatch();
    }
  
  if (CommandIs("SetCtrl"))
    {
      ifF(z, setCtrl(z) );
      return Uncatch();
    }
  
  if (CommandIs("SetScaleAndOffset"))
    {
      ifFF(z1,z2, setScaleandOffset(z1, z2) );
      return Uncatch();
    }
  
  if (CommandIs("SetOutput"))
    {
      ifF(z, setOutput(z) );
      return Uncatch();
    }

  if (CommandIs("SetOutputMode"))
    {
      ifF(z, setOutputMode(z) );
      return Uncatch();
    }

  if (CommandIs("SetNumberOfBins"))
    {
      ifF(z, setBinNum(z) );
      return Uncatch();
    }

  if (CommandIs("SetUserFloat"))
    {
      ifF( z, zUserFloat=z );
      return Uncatch();
    }

  return VActor::receiveMessage(Message);
}

void logisticActor::setMG(char *sz)
{
  if (isDebug()) printf("got mg %s\n",sz);
  strncpy(szMG, sz, sizeof(szMG)-1);
}

void logisticActor::goOnce()
{
  go = 1;
}

void logisticActor::setState(float z)
{
  if (!CheckState(z))
    {
      printf("logisticActor got bogus state %f. Valid range is [0,1].\n",z);
      return;
    }
  state = z;
}

void logisticActor::setCtrl(float z)
{
  ctrl = z;
}

void logisticActor::setScaleandOffset(float z1, float z2)
{
  scale = z1;
  offset = z2;
}

void logisticActor::setOutput(float z)
{
  output = z;
  go = 1;
}

void logisticActor::setOutputMode(float z)
{
  if (!CheckOutputMode(z))
    {
      printf("logisticActor got bogus output mode %.0f. Valid range is [%d,%d].\n",z,NORMAL,SEQ);
      return;
    }
  outputMode = int(z);
  if (outputMode == LINBIN || outputMode == LOGBIN)
	{
	  scale = 1.;
	  offset = 0.;
	}
}

void logisticActor::setBinNum(float z)
{
  if (!CheckBinNum(z))
    {
      printf("logisticActor got bogus number of bins %.0f. Valid range is [1,Inf).\n",z);
      return;
    }
  binNum = int(z);
}
