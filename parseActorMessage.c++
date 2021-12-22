#include <cstring> // for strstr()
#include "VActor.h"

//===========================================================================
// sscanf-parsing support.

// These functions are not "re-entrant" in spirit, since they
// use a fixed set of variables to maintain state from a
// CommandFromMessage() call through the various calls to
// CommandIs() and ifFFF().  In special cases it *may* work
// to call CommandFromMessage() twice (say, to parse a command
// embedded in another command such as in AddMessage or AddEvent);
// but this is not guaranteed.
// Generally, commands are parsed one at a time, so re-entrancy is not demanded.

// temporaries to hold data during the sscanf-parsing of a message
const char* sscanf_msg = NULL;
char sscanf_cmd[1000] = {0};

// extract command (1st token) from *message,
// skips over actor handle (2nd token),
// leaves *message pointing to remaining tokens for further parsing.

void CommandFromMessage(const char* message, int fGenActor)
{
	int cch = 0;
	if (!message)
		{
		cerr << "vss error: null message.  Crash is imminent.\n";
		return;
		}
	if (!*message)
		cerr << "vss error: empty message.\n\tMaybe your client used vss 2.3's libsnd.a instead of vss 4.0's?\n";
	else
		{
		int n = sscanf(message, fGenActor ? "%s %n" : "%s %*f%n",
			sscanf_cmd, &cch);
		if (n != 1)
			{
			cerr << "garbled message <" << message << ">: ignoring\n";
			*sscanf_cmd = '\0';
			}
		}
	sscanf_msg = message + cch;
//printf("\t\t\t\tmessage: \"%s\"\n", message /*, sscanf_msg*/);
}

// This gets stuffed by MessageGroup::receiveData, and read by SscanfFloats.
// Bogus indices into this are NOT tested for.
// If you use "*4" peculiarly, you may get garbage data.  Caveat user.
constexpr auto czMG = 200;
float vrgzMG[czMG];
float* VrgzMG()
{
	return vrgzMG;
}

// Match up to cz (possibly 0) floats in sz.  Stuff them in rgz.
// Return # of floats matched, or -1 if none were.
int SscanfFloats(int cz, float* rgz, const char* sz) {
	char ch;
	int cch;
	// Parse leading '['.  sscanf(" [%n") wouldn't distinguish a [ from a missing [.
	if (1 != sscanf(sz, " %c %n", &ch, &cch) || ch != '[') {
		fprintf(stderr, "vss: ignoring message whose arguments lacks \"[x0 x1 ...]\": \"%s\"\n", sz);
		return -1;
	}
	const char* sz0 = sz;
	sz += cch;

	// Parse cz floats.
	int i = 0;
	for (; i < cz; ++i) {
		if (1 == sscanf(sz, "%f %n", &rgz[i], &cch))
			goto LContinue;
		int w;
		if (1 == sscanf(sz, "*%d %n", &w, &cch)) {
			if (w < 0 || w > czMG) {
				fprintf(stderr, "out-of-range potential messagegroup index *%d\n", w);
				rgz[i] = 0.;
			} else {
				rgz[i] = vrgzMG[w];
				//printf("\t%d'th float in array is now vrgzMG[%d] == %f\n", i, w, vrgzMG[w]);
			}

			// special test for "*4 to *9" or "*4 to *$"
			int cch2;
			if (1 == sscanf(sz, "to *%d %n", &w, &cch2)) {
				//;; NYI: side effect of assigning to rgz
				cch += cch2;
			} else if (!strncmp(sz, "to *$", 5)) {
				cch += 5;
			}
LContinue:
			sz += cch;
			continue;
		}
		break;
	}

	// We should hit the ']' after the last float.
	// Ignore any more floats before the ']'.
	// Complain about anything else before the ']'.
	// Complain and don't ignore extra "*4" vrgzMG guys.

	// Parse more floats
	float dummy;
	while (1 == sscanf(sz, "%f %n", &dummy, &cch))
		sz += cch;

	// Now we better have the ']'.
	if (1 != sscanf(sz, " %c %n", &ch, &cch) || ch != ']') {
		if (NULL != strstr(sz, "inf") || NULL != strstr(sz, "NaN"))
			fprintf(stderr, "vss error: NaN in array of floats.\n");
		else
			fprintf(stderr, "vss error: syntax error in array of floats, at \"%s\" in \"%s\".\n",
			sz, sz0);
		return -1;
	}

	// If you want to do SscanfFloatsAndOtherStuff() based on 
	// this function SscanfFloats, put more parsing stuff on sz here.
	return i;
}

// Match up to cw (possibly 0) ints in sz.  Stuff them in rgw.
// Return # of ints matched, or -1 if none were.
// Used only by thresh/switchActor.c++ ifIntArray.  Retire?
int SscanfInts(int cw, int* rgw, const char* sz) {
	char ch;
	int cch;
	// Parse leading '['.  sscanf(" [%n") wouldn't distinguish a [ from a missing [.
	if (1 != sscanf(sz, " %c %n", &ch, &cch) || ch != '[')
		return -1;
	sz += cch;

	// Parse cw ints.
	auto i = 0;
	for (; i < cw; ++i) {
		if (1 != sscanf(sz, "%d %n", &rgw[i], &cch))
			break;
		sz += cch;
	}

	// We should hit the ']' after the last int.
	// Ignore any more ints before the ']'.
	// Complain about anything else before the ']'.

	// Parse more ints.
	int dummy;
	while (1 == sscanf(sz, "%d %n", &dummy, &cch))
		sz += cch;

	// Now we better have the ']'.
	if (1 != sscanf(sz, " %c %n", &ch, &cch) || ch != ']') {
		fprintf(stderr, "SscanfInts: garbage before the ].\n");
		return -1;
	}
	return i;
}
