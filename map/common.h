// General things.

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <assert.h>
//;;#ifndef VSS_WINDOWS
#include <unistd.h>
//;;#endif
#include <cfloat> // for DBL_MAX

#include "platform.h"

// constants _____________________________________________________________

#define iNil (-1)
#define Pi2 (2 * 3.141592653589793231)
#ifndef MAXFLOAT
#define MAXFLOAT ((float)3.40282346638528860e+38) // not from math.h?
#endif
#ifndef DBL_MAX
#define DBL_MAX (1.7976931348E+308) // in float.h?
#endif

// functions _____________________________________________________________

inline void crash() {}

#ifdef min
#undef min
#undef max
#endif

inline float min(float i, float j) { return i < j ? i : j; }
inline float max(float i, float j) { return i > j ? i : j; }

inline int min(int i, int j) { return i < j ? i : j; }
inline int max(int i, int j) { return i > j ? i : j; }

// #define min(a,b) ((a) > (b) ? (b) : (a))
// #define max(a,b) ((a) < (b) ? (b) : (a))


// assign values to a point in 3-space
inline void SetPt(float *pz, float x, float y, float z)
	{ pz[0] = x; pz[1] = y; pz[2] = z; }

inline float sq(float z)
	{ return z * z; }

inline void MySystem(const char *sz)
{
	if (system(sz) == -1)
		perror("system() call failed");
}

inline float dist2D(const float a[3], const float b[3])
{
	return fsqrt(sq(b[0] - a[0]) + sq(b[1] - a[1]));
}
inline float dist3D(const float a[3], const float b[3])
{
	return fsqrt(sq(b[0] - a[0]) + sq(b[1] - a[1]) + sq(b[2] - a[2]));
}

inline float dot2D(const float a[2], const float b[2])
{
	return b[0]*a[0] + b[1]*a[1];
}
inline float dot3D(const float a[3], const float b[3])
{
	return b[0]*a[0] + b[1]*a[1] + b[2]*a[2];
}

inline float norm2D(const float a[2])
{
	return fsqrt(sq(a[0]) + sq(a[1]));
}
inline float norm3D(const float a[3])
{
	return fsqrt(sq(a[0]) + sq(a[1]) + sq(a[2]));
}
