#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h> // for memcmp(), memcpy()
#include <sys/time.h>
#include "platform.h"
#include "ga.h"

//extern float* vpz;
//inline void x(int _) { if (vpz) printf("\t\t\t\t\t\t\t\t# %d %g\n", _, *vpz); }

const long cMaxGen = 1000;	// max # generations to try
int  cBest;			// how many fittest members to keep each gen.
const int  MAXPOP    = 1000;	// maximum population

void* rgpop;
int cbMember = 0;
float zFitnessMax, zFitnessMin;

void (*GenerateRandom)(void* pv);
void (*MutateRandom)(void* pv, long cIter);
void (*Tweak)(void* pv);
float (*ComputeSuitability)(void* pv);

float suitability[MAXPOP];
int nextidx[MAXPOP];
int nextfast[MAXPOP];

#ifdef VSS_WINDOWS
inline long myrandom(void) { return random(); }
#else
#ifdef DEBUG
long myrandom(void)
{
	static int i=0;
	long l = random();
	if (!(++i %= 5000)) printf("%d\n", l);
	return l;
};
#else
inline long myrandom(void) { return random(); }
#endif
#endif

inline int Rand( int x )  { return myrandom() % x; }
//inline void Swap( int& x, int& y ) { int t = x; x = y; y = t; }

inline void* PvFromI(int i)
{
	return (void*)((char *)rgpop + i * cbMember);
}

void AllocPopulation(int cMember)
{
	rgpop = calloc(cMember, cbMember);
}

void FreePopulation(void)
{
	free(rgpop);
}

inline int fMembersEQ( int i, int j )
{
	return memcmp(PvFromI(i), PvFromI(j), cbMember) == 0;
}

void Breed(int c1, int c2, int p1, int p2)
{
	long lMask;
	int cbLeft = cbMember;

	long* plc1 = (long*)PvFromI(c1);
	long* plc2 = (long*)PvFromI(c2);
	long* plp1 = (long*)PvFromI(p1);
	long* plp2 = (long*)PvFromI(p2);

	while (cbLeft > 0)
		{
		lMask = myrandom();
		*plc1++ = (*plp1   & lMask) | (*plp2   & ~lMask);
		*plc2++ = (*plp2++ & lMask) | (*plp1++ & ~lMask);
		cbLeft -= sizeof(long);
		}

	if (Tweak)
		{
		Tweak(PvFromI(c1));
		Tweak(PvFromI(c2));
		}
}

void NewMemberAt(int i)
{
	GenerateRandom(PvFromI(i));
	suitability[i] = 0.;
	nextidx[i] = -1;
}

void ProduceInitialMembers( int cInitPop )
  {
  for( int i = 0; i < cInitPop; i++ )
    NewMemberAt( i );
  }

int    iBestMember;         // index of current best member
static int cFittest;
int RankAndCalculateFitness( int cPop, int cTopMost )
{
  int i;
  int c;
  int bi;
  float bv;
  int iLast;

  iBestMember = 0;
  cFittest = cTopMost;  // for next function's use
  if (cFittest == 0)
	printf("\n\n\n\t\t\tcoredump imminent 0\n\n");

  for( i = 0; i < cPop; i++ )
    nextidx[i] = -1;

  for( i = 0; i < cPop; i++ )
    {
    suitability[i] = ComputeSuitability(PvFromI(i));
    //;; cache retval of ComputeSuitability with a checksum on arg using cbMemberArg.
    if( suitability[i] == zFitnessMax )
      {
      iBestMember = i;
      return i;     // a correct solution was found!
      }
    }
#define GADEBUG
#ifdef GADEBUG
  int fDump = 0;
LDump:
#endif
  iLast = -1;
  for( c = 0; c < cTopMost; c++ )
    {
    bv = zFitnessMin;  // best value
    bi = -1;  // best index
    for( i = 0; i < cPop; i++ )   // find the next most suitable member
      {
#ifdef GADEBUG
      if (fDump)
	printf("%d\t%d\t%g\t\t\t", i, nextidx[i], suitability[i]);
#endif
      if( nextidx[i] == -1 && suitability[i] > bv )
        {
        bv = suitability[i];
        bi = i;
        }
      }

    if( bi == -1 )
      {
      cFittest = c;
	  if (cFittest == 0)
		{
		printf("\n\n\n\t\t\tcoredump imminent 1\n\n");
#ifdef GADEBUG
		if (fDump) break; // go on and dump core
		fDump = 1;
		goto LDump;
#endif
		}
      break;
      }

    if( iLast == -1 )     // if first one, remember that
      iBestMember = bi;
    else                  // else put member into list
      nextidx[iLast] = bi;

    // if you want to count how many times it is in the top, here's where to do it
    iLast = bi;
    nextidx[bi] = -2;  // so you don't pick this one again
    }

  return -1;
}

// parents are chosen from the top individuals (ranked by previous function)
// all unranked members are replaced with new members
int BreedNewMembers( int cCurrPop, int cEndPop )
{
  int i, j;
  int n1, n2;
  int c1, c2;
  static int wCrud = 0;

LRestart:
    {
    //ASSERT( cEndPop <= MAXPOP );
    for( i = cCurrPop; i < cEndPop; i++ )
      nextidx[i] = -1;
    }

// find duplicates, set suitability to -1
// this is expensive (38% for TETRAHEDRON), so do it rarely.
  if (++wCrud > 7)
	  {
	  wCrud = 0;
	  int cCrud = 0;
	  for( i = 0; i < cCurrPop; i++ )
	    {
	    if( suitability[i] != zFitnessMin )
	      for( j = i+1; j < cCurrPop; j++ )
		{
		if( fMembersEQ( i, j ) )
		  {
		  suitability[i] = zFitnessMin;
		  break;
		  }
		}
	    if( suitability[i] == zFitnessMin )
	      cCrud++;
	    }

	// For all members with suitability == -1, generate a new random member.
	// NewMemberAt() cuts the nextidx[] chain, forcing a RankAndCalculateFitness(),
	// so do this only if there's a lot of crud (suitability == -1).
	  if (cCrud > cCurrPop / 5)
		{
#ifdef NOISY
		printf("decrud %d  ", cCrud);
#endif
		  for( i = 0; i < cCurrPop; i++ )
		    if( suitability[i] == zFitnessMin )
			NewMemberAt( i );
		(void)RankAndCalculateFitness( cCurrPop, cBest );
		goto LRestart;
		}
	  }

  j = iBestMember;
  for (i = 0; i < cFittest; i++)
    {
    nextfast[i] = j;
    j = nextidx[j];
    }

  c1 = -1;
  for( i = 0; i < cEndPop; i++ )
    {
    if( nextidx[i] != -1 )	// skip who we're going to keep
      continue;

    if( c1 == -1 )		// find two places for new children
      {
      c1 = i;
      continue;
      }
    else
      c2 = i;

    n1 = Rand(cFittest);
    do n2 = Rand(cFittest);
      while( n2 == n1 );

    Breed(c1, c2, nextfast[n1], nextfast[n2]);

    c1 = -1;    // to find two new children
    }

  return cEndPop;
}

void Mutate( int count, int cPop, long cIter )
{
	for (int i = 0; i < count; i++ )
		{
		// make one mutation in the list
		MutateRandom(PvFromI(Rand(cPop)), cIter);
		}
}

char* pbBuf = NULL;
void* GA(
	int cbMemberArg,
	void (*pfnGenerateRandom)(void* pv),
	void (*pfnMutateRandom)(void* pv, long cIter),
	void (*pfnTweak)(void* pv),
	float (*pfnComputeSuitability)(void* pv),
	float zSuitabilityMaxArg,
	float zSuitabilityMinArg,
	int cBestArg,
	float tMaxSec
	)
{
	struct timeval tStart, tNow;
	int iSolution;
	int cPopulation;
	float   BestSuitEver = zSuitabilityMinArg;
//	long    BestIteration;
//	int     BestMemberIndex;

#ifdef __GNUC__
	gettimeofday(&tStart, 0);
#else
	gettimeofday(&tStart);
#endif
	cbMember = cbMemberArg;
	pbBuf = NULL;
	if (cbMember % sizeof(long) != 0)
		{
		// round up cbMember to nearest long.
		cbMember += sizeof(long) - (cbMember % sizeof(long));
		}

	GenerateRandom = pfnGenerateRandom;
	MutateRandom = pfnMutateRandom;
	Tweak = pfnTweak;
	ComputeSuitability = pfnComputeSuitability;
	zFitnessMax = zSuitabilityMaxArg;
	zFitnessMin = zSuitabilityMinArg;
	cBest = cBestArg;

	AllocPopulation(MAXPOP);
	if (pbBuf != NULL)
		free(pbBuf);
	pbBuf = (char*)malloc(cbMember);
	srandom(42);
	cPopulation = MAXPOP;
	ProduceInitialMembers( cPopulation );
	iSolution = RankAndCalculateFitness( cPopulation, cBest );
#ifdef NOISY
	if (iSolution != -1)
		printf("found it right away.\n");
#endif
	long cIter = 0L;
	while(iSolution == -1 && cIter <= cMaxGen)
		{
		cIter++;
		cBest = (int)(cBestArg / (1 + sqrt((double)cIter+5.)*.08));
		cPopulation = BreedNewMembers( cPopulation, MAXPOP );
		Mutate((int)(cBest * .4), MAXPOP, cIter);
		iSolution = RankAndCalculateFitness( cPopulation, cBest );

		if( suitability[iBestMember] > BestSuitEver )
			{
			BestSuitEver = suitability[iBestMember];
#ifdef NOISY
			printf("\n\tbest: %.3g  ", zFitnessMax-BestSuitEver);
#endif
			memcpy(pbBuf, PvFromI(iBestMember), cbMember);
		//	BestIteration = cIter;
		//	BestMemberIndex = iBestMember;
			}
	//	putchar('.');fflush(stdout);
#ifdef __GNUC__
		gettimeofday(&tNow, 0);
#else
		gettimeofday(&tNow);
#endif
		if ((float)(tNow.tv_sec - tStart.tv_sec) > tMaxSec)
			{
#ifdef NOISY
			printf("\ntimeout\n");
#endif
			break;
			}
		}
	FreePopulation();
	return (void*)pbBuf;
}
