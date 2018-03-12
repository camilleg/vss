#if !defined(__pyactor_header__)
#define __pyactor_header__
/////////////////////////////////////////////////
//
//  Definition of a class of vss actors
//  wrapping a Python interpreter.
//
//  -kel 20 Sept 99
//
////////////////////////////////////////////////
#include "VActor.h"
#include <string> 

class PyInterp;

//===========================================================================
//		class PyActor
//
//
class PyActor : public VActor	
{
public:
	PyActor();
	~PyActor();

//	actor behavior
	virtual	int receiveMessage(const char*);
	virtual ostream & dump( ostream & os, int tabs );	

private:
//	message handling:
	float execute(const char *);
	void evaluate(const char *);
	void setNamespace(const char *);

//	instance variables:
//	for safety, we should look up the interpreter by
//	name every time we use it; for efficiency, we should
//	avoid extra tree searches for a pointer whose value
//	should rarely change; you figure it out.
#define __safe__
#ifdef __safe__
	std::string _namespace;	//	the name of my interpreter instance
#else
	PyInterp * _interp;		//	my interpreter wrapper
#endif

};	// 	end of class PyActor

#endif	//	ndef __pyactor_header__
