#ifdef VSS_MAC // Porting this Intel-specific code might be hard.
#include "VActor.h"
class LedActor: public VActor {};
ACTOR_SETUP(LedActor, LedActor)
#else

#include "ledActor.h"

#include <assert.h>
#include <sys/io.h> // for ioperm()
#include <unistd.h>

ACTOR_SETUP(LedActor, LedActor)

#define outb(a,b) ((void)0)
#define inb(a) ((void)0)

/* /dev/lp[012] */
#define LPT0 (0x3bc)
#define LPT1 (0x378)
#define LPT2 (0x278)
#define BASE LPT0

#define DATA    (BASE+0)
#define STATUS  (BASE+1)
#define CONTROL (BASE+2)

static int __VSS__LedActorCount = 0;

LedActor::LedActor() :
	zBright(0.0),
	zHue(0.0),
	zBlinkPeriod(1.0),
	zBlinkDutyCycle(1.0)
{
	setTypeName("LedActor");

	// Can't have more than one of these, because there's only 1 parallel port.
	// (well, if we use LPT0 LPT1 and LPT2...)
	if (__VSS__LedActorCount++)
		{
		cerr << "vss error: can't have more than one LedActor.  Disabling this one.\n";
		fDisabled = 1;
		return;
		}

	(void)ioperm(BASE, 3, 1);
	// Don't complain if we can't access the parallel port.
	// This ioperm() is only for if vss runs as root,
	// and we forgot to wrap vss inside an ioperm(1).

	// Don't zero the outputs.  Leave them as however they are right now.
}

LedActor::~LedActor()
{
	--__VSS__LedActorCount;
	if (!fDisabled)
		(void)ioperm(BASE, 3, 0);
	// Don't zero the outputs.  We might want them to stay lit.
}

void LedActor::act()
{
	VActor::act();
	if (fDisabled)
		return;

	// Toggle strobe between 1 and 0,
	// so if act() is called at 300 Hz (44K/128), LED update rate is 150 Hz.
	fStrobe ^= 1;

	unsigned char bEffective;
	if (zBlinkPeriod == 0)
		bEffective = b;
	else
		{
		// This elaborate code is better than fmod(),
		// because it keeps the blinking smooth even when
		// zBlinkPeriod is rapidly changing a small amount.
		static float t = 0.;
			{
			static float tPrev = -1.;
			if (tPrev < 0)
				tPrev = currentTime(); // so the while loop below doesn't take too long!

			float tCur = currentTime();
			t += tCur - tPrev;
			tPrev = tCur;
			}
		while (t > zBlinkPeriod)
			t -= zBlinkPeriod;
		// 0 <= t < zBlinkPeriod
		bEffective = (t / zBlinkPeriod < zBlinkDutyCycle) ? b : 0;
		}

	// If data byte has changed, and we're going 0->1, write data byte.
	if (fStrobe)
		{
		if (bEffective != bPrev)
			{
			outb(bEffective, DATA);
			outb(inb(CONTROL) | 0x01, CONTROL);   // set strobe bit
			bPrev = bEffective;
			}
		}
	else
		outb(inb(CONTROL) & 0xFE, CONTROL);   // reset strobe bit
}

void LedActor::setBrightness(float z)
{
	if (z<0.) z=0.;
	if (z>1.) z=1.;
	if (isDebug())
		cerr << "brightness " << z <<endl;
	zBright = z;
	recalc();
}

void LedActor::setHue(float z)
{
	if (z<0.) z=0.;
	if (z>1.) z=1.;
	if (isDebug())
		cerr << "hue " << z <<endl;
	zHue = z;
	recalc();
}

void LedActor::recalc(void)
{
	assert(0. <= zHue);
	assert(zHue <= 1.);
	float z = zHue * 6.;
	/*
	 * z : 0=r  1=rg  2=g  3=gb  4=b  5=br  6=r
	 * zR: -----\\\\\\___________/////------
	 * zG: /////-----------\\\\\\___________
	 * zB: ___________/////-----------\\\\\\
	 *
	 */

	zR = (z<1 || z>5) ? 1 :
		 z<2 ? 2-z :
		 z>4 ? z-4 :
		 0;

	zG = (z>1 && z<3) ? 1 :
		 z<=1 ? z-0 :
		 z<4  ? 4-z :
		 0;

	zB = (z>3 && z<5) ? 1 :
		 z>=5 ? 6-z :
		 z>=2 ? z-2 :
		 0;

	zR *= zBright;
	zG *= zBright;
	zB *= zBright;
	assert(0. <= zR && zR <= 1.);
	assert(0. <= zG && zG <= 1.);
	assert(0. <= zB && zB <= 1.);
	if (zR>.999) zR = .999;
	if (zG>.999) zG = .999;
	if (zB>.999) zB = .999;

	static const int rgRed[4] = {0,2,1,3};
	static const int rgGreen[5] = {0,2,5,6,7};
	static const int rgBlue[6] = {0,2,4,3,5,7};

	b = (rgRed  [(int)floor(zR*4)] << 6) |	// bits 6,7
		(rgGreen[(int)floor(zG*5)] << 3) |	// bits 3,4,5
		(rgBlue [(int)floor(zB*6)])			// bits 0,1,2
		;
}

void LedActor::setBlink(float period, float dutyCycle)
{
	if (period<0.05) period=0.;
	if (dutyCycle<0.) dutyCycle=0.;
	if (dutyCycle>1.) dutyCycle=1.;
	if (isDebug())
		cerr << "blink " << period << " at " << dutyCycle <<endl;
	zBlinkPeriod = period;
	zBlinkDutyCycle = dutyCycle;
}

int LedActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetBrightness"))
	{
		ifF( z, setBrightness(z) );
		return Uncatch();
	}

	if (CommandIs("SetHue"))
	{
		ifF( z, setHue(z) );
		return Uncatch();
	}

	if (CommandIs("SetBlink"))
	{
		ifFF( z1, z2, setBlink(z1,z2) );
		return Uncatch();
	}

	return VActor::receiveMessage(Message);
}
#endif
