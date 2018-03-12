void* GA(					// run the GA.  returns member.
        int cbMemberArg,			// size of population
        void (*pfnGenerateRandom)(void* pv),	// randomly generate a member
        void (*pfnMutateRandom)(void* pv, long cIter),	// mutate a member
        void (*pfnTweak)(void* pv),			// tweak a member
        float (*pfnComputeSuitability)(void* pv),	// fitness function
        float zSuitabilityMax,				// perfect fit
        float zSuitabilityMin,			// worse than what's possible
	int cBestArg,				// # of members to keep per gen.
	float tMaxSec				// timeout
        );
