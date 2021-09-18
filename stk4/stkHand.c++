#include "stk.h"
#include "string.h"

#include "SKINI.msg"  // for all the SKINI control names __SK_*

// STK instrument classes
#include "Plucked.h"
//#include "Plucked2.h"
#include "Bowed.h"
//#include "BowedBar.h"
#include "Mandolin.h"
#include "Brass.h"
#include "Flute.h"
#include "Clarinet.h"

#include "Shakers.h"

//#include "AgogoBel.h"
//#include "Marimba.h"
//#include "Vibraphn.h"
#include "VoicForm.h"

#include "HevyMetl.h"
#include "PercFlut.h"
#include "Rhodey.h"
#include "Wurley.h"
#include "TubeBell.h"
#include "FMVoices.h"
#include "BeeThree.h"

#include "Moog.h"
//#include "DrumSynt.h"

enum {
	// Physical Models
STK_Plucked, 	// 0
STK_Bowed, 	// 1
STK_BowedBar, 	// 2
STK_Mandolin, 	// 3 need rawwaves/mand*.raw
STK_Brass, 	// 4
STK_Clarinet, 	// 5
STK_Flute, 	// 6
	// Stochastic Event Models
STK_Shakers,	// 7 
	// Modal/Formant
STK_Marimba, 	// 8
STK_Vibraphn, 	// 9
STK_AgogoBel, 	// 10
STK_VoicForm, 	// 11
	// FM
STK_HeavyMtl, 	// 12
STK_PercFlut, 	// 13
STK_Rhodey, 	// 14
STK_Wurley, 	// 15
STK_TubeBell, 	// 16
STK_BeeThree, 	// 17
STK_FMVoices, 	// 18
	// Sampling
STK_Moog1,	// 19
STK_DrumSynt,   // 20
NUM_INSTRU      // 21: number of STK instruments
};

char instruName[NUM_INSTRU][10] =  // the order has to be consistent with enum
  {"Plucked",
   "Bowed",
   "BowedBar",
   "Mandolin",
   "Brass",
   "Clarinet",
   "Flute",

   "Shakers",

   "Marimba",
   "Vibraphn",
   "Agogobel",
   "VoicForm",

   "HeavyMtl",
   "PercFlut",
   "Rhodey",
   "Wurley",
   "TubeBell",
   "BeeThree",
   "FMVoices",

   "Moog1",
   "DrumSynt"
  };

static inline int CheckInstruNum(int i) { return i >= 0 && i < NUM_INSTRU; }

//===========================================================================
//		construction
//
stkHand::stkHand(stkAlg * alg) :
  VHandler(alg),
  fValue(alg, &stkAlg::setCtrlValue),
  instruNum(0),
  pHandInstru(NULL)
{ 
  fValue.init(this);
  setTypeName("stkHand"); 
  Stk::setRawwavePath("/r/vss/4/dev/srv/stk-4.4.4/rawwaves");
  // How not hardcode this?
  // Store those few dozen files in the exe, and unpack them into /run/shm (or man 3 mkdtemp, e.g. on macos).
  // Then the exe really is standalone and can run from anywhere.
  // How can we guarantee that the tmpdir is destroyed?  man 3 atexit?
}

int
stkHand::receiveMessage(const char * Message)
{
  CommandFromMessage(Message);

  if (CommandIs("SetInstrument"))
    {
      ifS(z, setInstru(z) );
      return Uncatch();
    }
  if (CommandIs("SetInstrumentNum"))
    {
      ifD(z, setInstruNum(z) );
      return Uncatch();
    }

  if (CommandIs("SetControl"))
    {

      ifDFF(z, z1, z2, setControl(z, z1, z2) );
      ifDF(z, z1, setControl(z, z1) );
      return Uncatch();
    }
  if (CommandIs("NoteOn"))
    {
      ifFF(z, z1, setNoteOn(z, z1) );
      ifF(z, setNoteOn(z) );
      return Uncatch();
    }
  if (CommandIs("NoteOff"))
    {
      ifF(z, setNoteOff(z) );
      ifNil(setNoteOff());
    }

  //******************************************
  // Physical Modeling family  messages
  //******************************************

  if (CommandIs("SetBreathPressure"))
    {
      ifFF(z, z1, setControl(__SK_AfterTouch_Cont_, z, z1) );
      ifF(z, setControl(__SK_AfterTouch_Cont_, z) );
      return Uncatch();
    }

  if (CommandIs("SetVibFreq"))
    {
      ifFF(z, z1, setControl(__SK_ModFrequency_, z, z1) );
      ifF(z, setControl(__SK_ModFrequency_, z) );
      return Uncatch();
    }

  if (CommandIs("SetVibGain"))
    {
      ifFF(z, z1, setControl(__SK_ModWheel_, z, z1) );
      ifF(z, setControl(__SK_ModWheel_, z) );
      return Uncatch();
    }

  //*********************
  // Bowed messages
  //*********************

  if (CommandIs("SetBowPressure"))
    {
      ifFF(z, z1, setControl(__SK_BowPressure_, z, z1) );
      ifF(z, setControl(__SK_BowPressure_, z) );
      return Uncatch();
    }

  if (CommandIs("SetBowPosition"))
    {
      ifFF(z, z1, setControl(__SK_BowPosition_, z, z1) );
      ifF(z, setControl(__SK_BowPosition_, z) );
      return Uncatch();
    }

  //*********************
  // BowedBar messages
  //*********************

  if (CommandIs("SetBalance"))
    {
      ifFF(z, z1, setControl(__SK_Balance_, z, z1) );
      ifF(z, setControl(__SK_Balance_, z) );
      return Uncatch();
    }

  if (CommandIs("SetSustain"))
    {
      ifFF(z, z1, setControl(__SK_Sustain_, z, z1) );
      ifF(z, setControl(__SK_Sustain_, z) );
      return Uncatch();
    }

  if (CommandIs("SetPortamento"))
    {
      ifFF(z, z1, setControl(__SK_Portamento_, z, z1) );
      ifF(z, setControl(__SK_Portamento_, z) );
      return Uncatch();
    }

  //*********************
  // Mandolin messages
  //*********************

  if (CommandIs("SetBodySize"))
    {
      ifFF(z, z1, setControl(__SK_BodySize_, z, z1) );
      ifF(z, setControl(__SK_BodySize_, z) );
      return Uncatch();
    }

  if (CommandIs("SetPluckPosition"))
    {
      ifFF(z, z1, setControl(__SK_PickPosition_, z, z1) );
      ifF(z, setControl(__SK_PickPosition_, z) );
      return Uncatch();
    }

  if (CommandIs("SetStringDamping"))
    {
      ifFF(z, z1, setControl(__SK_StringDamping_, z, z1) );
      ifF(z, setControl(__SK_StringDamping_, z) );
      return Uncatch();
    }

  if (CommandIs("SetStringDetune"))
    {
      ifFF(z, z1, setControl(__SK_StringDetune_, z, z1) );
      ifF(z, setControl(__SK_StringDetune_, z) );
      return Uncatch();
    }

  if (CommandIs("SetPluckStrength"))
    {
      ifFF(z, z1, setControl(__SK_AfterTouch_, z, z1) );
      ifF(z, setControl(__SK_AfterTouch_, z) );
      return Uncatch();
    }

  if (CommandIs("SetMic"))
    {
      ifFF(z, z1, setControl(411, int(z)%12) );
      ifF(z, setControl(411, int(z)%12) );
      return Uncatch();
    }

  //*********************
  // Brass messages
  //*********************

  if (CommandIs("SetLipTension"))
    {
      ifFF(z, z1, setControl(__SK_LipTension_, z, z1) );
      ifF(z, setControl(__SK_LipTension_, z) );
      return Uncatch();
    }

  if (CommandIs("SetSlideLength"))
    {
      ifFF(z, z1, setControl(__SK_SlideLength_, z, z1) );
      ifF(z, setControl(__SK_SlideLength_, z) );
      return Uncatch();
    }

  //*********************
  // Clarinet messages
  //*********************

  if (CommandIs("SetReedStiffness"))
    {
      ifFF(z, z1, setControl(__SK_ReedStiffness_, z, z1) );
      ifF(z, setControl(__SK_ReedStiffness_, z) );
      return Uncatch();
    }

  if (CommandIs("SetNoiseGain"))
    {
      ifFF(z, z1, setControl(__SK_NoiseLevel_, z, z1) );
      ifF(z, setControl(__SK_NoiseLevel_, z) );
      return Uncatch();
    }

  //*********************
  // Flute messages
  //*********************

  if (CommandIs("SetJetDelay"))
    {
      ifFF(z, z1, setControl(__SK_JetDelay_, z, z1) );
      ifF(z, setControl(__SK_JetDelay_, z) );
      return Uncatch();
    }

  if (CommandIs("SetNoiseGain"))
    {
      ifFF(z, z1, setControl(__SK_NoiseLevel_, z, z1) );
      ifF(z, setControl(__SK_NoiseLevel_, z) );
      return Uncatch();
    }

  //*********************
  // Shakers messages
  //*********************

  if (CommandIs("SetShakerType"))
    {
      ifD(z, setShakerType(z) );
      return Uncatch();
    }

  if (CommandIs("SetShakerEnergy"))
    {
      ifFF(z, z1, setControl(__SK_Breath_, z, z1) );
      ifF(z, setControl(__SK_Breath_, z) );
      return Uncatch();
    }

  if (CommandIs("SetShakerDecay"))
    {
      ifFF(z, z1, setControl(__SK_FootControl_, z, z1) );
      ifF(z, setControl(__SK_FootControl_, z) );
      return Uncatch();
    }

  if (CommandIs("SetShakerNum"))
    {
      ifFF(z, z1, setControl(__SK_ModFrequency_, z, z1) );
      ifF(z, setControl(__SK_ModFrequency_, z) );
      return Uncatch();
    }

  if (CommandIs("SetShakerRes"))
    {
      ifFF(z, z1, setControl(__SK_ModWheel_, z, z1) );
      ifF(z, setControl(__SK_ModWheel_, z) );
      return Uncatch();
    }

  //*********************
  // Modal family messages
  //*********************

  if (CommandIs("SetStickHardness"))
    {
      ifFF(z, z1, setControl(__SK_StickHardness_, z, z1) );
      ifF(z, setControl(__SK_StickHardness_, z) );
      return Uncatch();
    }

  if (CommandIs("SetStrikePosition"))
    {
      ifFF(z, z1, setControl(__SK_StrikePosition_, z, z1) );
      ifF(z, setControl(__SK_StrikePosition_, z) );
      return Uncatch();
    }

  if (CommandIs("SetStrikeSpeed"))
    {
      ifFF(z, z1, setControl(__SK_AfterTouch_Cont_, z, z1) );
      ifF(z, setControl(__SK_AfterTouch_Cont_, z) );
      return Uncatch();
    }

  // SetVibFreq and SetVibGain are captured by the PhysModel family

  //*********************
  // VoicForm messages
  //*********************

  if (CommandIs("SetVoice"))
    {
      ifFF(z, z1, setControl(__SK_Breath_, z, z1) );
      ifF(z, setControl(__SK_Breath_, z) );
      return Uncatch();
    }

  if (CommandIs("SetPhoneme"))
    {
      ifFF(z, z1, setControl(__SK_FootControl_, z, z1) );
      ifF(z, setControl(__SK_FootControl_, z) );
      return Uncatch();
    }

  if (CommandIs("SetPole"))
    {
      ifFF(z, z1, setControl(__SK_AfterTouch_Cont_, z, z1) );
      ifF(z, setControl(__SK_AfterTouch_Cont_, z) );
      return Uncatch();
    }

  //*********************
  // FMVoices messages
  //*********************

  if (CommandIs("SetGain"))
    {
      ifFF(z, z1, setControl(__SK_Breath_, z, z1) );
      ifF(z, setControl(__SK_Breath_, z) );
      return Uncatch();
    }

  if (CommandIs("SetFormant"))
    {
      ifFF(z, z1, setControl(__SK_FootControl_, z, z1) );
      ifF(z, setControl(__SK_FootControl_, z) );
      return Uncatch();
    }

  if (CommandIs("SetModSpeed"))
    {
      ifFF(z, z1, setControl(__SK_ModFrequency_, z, z1) );
      ifF(z, setControl(__SK_ModFrequency_, z) );
      return Uncatch();
    }

  if (CommandIs("SetModDepth"))
    {
      ifFF(z, z1, setControl(__SK_ModWheel_, z, z1) );
      ifF(z, setControl(__SK_ModWheel_, z) );
      return Uncatch();
    }

  if (CommandIs("SetTilt"))
    {
      ifFF(z, z1, setControl(__SK_AfterTouch_Cont_, z, z1) );
      ifF(z, setControl(__SK_AfterTouch_Cont_, z) );
      return Uncatch();
    }

  //*********************
  // Moog1 messages
  //*********************

  if (CommandIs("SetQ"))
    {
      ifFF(z, z1, setControl(__SK_FilterQ_, z, z1) );
      ifF(z, setControl(__SK_FilterQ_, z) );
      return Uncatch();
    }

  if (CommandIs("SetSweepRate"))
    {
      ifFF(z, z1, setControl(__SK_FilterSweepRate_, z, z1) );
      ifF(z, setControl(__SK_FilterSweepRate_, z) );
      return Uncatch();
    }

  if (CommandIs("SetEnvelope"))
    {
      ifFF(z, z1, setControl(__SK_AfterTouch_Cont_, z, z1) );
      ifF(z, setControl(__SK_AfterTouch_Cont_, z) );
      return Uncatch();
    }

  // SetModSpeed and SetModDepth are caught by FMVoices

  return VHandler::receiveMessage(Message);
}

void 
stkHand::setInstru(char *sInstruName)
{
  int i, temp = -1;

  /*
  for (i=0; i<NUM_INSTRU; i++)
    printf("%s ",instruName[i]);
    fflush(stdout);
  */

  for (i=0; i<NUM_INSTRU; i++)
    if ( !strcmp(instruName[i], sInstruName) )
      {
	temp = i;
	break;
      }

  if ( temp == -1 )
    {
      printf("stkHand doesn't have instrument with the name %s\n",sInstruName);
      return;
    }

  setInstruNum(temp);
}

void
stkHand::setInstruNum(int iNum)
{
  if (!CheckInstruNum(iNum))
    {
      printf("setHand got bogus instrument number %d. ", iNum);
      printf("Valid range is [0,%d]\n", NUM_INSTRU);
      return;
    }

  instruNum = iNum;

  // instrument init copied from VoicMang.cpp

  switch(instruNum)
    {

    case(STK_Plucked): 
      pHandInstru = (Instrmnt *)new Plucked(20.0);
      //printf("Instrmnt* plucked handler is %p\n", pHandInstru);;
      break;
    case(STK_Bowed):
      pHandInstru = (Instrmnt *)new Bowed(50.0);
      break;   
    case(STK_Mandolin): 
      pHandInstru = (Instrmnt *)new Mandolin(50.0);
      //printf("Instrmnt* mandolin handler is %p\n", pHandInstru);;
      break;
    case(STK_Brass):
      pHandInstru = (Instrmnt *)new Brass(50.0);
      break;
    case(STK_Flute):
      pHandInstru = (Instrmnt *)new Flute(50.0);
      break; 
    case(STK_Clarinet):
      pHandInstru = (Instrmnt *)new Clarinet(50.0);
      break; 

    case(STK_Shakers):
      pHandInstru = (Instrmnt *)new Shakers;
      break;

    case(STK_VoicForm):
      pHandInstru = (Instrmnt *)new VoicForm;
      break;  

    case(STK_PercFlut):
      pHandInstru = (Instrmnt *)new PercFlut;
      break;
    case(STK_Rhodey):
      pHandInstru = (Instrmnt *)new Rhodey;
      break;
    case(STK_Wurley):
      pHandInstru = (Instrmnt *)new Wurley;
      break;
    case(STK_TubeBell):
      pHandInstru = (Instrmnt *)new TubeBell;
      break;
    case(STK_BeeThree):
      pHandInstru = (Instrmnt *)new BeeThree;
      break;
    case(STK_FMVoices):
      pHandInstru = (Instrmnt *)new FMVoices;
      break; 
      /**/
    }

  getAlg()->setInstru( pHandInstru );
}

void 
stkHand::setControl(int iCtrlNum, float fNewValue, float t)
{
  if ( pHandInstru == NULL )
    {
      printf("stkHand hasn't SetInstrument, cannot SetControl on nothing!\n");
      return;
    }

  if ( instruNum == STK_Shakers && iCtrlNum == __SK_ShakerInst_ )
    {
      setShakerType(int(fNewValue));
      return;
    }

  /*
  if ( !CheckMIDIVal(float(iCtrlNum)) )
    {
      printf("stkHand got bogus control number %d. ", iCtrlNum);
      printf("Valid range is [0 127]\n");
      return;
    }
  */

  if ( !CheckMIDIVal(fNewValue) )
    {
      printf("stkHand got bogus control value %.0f. ", fNewValue);
      printf("Valid range is [0. 127.]\n");
      return;
    }


  getAlg()->setCtrlNum(iCtrlNum);
  fValue.set(fNewValue, t);
}

void
stkHand::setNoteOn(float freq, float amp)
{
  if ( pHandInstru == NULL )
    {
      printf("stkHand hasn't SetInstrument, cannot NoteOn on nothing!\n");
      return;
    }

  if ( !CheckFreq(freq) )
    {
      printf("stkHand got bogus noteOn frequency %f. ", freq);
      printf("Valid range is [0. %.0f]\n", MAX_FREQ);
      return;
    }

  getAlg()->noteOn(freq, amp);
}

void
stkHand::setNoteOff(float amp)
{
  if ( pHandInstru == NULL )
    {
      printf("stkHand hasn't SetInstrument, cannot NoteOff on nothing!\n");
      return;
    }

  getAlg()->noteOff(amp);
}

void
stkHand::setShakerType(int iType)
{
  if ( pHandInstru == NULL )
    {
      printf("stkHand hasn't SetInstrument, cannot SetShakerType on nothing!\n");
      return;
    }

  if ( instruNum != STK_Shakers )
    {
      printf("stkHand only accepts SetShakerType for STK shakers, \n");
      printf("while this particular stkHand is a %s\n", instruName[instruNum]);
      return;
    }
  if ( !CheckShakerType(iType) )
    {
      printf("stkHand got bogus shaker type %d. ", iType);
      printf("Valid range is [0 %d]. Will use maraca\n", NUM_SHAKER);
    }

  getAlg()->setCtrlNum(__SK_ShakerInst_);
  getAlg()->setCtrlValue(iType);
}
