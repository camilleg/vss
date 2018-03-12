/////////////////////////////////////////////////
//
//  Implementation of a class of vss actors
//  wrapping a Python interpreter.
//
//  -kel 20 Sept 99
//
////////////////////////////////////////////////
#include "PyActor.h"
#include "PyInterp.h"
#include <string>

//===========================================================================
//	dso magic
//
#include "actorDso.h"
extern char* actorlist[]; char* actorlist[] = { "PyActor", "" };
DSO_DECLARE(PyActor, PyActor)

//===========================================================================
//		construction
//
PyActor::PyActor() :
#ifdef __safe__
	_namespace( "" )
#else
	_interp( PyInterp::instance() )
#endif
{
	setTypeName("PyActor");
}

//===========================================================================
//		destruction
//
PyActor::~PyActor()
{
}

//===========================================================================
//		receiveMessage
//
int 
PyActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("Execute"))
	{
		//	use ifM to get the entire remaining string:
		//	ifS gets only up to the next whitespace
		ifM( msg, execute(msg) );
		return Uncatch();
	}

	if (CommandIs("Evaluate"))
	{
		//	use ifM to get the entire remaining string:
		//	ifS gets only up to the next whitespace
		ifM( msg, evaluate(msg) );
		return Uncatch();

	}

	if (CommandIs("Namespace"))
	{
		ifS( name, setNamespace( name ) );
		ifNil( setNamespace("") );
		return Uncatch();
	}

	return VActor::receiveMessage(Message);
}

//===========================================================================
//	execute
//
float
PyActor::execute( const char * s )
{
#ifdef __safe__
	PyInterp * _interp = PyInterp::instance( _namespace );
#endif

	if (! _interp) {
		cerr << "yikes! PyActor found itself with no interpreter!" << endl;
		return -1;
	}

	std::string better( s );

	// cerr << "PyActor executing: " << better << endl;
	float f = _interp->execute(better);
	// cerr << "result: " << f << endl;
	ReturnFloatToClient( f );
	return f;
}

//===========================================================================
//	evaluate
//
void
PyActor::evaluate( const char * s )
{
#ifdef __safe__
	PyInterp * _interp = PyInterp::instance( _namespace );
#endif

	if (! _interp) {
		cerr << "yikes! PyActor found itself with no interpreter!" << endl;
		return;
	}

	std::string better( s );

	//cerr << "PyActor evaluating: " << better << endl;
	std::string res = _interp->evaluate(better);
	//cerr << "result: " << res << endl;
	ReturnStringToClient( res.c_str() );
}

//===========================================================================
//	setNamespace
//
void
PyActor::setNamespace( const char * name ) 
{
#ifdef __safe__
	_namespace = name;
#else
	_interp = PyInterp::instance( name );
#endif
}

//===========================================================================
//  dump
//
ostream &
PyActor::dump(ostream &os, int tabs)
{
    VActor::dump(os, tabs);
#ifdef __safe__
    indent(os, tabs) << "Namespace: " << _namespace << endl;
#endif
	return os;
}
