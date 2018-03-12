#ifndef _INPUT_H_
#define _INPUT_H_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//===========================================================================
//		inputAlg 
//
//	class inputAlg plays vss's audio input port on class inputAlg's output.
//
class inputAlg : public VAlgorithm
{
public:

//	sample generation
	void	generateSamples(int);

//	construction/destruction
		inputAlg(void);
		~inputAlg();

};	// end of class inputAlg

//===========================================================================
//		inputHand 
//
//	class inputHand is a handler class for inputAlg.
//
class inputHand : public VHandler
{

//	Algorithm access:
// 	Define a version of getAlg() that returns a pointer to inputAlg.
protected:
	inputAlg * getAlg(void)	{ return (inputAlg *) VHandler::getAlg(); }

//	parameter modulation
public:

//	construction
		inputHand(inputAlg * alg = new inputAlg);

//	destruction
virtual	~inputHand() {}

	int receiveMessage(const char * Message);

};	// end of class inputHand

//===========================================================================
//		inputActor
//
//	class inputActor is a generator actor class for dumbfmAlg
//
class inputActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new inputHand(); }

//	construction/destruction
public:
	inputActor(void);
virtual	~inputActor() {}

virtual	void 	sendDefaults(VHandler *);
virtual int		receiveMessage(const char * Message);

};	// end of class inputActor

#endif // ndef _INPUT_H_
