#include "piano.h"

//===========================================================================
//		construction
//
pianoHand::pianoHand( pianoAlg * alg ):
  VHandler( alg )
{ 
  setTypeName("pianoHand");
  printf("pianoHand construction done\n"); fflush(stdout);
}

//===========================================================================
//		pianoHand act
//
//	suicide when note is done
//
void
pianoHand::act(void)
{
  VHandler::act();	
  if ( getAlg()->finished() )
    delete this;
}

//===========================================================================
//		receiveMessage
//
int	
pianoHand::receiveMessage(const char * Message)
{
  CommandFromMessage(Message);
  
  if (CommandIs("SetFrequency"))
    {
      ifF(z, setFreq(z) );
      return Uncatch();
    }

  if (CommandIs("SetFreq"))
    {
      ifF(z, setFreq(z) );
      return Uncatch();
    }

  if (CommandIs("SetDynamic"))
    {
      ifF(z, setDyna(z) );
      return Uncatch();
    }

  if (CommandIs("SetDyna"))
    {
      ifF(z, setDyna(z) );
      return Uncatch();
    }

  if (CommandIs("SetInharmonicity"))
    {
      ifF(z, setInhar(z) );
      return Uncatch();
    }

  if (CommandIs("SetInhar"))
    {
      ifF(z, setInhar(z) );
      return Uncatch();
    }

  if (CommandIs("SetAttnAmp"))
    {
      ifF(z, setAttnAmp(z) );
      return Uncatch();
    }

  if (CommandIs("SetNoteOn"))
    {
      ifF(z, setNoteOn(z) );
      return Uncatch();
    }

  return VHandler::receiveMessage(Message);
}

//===========================================================================
//		setFreq
//
void	
pianoHand::setFreq(float z)
{
  if (!CheckFreq(z))
    {
      printf("reverbHand got bogus frequency %f.\n",z);
      return;
    }

  getAlg()->setFreq(z);
}

//===========================================================================
//		setDyna
//
void	
pianoHand::setDyna(float z)
{
  if (!CheckMIDI(z))
    {
      printf("reverbHand got bogus dynamic %f.\n",z);
      return;
    }

  getAlg()->setDyna(int(z));
}

