
typedef struct
{
	union {
	short rgl[1];	// placeholder for bigger array
	float rgz[1];
	};
} Member;
const short sHuge = 0x7fff;

Member* GADistanceMatrix(int cptArg, int cdimSrcArg, int cdimDstArg,
	float* rgzSrc);
