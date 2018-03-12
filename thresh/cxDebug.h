/*
  Some debuggin macros.

  This should NOT be in header files, but should be placed directly
  in the source files to debug.

  DEFINE cxDEBUG to enable these debugging macros.
  DEFINE cxVERBOSE to enable verbose echo macros.
  DEFINE cxCOUT	    to the io stream that output should be sent to,
		    otherwise a default of cout will be used.
 */

#ifndef cxCOUT
#define cxCOUT	cout
#endif

#ifdef cxDEBUG
#   define D(exp)	exp
#   define DF(f, stmt)	if (f) {stmt;}
// Debug with line info
#   define DL(stmt)	{ cxCOUT << form("%s(%4d): ", __FILE__,__LINE__); stmt; }
#   define DFL(f, stmt)	if (f) { cxCOUT << form("%s(%4d): ", __FILE__,__LINE__); stmt; }

#else
#   define D(exp)
#   define DF(f, exp)
#   define DL(exp)	
#   define DFL(f, exp)	

#endif

#ifdef cxVERBOSE
// verbose flagged print.
#   define VF(f,stmt)	if (f) {stmt;}
// verbose statement with line info.
#   define VL(stmt)	cxCOUT << form("%s(%4d): ", __FILE__,__LINE__); stmt
// verbose echo statement with line info.
#   define VEL(stmt)	cxCOUT << form("%s(%4d): %s;\n", __FILE__,__LINE__, #stmt); stmt

#else
#   define VF(f,stmt)
#   define VL(stmt)
#   define VEL(stmt)

#endif
