#include "loopActor.h"


// Constructor.
loopActor::loopActor() :
	VActor(),
	x(0.),
	n(0),
	xStart(0.),
	xLimit(0.),
	nLimit(0),
	tLimit(0.),
	fDisableXLimit(1),
	fDisableNLimit(1),
	fDisableTLimit(1),
	tOffset(0.),
	tEnd(0.),
	tNext(0.),
	dx(0.), dt(1.),
	ddx(0.), ddt(0.),
	ddxDur(0.), ddtDur(0.),
	numLoops(0),
	fSwing(0),
	xIrreg(0.),
	tIrreg(0.),
	zUserFloat(0.)
{
	setTypeName("LoopActor");
	tOffset = currentTime();
	*szMG = '\0';
}

void loopActor::act()
{
	VActor::act();

	if (numLoops == 0 || !*szMG || loopTime() < tNext)
		// Stopped, or no message group to talk to, or waiting for next event.
		return;

	// Send a message to the message group.

	{
	char sz[256];
	sprintf(sz, "SendData %s [%f %f]", szMG, x, zUserFloat);
	actorMessageHandler(sz);
	}

	// Update state.

	if (ddtDur > 0.)
		{
		ddtDur -= dt; // this is only an approximation of last time's dtEffective
		dt += ddt;
		}
	float dtEffective = dt /* with poisson randomness from tIrreg */;
	if (ddxDur > 0.)
		{
		ddxDur -= dtEffective;
		dx += ddx;
		}
	// This whole ddFoo thing is a lame and inaccurate attempt to
	// avoid more state variables, and needs rewriting.

	// (When xIrreg==0, x += dx.  When xIrreg==1,
	// x += a random number chosen uniformly between 0 and 2*dx.)
	x += dx * (1. + xIrreg * (2*drand48() - 1.));
	tNext += dtEffective;
	n++;

	if ((!fDisableXLimit && ((dx>0 && x>xLimit) || (dx<0 && x<xLimit))) ||
		(!fDisableTLimit && loopTime() > tEnd) ||
		(!fDisableNLimit && n > nLimit))
		{
		// Loop should end.
		if (fSwing)
			{
			float xT = xStart; xStart = xLimit; xLimit = xT;
			dx = -dx;
			}
		// setNumLoops() reinitializes a few variables for us,
		// instead of simply decrementing numLoops.
		if (numLoops>0)
			setNumLoops(numLoops-1);
		else if (numLoops<0)
			setNumLoops(-1);
		}

	MaybeDump();
}

// receiveMessage

int loopActor::receiveMessage(const char* Message)
{
	int ret = receiveMessageCore(strdup(Message)); // memory leak
	MaybeDump();
	return ret;
}

int loopActor::receiveMessageCore(char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("Active"))
		{ 
		ifD(d, setActive(d) ); 
		return Uncatch(); 
		}

	if (CommandIs("SetTimeStep"))
		{
		ifFF( timeIncr, t, setTimeStep(timeIncr, t) );
		ifF( timeIncr, setTimeStep(timeIncr) );
		return Uncatch();
		}

	if (CommandIs("SetDataStep"))
		{
		ifFF( dataIncr, t, setDataStep(dataIncr, t) );
		ifF( dataIncr, setDataStep(dataIncr) );
		return Uncatch();
		}

	if (CommandIs("SetDataStart"))
		{
		ifF( f, setDataStart(f) );
		return Uncatch();
		}

	if (CommandIs("SetDataLimit"))
		{
		ifF( z, setDataLimit(z) );
		return Uncatch();
		}

	if (CommandIs("SetNoDataLimit"))
		{
		ifNil( setNoDataLimit() );
		}

	if (CommandIs("SetTimeLimit"))
		{
		ifF( z, setTimeLimit(z) );
		return Uncatch();
		}

	if (CommandIs("SetStepLimit"))
		{
		ifD( w, setStepLimit(w) );
		return Uncatch();
		}

	if (CommandIs("ComputeDataLimit"))
		{ ifNil(   computeDataLimit() ); }
	if (CommandIs("ComputeDataStep"))
		{ ifNil(   computeDataStep() ); }
	if (CommandIs("ComputeTimeLimit"))
		{ ifNil(   computeTimeLimit() ); }
	if (CommandIs("ComputeTimeStep"))
		{ ifNil(   computeTimeStep() ); }
	if (CommandIs("ComputeStepLimit"))
		{ ifNil(   computeStepLimit() ); }

	if (CommandIs("SetMessageGroup"))
		{
		ifS( name, setMessageGroup(name) );
		return Uncatch();
		}

	if (CommandIs("SetNumLoops"))
		{
		ifD( w, setNumLoops(w) );
		return Uncatch();
		}

	if (CommandIs("SetSwing"))
		{
		ifD( f, setSwing(f) );
		return Uncatch();
		}

	if (CommandIs("SetDebug"))
		{
		fprintf(stderr, "vss warning: use Debug not SetDebug with loopactor.\n");
		ifD( f, VActor::setDebug(f) );
		return Uncatch();
		}

	if (CommandIs("SetUserFloat"))
		{
		ifF( z, zUserFloat=z );
		return Uncatch();
		}

	return VActor::receiveMessage(Message);
}

void loopActor::setActive(int f)
{
	//;; printf("How does loopActor::setActive work?\n");;;;
	//;; tOffset = currentTime(); //translate real time into loopActor time
	VActor::setActive(f);
}

void loopActor::setDataStep(float z, float t)
{
	if (t <= 0)
		{
		ddxDur = 0;
		dx = z;
		}
	else
		{
		ddxDur = t;
		ddx = (z - dx) / ddxDur; // Assumes ddt is zero!
		}
}

void loopActor::setTimeStep(float z, float t)
{
	dt = z;
	if (t <= 0)
		{
		ddtDur = 0;
		dt = z;
		}
	else
		{
		ddtDur = t;
		ddt = (z - dt) / ddtDur; // Assumes ddt is zero, and therefore is incorrect!
		}
}

ostream& loopActor::dump(ostream &os, int tabs)
{
	VActor::dump(os, tabs);
	indent(os, tabs)<<"  Data:    "<<x<<" from "<<xStart;
		if (!fDisableXLimit)
			os<<" to "<<xLimit;
		os<<" step "<<dx;
		if (xIrreg > 0)
			os<<" +- "<< xIrreg*100. <<"%";
		os<<endl;
	indent(os, tabs)<<"  Time:    "<<loopTime();
		if (!fDisableTLimit)
			os<<" from "<<tEnd-tLimit<<" to " <<tEnd;
		os<<" step "<<dt;
		if (tIrreg > 0)
			os<<" with irregularity " << tIrreg;
		os<<endl;
	indent(os, tabs)<<"  Steps:   "<<n<< " from 0 ";
		if (!fDisableNLimit)
			os<<"to "<<nLimit;
		os<<endl;
	indent(os, tabs)<<"End check: ";
		if (!fDisableXLimit) os<<"data ";
		if (!fDisableTLimit) os<<"time ";
		if (!fDisableNLimit) os<<"steps ";
		if (fDisableXLimit && fDisableTLimit && fDisableNLimit) os<<"(none)";
		os << endl;
	indent(os, tabs)<< numLoops;
		if (fSwing) os << " swinging";
		os << " loops left" <<endl;
	indent(os, tabs)<<"MG handle: "<<szMG <<endl;
	return os;
}

void loopActor::computeDataLimit(void)
{
	if (fDisableTLimit && fDisableNLimit)
		return;
	float xLimitT=0., xLimitN=0.;
	float durT=0., durN=0.;
	if (!fDisableTLimit)
		{
		float tLeft = tEnd - loopTime();
		xLimitT = x + dx * tLeft/dt;
		durT = tLeft;
		}
	if (!fDisableNLimit)
		{
		int nLeft = nLimit - n;
		xLimitN = x + dx * nLeft / 1;
		durN = dt * nLeft * 1;
		}
	if (!fDisableTLimit && !fDisableNLimit)
		setDataLimit(durN < durT ? xLimitN : xLimitT);
	else if (!fDisableTLimit)
		setDataLimit(xLimitT);
	else
		setDataLimit(xLimitN);
}

void loopActor::computeDataStep(void)
{
	if (fDisableTLimit && fDisableNLimit)
		return;
	float xStepT=0., xStepN=0.;
	float durT=0., durN=0.;
	if (!fDisableTLimit)
		{
		float tLeft = tEnd - loopTime();
		xStepT = (xLimit - x) / dt;
		durT = tLeft;
		}
	if (!fDisableNLimit)
		{
		int nLeft = nLimit - n;
		xStepN = (xLimit - x) / nLeft;
		durN = dt * nLeft / 1;
		}
	if (!fDisableTLimit && !fDisableNLimit)
		setDataStep(durN < durT ? xStepN : xStepT);
	else if (!fDisableTLimit)
		setDataStep(xStepT);
	else
		setDataStep(xStepN);
}

void loopActor::computeTimeLimit(void)
{
	int f=0;
	float tLimitT=0;
	if (!fDisableNLimit)
		{
		int nLeft = nLimit - n;
		tLimitT = loopTime() + dt * nLeft/1;
		f=1;
		}
	if (!fDisableXLimit)
		{
		float xLeft = xLimit-x;
		float tLimitT2 = loopTime() + dt * xLeft / dx;
		if (!f || tLimitT2 < tLimitT)
			tLimitT = tLimitT2;
		f=1;
		}
	if (f)
		setTimeLimit(tLimitT);
}

void loopActor::computeTimeStep(void)
{
	int f=0;
	int nLeft=0;
	if (!fDisableNLimit)
		{
		nLeft = nLimit - n;
		f=1;
		}
	if (!fDisableXLimit)
		{
		int nxLeft = (int)((xLimit - x) / dx);
		if (!f || nxLeft < nLeft)
			nLeft = nxLeft;
		f=1;
		}
	if (f)
		setTimeStep((tLimit - loopTime()) / nLeft);
}

void loopActor::computeStepLimit(void)
{
	int f=0;
	int nLimitT=0;
	if (!fDisableTLimit)
		{
		float tLeft = tEnd - loopTime();
		nLimitT = n + 1 * int(tLeft/dt);
		f=1;
		}
	if (!fDisableXLimit)
		{
		float xLeft = xLimit - x;
		float nLimitT2 = n + 1 * xLeft/dx;
		if (!f || nLimitT2 < nLimitT)
			nLimitT = int(nLimitT2);
		f=1;
		}
	if (f)
		setStepLimit(nLimitT);
}
