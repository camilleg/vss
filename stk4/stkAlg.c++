#include "stk.h"

stkAlg::stkAlg(void) :
  VAlgorithm(),
  controlNum(0),
  pAlgInstru(NULL)
{
}

stkAlg::~stkAlg() 
{ 
  delete pAlgInstru; 
}

void
stkAlg::generateSamples(int howMany)
{
  for (int j=0; j < howMany; j++)
    {
      //printf("\t stkAlg::generateSamples calls tick()\n");;;;
      const float temp = pAlgInstru->tick();
      Output(temp, j);
    }
}

void 
stkAlg::setInstru(Instrmnt *instru)
{
  delete pAlgInstru;
  //printf("armand;;;; stkAlg setInstru %x\n", instru);
  pAlgInstru = instru;
}

void 
stkAlg::setCtrlNum(int iValue)
{
  controlNum = iValue;
}

void 
stkAlg::setCtrlValue(float fValue)
{
  pAlgInstru->controlChange(controlNum, fValue);
}

void 
stkAlg::noteOn(float freq, float amp)
{
  //printf("armand;;;; stkAlg plucked freq=%f ampl=%f\n", freq,amp);
  pAlgInstru->noteOn(freq, amp);
}

void 
stkAlg::noteOff(float amp)
{
  pAlgInstru->noteOff(amp);
}
