#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

// Play what line-in hears.
class inputAlg : public VAlgorithm
{
public:
	void generateSamples(int);
	inputAlg();
	~inputAlg();
};

class inputHand : public VHandler
{
protected:
	inputAlg* getAlg() { return (inputAlg*)VHandler::getAlg(); }
public:
	inputHand(inputAlg* alg = new inputAlg);
	~inputHand() {}
	int receiveMessage(const char*);
};

class inputActor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new inputHand(); }
	inputActor();
	~inputActor() {}
	void sendDefaults(VHandler*);
	int receiveMessage(const char*);
};
