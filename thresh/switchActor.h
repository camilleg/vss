#pragma once
#include <iostream>		// used for debugging stuff and warning messages.
#include "VActor.h"
#if 1
#include "threshActor.h"
#else
#include "saThreshActor.h"
#endif
#include "cxArray.h"

typedef float (*SwitchGateTester)(float,float);

class SwitchActor;

/* Each input gate compares (using the input tester functon) an input
   parameter (from another message) with a test value stored in the
   gate.

   Each gate performs the following test, when its value is retrieved.
   testerFunc(inputVal, testVal);
   For logic type operations the float values are 0.0 or 1.0 and are
   clamped to int 0 or 1 by the combiner functions.
*/
class SwitchInputGate
{
  friend class SwitchActor;		// temp hack.  use accessors later.
  SwitchGateTester tester;
  float testVal;
  
#if 0
  int active;			// 1 if this gate has been set and has not be deactivated.
#endif
  
public:
  SwitchInputGate() : tester(True), testVal(0) { }

  void	setTestVal(float a)	{ testVal = a; }
  
  float getVal(float a)		{ return (*tester)(a, testVal); }
  int	isTrue(float a)		{ return getVal(a)!=0.0f; }
  int	isFalse(float a)	{ return getVal(a)==0.0f; }

  // For the following tests, a will be the input arg, and b will be the gate's testVal.
  // test that always returns true
  static float True(float /*a*/, float /*b*/)		{ return 1; }
  // always false.
  static float False(float /*a*/, float /*b*/)		{ return 0; }
  static float TestLT(float a, float b)		{ return a<b; }
  static float TestLTE(float a, float b)	{ return a<=b; }
  static float TestGT(float a, float b)		{ return a>b; }
  static float TestGTE(float a, float b)	{ return a>=b; }
  static float TestEq(float a, float b)		{ return a==b; }
  static float Mult(float a, float b)		{ return a*b; }
  static float Add(float a, float b)		{ return a+b; }
  friend std::ostream& operator<<(std::ostream& os, const SwitchInputGate& );
};

class SwitchActorAB : public cxBaseAB, public cxNewDeleteMB
{
public:
  static void  assertionFailed(const char* test, const char* file, int line) {
    //__assert(test,file,line);
    fprintf(stderr, "SwitchActor: assertion failed: %s:%d: %s\n",
	    file, line, test);
  }
};

/* VSJ:
   I am currently using my own vector class template library,
   until I figure out how the STL vector class works.

   The following macro declares an array of floats using the default
   array behavior class, cxDefaultAB.  This default class allocates
   using new/delete.  This macro declares the types SwitchInputGateL
   and SwitchInputGateLI (the iterator class).
   */
#if 1
typedef cxArrayB<SwitchInputGate, SwitchActorAB> SwitchInputGateL;
typedef cxArrayIterB<SwitchInputGate, SwitchActorAB> SwitchInputGateLI;
typedef cxArrayB<float, SwitchActorAB> FloatL;
#else
CXA_ArrayT(SwitchInputGate, cxDefaultAB, SwitchInputGate);
// Defines FloatL and FloatLI.
CXA_ArrayT(float, cxDefaultAB, Float);
#endif

typedef int (*SwitchCombiner)(SwitchInputGateL&, FloatL& inputs);

/* A SwitchActor is intended to act like a generalized switch statement.
   There is a test condition with multiple input values.
 */
class SwitchActor : public ThresholdActor
{
public:
  typedef ThresholdActor baseClass;
#if 0
  enum { FALSE=0 , TRUE=1 };
  //static const int FALSE=0;
  //static const int TRUE=1;
#endif
protected:
  SwitchInputGateL  gates;
  FloatL	    inputs;
  int caseNum;			// current block of messages.
  SwitchCombiner    combiner;	// function that combines gates values into test value.
  
public:
  SwitchActor();
  //~SwitchActor();
  int receiveMessage(const char*);

  /* Init the combiner and all the gate testers and the initial input values. */
  void	init(int numGates, const char* combinerName);
  void	init(int numGates, SwitchCombiner combinerFunc,
	     SwitchGateTester testerFunc=SwitchInputGate::TestGT, float testVal=0, float inputVal=0);
  /* Set the tester functions and testVal for elements start..end-1.  */
  int	setGateRange(int start, int end, SwitchGateTester tester, float testVal);
  /* Set all the gates.  Same return value as setGateRange. */
  int	setGates(SwitchGateTester tester, float testVal);
  /* Set the testVals for elements start..end-1.  */
  int	setTestValRange(int start, int end, float testVal);
  /* Similarly set the input values are each gate to inputVal. */
  int	setInputRange(int first, int last, float inputVal);
  /* Set all the inputs to inputVal.  Same return value as setInputRange. */
  int	setInputs(float inputVal);
  /* Set the input gate values starting at start, to values from inputArray. */
  int	copyToInputs(const float* inputArray, int count, int start=0);
  // Return a gate tester function mapped under testerName.
  SwitchGateTester getSwitchGateTester(const char* testerName);
  // Return a combiner function mapped under combinerName.
  SwitchCombiner getSwitchCombiner(const char* combinerName);

  void setCombiner(SwitchCombiner func)			{ combiner = func; }

  /* Set the current case, that selects the message group to add to.
    FUTURE use:  Put optional rounding behavior here. */
  void setCase(int num);

  void addMessage(char* message);
  
  /* Compare two float numbers for equality, by converting to integers first.
     This is where rounding behavior (and error thresholds) should be put if needed. */
  static int IntEq(float a, float b)	    { return int(a) == int(b); }
  int computeTestVal();

  void evalSwitch(int testVal);
  void sendAndEval(const float* inputArray, int count);
  
  void printAll();		// print current value contents.

  /* These combiners evaluate all the gates as logic gates.
     Each combiner returns 0 or 1, appropriate for use with IfTrue and IfFalse.
   */
  static int AND_Combiner(SwitchInputGateL& gates, FloatL& inputs);
  static int NAND_Combiner(SwitchInputGateL& gates, FloatL& inputs);
  static int OR_Combiner(SwitchInputGateL& gates, FloatL& inputs);
  static int XOR_Combiner(SwitchInputGateL& gates, FloatL& inputs);

  /* Add the outputs of all the gates and return the integer value.
     Useful for multiple cases, since result can be any integer that is
     returned by logic gates. */
  static int SUM_Combiner(SwitchInputGateL& gates, FloatL& inputs);
  /* Multiply the gates and return the integer value. */
  static int PRODUCT_Combiner(SwitchInputGateL& gates, FloatL& inputs);

  // Return the stream to write warning messages to.  Currently just cerr.
  std::ostream& vssWarningStream();
#if 0
  std::ostream& vssOutputStream();	// currently just cout.
#endif
  
  //--------------- Histogram stuff: MultiSwitchActor? ---------------
  // Add values in the float list to the input values for each gate.
  void addToInputs(const float* data, int count, int startIdx=0);
  // like above, but subtract.
  void subtractFromInputs(const float* data, int count, int startIdx=0);

  // For each gate where input >= testVal, subtract testVal, until input < testVal.
  void rollOverflow();
  // For each gate where input >= testVal, set input to 0.
  void resetOverflow();

  // For each gate i whose test condition is true, invoke case i's message group.
  void multiEval();

  //--------------- Testing protocol: TestActor?? ---------------
  void assertGates(int* bools, int count, int trueCase=1, int falseCase=0);
};
