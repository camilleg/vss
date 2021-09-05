//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
//===========================================================================

#include <cstdio>
#include <iostream>

#include "cxArray.h"
//#define cxDEBUG			// enables debug macros, otherwise macros will be blank.
#define cxCOUT cerr
#include "cxDebug.h"
#define D_SA  1			// debug SwitchActor stuff

#include "switchActor.h"
//#include "saThresholdActor.h"

#include "cxMem.h"

/*------------------------------------------------------------
  SwitchInputGate
 */

ostream& operator<< (ostream& os, const SwitchInputGate& sg)
{
  os << "testVal: " << sg.testVal;
  return os;
}

// these 2 functions avoid a link error, but Camille doesn't pretend to understand Vijay's code here.
ostream& operator<< (ostream& os, SwitchInputGateL& sg)
{
  os << "NYI.\n";
  return os;
}
ostream& operator<< (ostream& os, FloatL& sg)
{
  os << "NYI.\n";
  return os;
}

//===========================================================================
//		construction
//
SwitchActor::SwitchActor()
  : gates(0), inputs(0), combiner(AND_Combiner)
{
  setTypeName("SwitchActor");
  // By default act, like a 1-input AND gate.
  setGateRange(0, 1, SwitchInputGate::TestGT, 0);
  setCombiner(AND_Combiner);
}

#if 0
SwitchActor::~SwitchActor()
{
}
#endif

/* This receiveMessage should not forward everything up to the Threshold superclass,
   because the superclass may change.
 */
int SwitchActor::receiveMessage(const char* Message)
{
  CommandFromMessage(Message);
  //--------------- Switch invocation type stuff ---------------
  if (CommandIs("Eval"))
    {
      ifNil( evalSwitch(computeTestVal()) );
    }
  if (CommandIs("SendAndEval"))
    {
      // SendAndEval [ floats ... ]
      ifFloatArray( data, count, sendAndEval( data, count) );
      return Uncatch();
    }
  if (CommandIs("SetInputs"))
    {
      // Like SetGates, this message takes an inclusive range of indices.
      ifDDF(start,end,inputVal, setInputRange(start, end+1, inputVal));
      ifF(inputVal, setInputs(inputVal));
      return Uncatch();
    }
  if (CommandIs("SetInput"))
    {
      // Like SetInputs but set a single input gate.
      ifDF(idx,inputVal, setInputRange(idx, idx+1, inputVal));
      return Uncatch();
    }
  if (CommandIs("SendData"))
    {
      // SendData [ floats ... ]  sets a bunch of inputs using the float list.
      ifFloatArray( data, count, copyToInputs( data, count, 0) );
      return Uncatch();
    }
  if (CommandIs("SetTestVals"))
    {
      // Like SetGates, this message takes an inclusive range of indices.
      ifDDF(start,end,testVal, setTestValRange(start, end+1, testVal));
      ifF(testVal, setTestValRange(0, gates.count(),testVal));
      return Uncatch();
    }
  if (CommandIs("SetTestVal"))
    {
      // Like SetTestVals but set a single input gate.
      ifDF(idx,testVal, setTestValRange(idx, idx+1, testVal));
      return Uncatch();
    }
  if (CommandIs("SetGates"))
    {
      // NOTE: this *message* takes an inclusive argument range, UNLIKE setGateRange.
      ifDDSF(start, end, testerName, testVal, setGateRange(start, end+1,
							   getSwitchGateTester(testerName), testVal));
      /* Update all current gates, with the new tester and testVal. */
      ifSF(testerName, testVal, setGates(getSwitchGateTester(testerName), testVal));
      return Uncatch();
    }

  if (CommandIs("PrintAll"))
    {
      ifNil(printAll());
    }

  //--------------- Extra protocol ---------------

  // SendData [ floats ] (start_idx)
  
  // SetTestValList [ floats ] (start_idx)
  
  // Add/Subtract [ floats ] (start_idx)
  if (CommandIs("Add"))
    {
      ifFloatArrayFloat(data, count, start, addToInputs(data, count, int(start)) );
      ifFloatArray(data, count, addToInputs(data, count) );
      return Uncatch();
    }
  if (CommandIs("Subtract"))
    {
      ifFloatArrayFloat(data, count, start, subtractFromInputs(data, count, int(start)) );
      ifFloatArray(data, count, subtractFromInputs(data, count) );
      return Uncatch();
    }
  // Init num_gates COMBINER	to set the combiner and the individual gates and inputs.
  if (CommandIs("Init"))
    {
      ifDS(numGates, combinerName, init(numGates, combinerName));
      return Uncatch();
    }
  /* Switch num_gates		to become a switch statement with num_gates inputs.
     Each gate will multiply it's input by the threshold value.
     The integer value of the sum of the gate values is the condition value.
  */
  if (CommandIs("Switch"))
    {
      ifD(numGates, init(numGates, SUM_Combiner,
			 SwitchInputGate::Mult, 1, 0));
      //return Uncatch();
      ifNil(init(1, SUM_Combiner,
		 SwitchInputGate::Mult, 1, 0));	// no args, so be a 1-input switch
    }

  // RollOverflow/ResetOverflow   to rollover or reset inputs that have exceed their testVals.
  if (CommandIs("RollOverflow"))
    {
      ifNil(rollOverflow());
    }
  if (CommandIs("ResetOverflow"))
    {
      ifNil(resetOverflow());
    }


  // MultiEval  to evaluate all cases matching gates that are true.
  if (CommandIs("MultiEval"))
    {
      ifNil(multiEval());
    }
  
  /* AssertGates [ bools ].  For each gate, if the corresponding value in
     bools is not -1, then test the gate. */
  
  if (CommandIs("AssertGates"))
    {
      ifIntArray(bools, count, assertGates(bools, count));
    }
  
  //--------------- Message grouping and build stuff ---------------
  if (CommandIs("SetCombine"))
    {
      ifS(combinerName, setCombiner(getSwitchCombiner(combinerName)));
      return Uncatch();
    }
  if (CommandIs("Case"))
    {
      ifD(num, setCase(num));
      return Uncatch();
    }
  if (CommandIs("IfTrue"))
    {
      ifNil(setCase(1));
    }
  if (CommandIs("IfFalse"))
    {
      ifNil(setCase(0));
    }
  if (CommandIs("AddMessage"))
    {
      ifM( msg, addMessage(msg) );
      return Uncatch();
    }
  
  return baseClass::receiveMessage(Message);
}

/* Use this to initialize a range of gates, that have indices start to
   (end-1).  (i.e. end is one PAST the last gate to change.  This is
   often more convenient and reliable, for callers of method, than
   using the last index directly as an argument.)  Each gate is
   initialized with a tester function (tester) and the test value
   (testVal).  (NOTE: The initial input value for the gate is not set.
   Use setInputRange for that.).

   The list of gates is expanded as necessary.

   PRECOND:	0 <= start < end, else return -1.
   RETURNS:	number of gates set.  (negative if indices are bad.) */
int SwitchActor::setGateRange(int start, int end, SwitchGateTester tester, float testVal)
{
  DF(D_SA, cerr << "setGateRange(start=" << start
     << ", end=" << end
     << ", testVal=" << testVal << ")\n");
  
  if (!(0 <= start && start < end))
    return -1;

  /* NOTE: should use and check trySetCapacity(int) before calling count(int). */
  if (end > gates.count()) {
    gates.count(end);
    inputs.count(end);
  }

  for (SwitchInputGateLI iter(gates); !iter; ++iter) {
    (*iter).tester = tester;
    (*iter).testVal = testVal;
  }
  
  return end-start;
}

int SwitchActor::setGates(SwitchGateTester tester, float testVal)
{
  return setGateRange(0, gates.count(), tester, testVal);
}

/* Decode the string and return a pointer to the gate tester function.
 */
SwitchGateTester SwitchActor::getSwitchGateTester(const char* testerName)
{
  if (!strcmp("<", testerName))		    { return SwitchInputGate::TestLT; }
  else if (!strcmp("<=", testerName))	    { return SwitchInputGate::TestLTE; }
  else if (!strcmp(">", testerName))	    { return SwitchInputGate::TestGT; }
  else if (!strcmp(">=", testerName))	    { return SwitchInputGate::TestGTE; }
  else if (!strcmp("=", testerName))	    { return SwitchInputGate::TestEq; }
  else if (!strcmp("*", testerName))	    { return SwitchInputGate::Mult; }
  else if (!strcmp("+", testerName))	    { return SwitchInputGate::Add; }
  else {
    vssWarningStream() << "SwitchActor could not find function corresponding to name '"
 		       << testerName << "' Defaulting to '>'\n";
    return SwitchInputGate::TestGT;
  }
}

/* Set the current input value for each gate in range start..end-1.
   If any indices are out of range, then do nothing to any of the gates.

   PRECOND:	0 <= start < end   AND end <= number_of_gates.
   RETURN:	number of input values changed, or negative if indices were bad.
 */
int SwitchActor::setInputRange(int start, int end, float inputVal)
{
  DF(D_SA, cerr << "setInputRange(start=" << start
     << ", end=" << end
     << ", inputVal=" << inputVal << ")\n");
  
  if (0 <= start &&
      start < end &&
      end <= inputs.count()) {
    
#if 1
    inputs.fill(inputVal, start, end);
#else
    for (SwitchInputGateLI iter(gates); !iter; ++iter) {
      (*iter)._inputVal = inputVal;
    }
#endif
    return end-start;
    
  }
  else {
    return -1;
  }
}

int SwitchActor::setInputs(float inputVal)
{
  return setInputRange(0, gates.count(), inputVal);
}

/* Copy the float values from inputArray to the input value array for
   the gates.  Up to 'count' gates are initialized starting with gate
   number 'start'.

   If (start+count) exceeds the number of gates, then skip the out of range
   values and CURRENTLY complain about it, to the warning stream.

   PRECOND:	0 <= start AND (start+count)<= gates.count().
   RETURN: number of values copied.  */
int SwitchActor::copyToInputs(const float* inputArray, int count, int start)
{
  DF(D_SA, cerr << "copyToInputs(start=" << start << "): [");
  int end = start+count;
  if (end > gates.count()) {
    // NOTE: be more informative later.
#if 1
    fprintf(stderr,"SwitchActor::copyToInputs: bad indices received.\n"
	    "\tAttempted to copy %d elements to gates starting at %d.",
	    count, start);
#else
    vssWarningStream() << form("SwitchActor::copyToInputs: bad indices received.\n"
			       "\tAttempted to copy %d elements to gates starting at %d.",
			       count, start);
#endif
    end = gates.count();
  }
  inputs.atReplaceFromTo(start, inputArray, 0, count);
  DF(D_SA, cerr << inputs << "]\n");
  return end-start;
}

int SwitchActor::setTestValRange(int start, int end, float testVal)
{
  DF(D_SA, cerr << "setTestValRange(start=" << start
     << ", end=" << end
     << ", testVal=" << testVal << ")\n");
  
  if (0 <= start &&
      start < end &&
      end <= gates.count()) {

    for (int i=start; i<end; i++) {
      gates[i].testVal = testVal;
    }
    
    return end-start;
    
  }
  else {
    return -1;
  }
}

/* Set the current message block. */
void SwitchActor::setCase(int num)
{
  caseNum = num;
  DF(D_SA, cerr << "setCase(" << caseNum << ")\n");
}

/* Add the message to the current message group (indexed by the case num).
   Do this by using the threshold superclass to manage the groups.
   Each message is added under a threshold of the caseNum.
 */
void SwitchActor::addMessage(char* message)
{
  baseClass::addThreshold(caseNum, IntEq, message);
}

SwitchCombiner SwitchActor::getSwitchCombiner(const char* combinerName)
{
  if (!strcmp("AND", combinerName))	    { return AND_Combiner; }
  else if (!strcmp("NAND", combinerName))   { return NAND_Combiner; }
  else if (!strcmp("XOR", combinerName))    { return XOR_Combiner; }
  else if (!strcmp("OR", combinerName))	    { return OR_Combiner; }
  else if (!strcmp("SUM", combinerName))    { return SUM_Combiner; }
  else if (!strcmp("PRODUCT", combinerName)) { return PRODUCT_Combiner; }
  /* LATER add SUM_Combiner, MULT_Combiner, ChooseFirst. */
  else {
    vssWarningStream() << "SwitchActor could not find COMBINER function corresponding to name '"
 		       << combinerName << "' Defaulting to 'AND'\n";
    return AND_Combiner;
  }
}

/*
  Each gate is expected to return 0.0f or 1.0f, which are interpreted as false and true.
  Return Logical AND of these.  This short-circuits evaluation when a false is found.
 */
int SwitchActor::AND_Combiner(SwitchInputGateL& gates, FloatL& inputs)
{
  int count = gates.count();
  for (int i=0; i<count; i++) {
    if (gates[i].isFalse(inputs[i])) return false;
  }
  return true;
}
int SwitchActor::OR_Combiner(SwitchInputGateL& gates, FloatL& inputs)
{
  int count = gates.count();
  for (int i=0; i<count; i++) {
    if (gates[i].isTrue(inputs[i])) return true;
  }
  return false;
}
/* !(A && B && C ...) == !A || !B || !C ...
 */
int SwitchActor::NAND_Combiner(SwitchInputGateL& gates, FloatL& inputs)
{
  int count = gates.count();
  for (int i=0; i<count; i++) {
    if (gates[i].isFalse(inputs[i])) return true;
  }
  return false;
}
/* Compute logical XOR over all values. */
int SwitchActor::XOR_Combiner(SwitchInputGateL& gates, FloatL& inputs)
{
  int fl=false;
  int count = gates.count();
  for (int i=0; i<count; i++) {
    // For XOR, each true value reverses the flag state.
    fl ^= gates[i].isTrue(inputs[i]);
  }
  return fl;
}

/* Sum over all the gates. */
int SwitchActor::SUM_Combiner(SwitchInputGateL& gates, FloatL& inputs)
{
  float sum = 0;
  int count = gates.count();
  for (int i=0; i<count; i++) {
    sum += gates[i].getVal(inputs[i]);
  }
  return (int)sum;
}
/* Multiply all the gates. */
int SwitchActor::PRODUCT_Combiner(SwitchInputGateL& gates, FloatL& inputs)
{
  float prod = 1;
  int count = gates.count();
  for (int i=0; i<count; i++) {
    prod *= gates[i].getVal(inputs[i]);
  }
  return (int)prod;
}

/* Invoke the combiner function and compute the test condition value using all the
   input gates and tests.  */
int SwitchActor::computeTestVal()
{
  int testVal = (*combiner)(gates,inputs);
  DF(D_SA, cerr << "computeTestVal returned: " << testVal << "\n");
  return testVal;
}

/* Compute the test value and call the appropriate message group.
   This is similar to ThresholdActor::testThresholds().
   Don't mess with testThreshold values.
 */
void SwitchActor::evalSwitch(int testVal)
{
  ThreshDeque::iterator it;
  for ( it = threshList.begin(); it != threshList.end(); it++ ) {
    if ((*it)->test( (*it)->thresh, testVal)) {
      (*it)->msg.receiveData( inputs._data(), inputs.count() );
    }
  }
}
/* For each gate that is true, invoke the corresponding case, if any. */
void SwitchActor::multiEval()
{
  int count=gates.count();
  for (int i=0; i<count; i++) {
    if (gates[i].isTrue(inputs[i])) {
      DF(D_SA, cerr << "multiEval: invoking case " << i << "\n");
      evalSwitch(i);
    }
  }
}
void SwitchActor::sendAndEval(const float* inputArray, int count)
{
  copyToInputs(inputArray, count, 0);
  evalSwitch(computeTestVal());
}

/*
  Vomit the internal values.
 */
void SwitchActor::printAll()
{
#if 1
  vssWarningStream() << "Gates: " << gates << " inputs: " << inputs << endl;
#else
  vssOutputStream() << "Gates: " << gates << " inputs: " << inputs << endl;
#endif
}

/* Perform all the initializations to the combiner function and
   individual gates to make this act like the given combiner gate.
 */
void SwitchActor::init(int numGates, const char* combinerName)
{
  init(numGates, getSwitchCombiner(combinerName),
       SwitchInputGate::TestGT, 0, 0);
}
/* Init the combiner, tester gates, test values, and input values. */
void SwitchActor::init(int numGates, SwitchCombiner combinerFunc,
		       SwitchGateTester testerFunc, float testVal,
		       float inputVal)
{
  setCombiner(combinerFunc);
  setGateRange(0, numGates, testerFunc, testVal);
  setInputRange(0, numGates, inputVal);
}

/* Add floats to all the inputs, starting at the given index. */
void SwitchActor::addToInputs(const float* data, int count, int start)
{
  int end = start+count;
  if (end > gates.count()) {
    end = gates.count();
  }
  for (int i=start; i<end; i++) {
    inputs[i] += data[i];
  }
  DF(D_SA, cerr << "Inputs after adding:" << inputs << "\n");
}
/* Similarly subtract floats from all inputs. */
void SwitchActor::subtractFromInputs(const float* data, int count, int start)
{
  int end = start+count;
  if (end > gates.count()) {
    end = gates.count();
  }
  for (int i=start; i<end; i++) {
    inputs[i] -= data[i];
  }
  DF(D_SA, cerr << "Inputs after subtracting:" << inputs << "\n");
}

/* Check each gate for overflow, and subtract into range, useful modulo.
 */
void SwitchActor::rollOverflow()
{
  int sz = inputs.count();
  float t;
  for (int i=0; i<sz; i++) {
    if (inputs[i] >= gates[i].testVal) {
      float& j=inputs[i];	// this will directly modify inputs[i],
				// since j is a reference to the data in inputs.
      t = gates[i].testVal;
      if (t > 0)
	while (j > t) {
	  j -= t;
	}
    }
  }
}
/* Check each gate for overflow, and for those, set input to 0.
 */
void SwitchActor::resetOverflow()
{
  int sz = inputs.count();
  for (int i=0; i<sz; i++) {
    if (inputs[i] >= gates[i].testVal)
      inputs[i] = 0;
  }
}
/* Treat each gate tester as a logic function.  For each int in bools,
   that is 0 or 1 (i.e. is >= 0), check if the corresponding gate is
   also true or false.  If all these tests are met, then invoke the
   case given by trueCase, or else invoke falseCase.  To invoke
   nothing, use a non-existent case, like -100.

   NOTE: this implicitly uses AND to combine.  Could use anything
   though.  */
void SwitchActor::assertGates(int* bools, int count, int trueCase, int falseCase)
{
  int sz = inputs.count();
  sz = (sz > count) ? count : sz; 
  for (int i=0; i<sz; i++) {
    if (bools[i] >= 0 &&
	((bools[i]!=0) != (gates[i].isTrue(inputs[i])))) {
      DF(D_SA, cerr << form("Assert Failed on gate %d. (bools:%d != %d) Invoking False case: %d.\n",
			    i, bools[i], gates[i].isTrue(inputs[i]), falseCase));
      evalSwitch(falseCase);
      return;
    }
  }
  DF(D_SA, cerr << "Assert True. Invoking True case: " << trueCase << "\n");
  evalSwitch(trueCase);
}

/*
  Eventually put this is global scope.  This should return a subclass
  of ostream that will print strings in a different color or
  optionally log warning messages to a file, etc.

  CURRENTLY: just return cerr.  */
ostream& SwitchActor::vssWarningStream()
{
  return cerr;
}

#if 0
/* Similarly, this is for the regular output.
 */
ostream& SwitchActor::vssOutputStream()
{
  return cout;
}
#endif

// #endif
