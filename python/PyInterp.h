#ifndef __interp__
#define __interp__
/////////////////////////////////////////////////
//
//  Definition of a class of objects
//  representing a Python interpreter.
//
//	There is only one actual Python interpreter
//	running. Each PyInterp has its own global
//	dictionary, so it appears to be isolated.
//
//	-kel 20 Sept 99
//
////////////////////////////////////////////////
#include <string>
#include "Python.h"

class PyInterp
{
public:
//	instance access:
	static PyInterp * instance( const std::string & name = "" );

//	construction:
	PyInterp( void );
	PyInterp( const PyInterp & );
	~PyInterp( void );

	PyInterp & operator=( const PyInterp & rhs );

//	public interface:
	float execute( const std::string & realstr );
	std::string evaluate( const std::string & realstr );

    const std::string & name( void ) const { return _name; }

//	initialization:
	bool ready( void ) const { return _globals != NULL; }
	void init( const std::string & name );
private:
	static bool init_python( void );
	static PyObject * copyGlobals( void );

//	instance variables:
	PyObject * _globals;
	std::string _name;

};	//end of class PyInterp

#endif	// ndef __interp__
