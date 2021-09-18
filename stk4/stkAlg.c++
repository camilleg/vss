#include "stk.h"

stkAlg::stkAlg() :
  VAlgorithm(),
  controlNum(0),
  pAlgInstru(nullptr)
{
  //printf("new stkAlg %p\n", this);;
}

stkAlg::~stkAlg() 
{ 
  //printf("~stkAlg %p deleting Instrmnt* pAlgInstru %p\n", this, pAlgInstru);;
  delete pAlgInstru;
}

void stkAlg::generateSamples(int howMany)
{
  for (int j=0; j < howMany; j++)
    {
      const float temp = pAlgInstru->tick();
      Output(temp, j);
    }
}

void stkAlg::setInstru(Instrmnt *instru)
{
  //printf("stkAlg::setInstru deleting Instrmnt* pAlgInstru %p\n", pAlgInstru);;
  delete pAlgInstru;
  pAlgInstru = instru;
  //printf("stkAlg::setInstru %p := Instrmnt* pAlgInstru %p\n", this, pAlgInstru);;
}

void stkAlg::setCtrlNum(int iValue)
{
  controlNum = iValue;
}

void stkAlg::setCtrlValue(float fValue)
{
  pAlgInstru->controlChange(controlNum, fValue);
}

void stkAlg::noteOn(float freq, float amp)
{
  pAlgInstru->noteOn(freq, amp);
}

void stkAlg::noteOff(float amp)
{
  pAlgInstru->noteOff(amp);
}
