/////////////////////////////////////////////////
//
//	Implementation of a class of objects 
//	representing a Python interpreter.
//
//  -kel 20 Sept 99
//
////////////////////////////////////////////////

#include "PyInterp.h"
#include <map>

#include "frozen.h"

//	initialization of the vss message module:
extern "C" void initvssSrv();

//
//	global map of namespace names (strings) to PyInterps.
//
static std::map< std::string, PyInterp > Namespaces;

//
//	static member for getting a Python namespace 
//	(PyInterp) by name. The default name is the empty string.
//
PyInterp *
PyInterp::instance( const std::string & name )
{
	// std::cerr << "looking up namespace " << name << std::endl;
	
	PyInterp & ref = Namespaces[ name ];
	if (! ref.ready() ) {
		// cerr << "initializing with name " << name << std::endl;
		ref.init( name );
	}

	// std::cerr << "returning " << & ref << std::endl;

	return & ref;
}

//
//	Python initialization:
//	(static)
//
bool
PyInterp::init_python( void )
{
	static bool _initialized = false;

	if ( ! _initialized ) {
		//	from frozen.h
		PyImport_FrozenModules = _PyImport_FrozenModules;

		std::cerr << "\tInitializing Python interpreter..." << std::endl;
		Py_SetProgramName("vss");
		Py_Initialize();

		//	print out some extra crap for debugging:
		if ( Py_IsInitialized() )
			std::cerr << "\tPython successfully initialized" << std::endl;

		std::cerr << "\tPython " << Py_GetVersion() 
			<< " on " << Py_GetPlatform() << std::endl
			<< '\t' << Py_GetCopyright() << std::endl;

		std::cerr << "initializing vssSrv module..." << std::endl;
		initvssSrv();   

		//	this is critical!
		std::cerr << "loading __main__..." << std::endl;
		int	n = PyImport_ImportFrozenModule("__main__");
		if (n == 0)
			Py_FatalError("__main__ not frozen");
		if (n < 0) {
			PyErr_Print();
		}

		_initialized = true;

		std::cerr << "\tdone." << std::endl;
	}

	return _initialized;
}

//
//	copyGlobals
//
//	Return a copy of the globals Python 
//	object. 
//	(static)
//
PyObject *
PyInterp::copyGlobals( void )
{
	PyObject * g = NULL;

	if ( init_python() ) {
		//	object if the interpreter is
		//	not initialized:
		if (! Py_IsInitialized() ) {
			std::cerr << "PyInterp tried to make a copy of the globals" << std::endl
				<< "dictionary of an uninitialized interpreter!" << std::endl;
			return g;
		}

		//	get a reference to the current globals:
		PyObject * z = PyModule_GetDict( PyImport_AddModule("__main__") );
		if ( ! z ) 
			return g;

		Py_INCREF( z );

		//	make a copy:
		g = PyObject_CallMethod( z, "copy", NULL );
		Py_XINCREF( g );

		Py_DECREF(z);
	}

	return g;
}

//
//	PyInterp constructor
//
PyInterp::PyInterp( void ) :
	_globals( NULL ),
	_name( "" )
{
	// std::cerr << "constructed " << this << std::endl;
}

//
//	init
//
//	Give it a name and a global dictionary.
//
void
PyInterp::init( const std::string & name )
{
	_globals = copyGlobals();
	_name = name;

    if ( ! _globals ) {
        std::cerr << "PyInterp couldn't get global dictionary." << std::endl
            << "I won't be doing anything very useful, I'm afraid." << std::endl;
        return;
    }

	//	identify the interpreter for
    //  purposes of vss messaging:
    std::string s( "setNamespace('" );
    s.append( _name );
    s.append("')");
    execute(s);

    std::cerr << "PyInterp got global dictionary, ready to roll." << std::endl;
    // std::cerr << "initialized " << this << " named " << _name << std::endl;
}

//
//	PyInterp destructor
//
PyInterp::~PyInterp( void )
{
	Py_XDECREF(_globals);

	// std::cerr << "destroying " << this << std::endl;
}

//
//  PyInterp copy constructor
//
//	Need this because I store PyInterps in a STL map,
//	so there's lots of copying going on. I could instead
//	store PyInterp pointers, but these are so small that
//	it isn't worth the extra headaches of trying to use
//	pointers safely. 
//
//	This copy constructor is essential though, because 
//	the reference count for the global dictionary needs
//	to be maintained.
//
PyInterp::PyInterp( const PyInterp & other ) :
	_globals( other._globals ),
	_name( other._name )
{
    if ( _globals != NULL ) {
        Py_INCREF( _globals );
    }

    // std::cerr << "copying PyInterp:" << this << " named " << name() << std::endl;
}

//
//  PyInterp assignment
//
//  Needed for the reasons as the copy constructor.
//
PyInterp &
PyInterp::operator=( const PyInterp & rhs )
{
	// std::cerr << "assigning PyInterp:" << &rhs << " named " << rhs.name() << " to " << this << std::endl;
    if ( &rhs != this ) {
        Py_XDECREF(_globals);
        _globals = rhs._globals;
        Py_XINCREF( _globals );
        _name = rhs._name;
    }
    return *this;
}

//
//	execute()
//
//	PyRun_String is used to execute code in a specified
//	context (the global dictionary is passed in).
//	The return value can be checked, etc. 
//	Using Py_single_input, I can send any code I
//	want, but the return is either None (success)
//	of NULL (failure). Not really very useful.
//	Using Py_eval_input, I am (very) restricted
//	in what I can send. Like, no assignments or
//	prints. There must be more of these, but I 
//	cant find any friggin' documentation!
//
//
float
PyInterp::execute( const std::string  & realstr )
{
//	This is really stupid, there's no way that the
//	string passed to the Python interpreter
//	shouldn't be const.
	char * str = const_cast< char * >( realstr.c_str() );

	//	trim leading white space:
	while ( *str == ' ' )
		++str;

	// std::cerr << "\tExecuting: " << str << std::endl;

	PyObject * rv = NULL;
	float rf = -1;

	//  execute the string:
	rv = PyRun_String(str, Py_single_input, _globals, _globals);

   //  check for errors:
	if ( !rv ) {
		//std::cerr << "sad: return value is null." << std::endl;
		if ( PyErr_Occurred() ) {   // this must be true if rv is null
			std::cerr << "Python exception thrown." << std::endl;
			PyErr_Print();
		}
	}
	else {	
		//	in Py_single_input mode, there is 
		//	never a return value from Python, other
		//	than None, so just return 0 if execution
		//	succeeded without error:
		rf = 0;
	}

    //  clean up:
    Py_XDECREF(rv);

	return rf;
}

//
//	evaluate()
//
//	This is just like execute, except that it uses 
//	Py_eval_input instead of Py_single_input, so it
//	is of more limited use, but it can return the
//	result of the evaluation.
//
std::string
PyInterp::evaluate( const std::string & realstr )
{
//	This is really stupid, there's no way that the
//	string passed to the Python interpreter
//	shouldn't be const.
	char * str = const_cast< char * >( realstr.c_str() );

	//	trim leading white space:
	while ( *str == ' ' )
		++str;

	PyObject * rv = NULL;
	std::string ret;

	//  evaluate the string:
	//std::cerr << "evaluating " << str << std::endl;
	rv = PyRun_String(str, Py_eval_input, _globals, _globals);

   //  check for errors:
	if ( !rv ) {
		//std::cerr << "sad: return value is null." << std::endl;
		if ( PyErr_Occurred() ) {   // this must be true if rv is null
			std::cerr << "Python exception thrown." << std::endl;
			PyErr_Print();
		}
	}
	else {	
		std::cerr << "evaluation succeeded." << std::endl;
		//	try to return a string:
		PyObject * pystr = PyObject_Str( rv );
		if ( pystr == NULL )
			std::cerr << "Python return value couldn't be converted to a string." << std::endl;
		else {
			char * c = PyString_AsString( pystr );
			ret.assign(c);
			//	???
			//	??? I assume I have to free the c-string, right?
			//	on the other hand, it doesn't dump core if I 
			//	don't free it...
			//	???
			//free(c);

			//	 I am pretty sure that I own this reference.
			Py_DECREF( pystr );
		}
			
	}

    //  clean up:
    Py_XDECREF(rv);

	//std::cerr << "returning " << ret << std::endl;
	return ret;
}
