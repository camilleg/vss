#ifndef _CHUA_H_
#define _CHUA_H_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

#define	CHUA_DIM	3
#define	TAUMAX		1000

//===========================================================================
//		chuaAlg 
//
//	class chuaAlg is a test algorithm with sample generation
//	code copied from the old classFMnote.
//
//	Note that some members are defined in the class definition 
//	owing to the lameness of our compiler.
//
class chuaAlg : public VAlgorithm
{
private:
//	synthesis parameters

	float	potentiometer;
	double	Q;
	int	cursamp;
	int	tau;

	double	R;
	double	R0;
	double	C1;
	double	C2;
	double	L;
	double	BPH1;
	double	BPH2;
	double	BP1;
	double	BP2;
	double	M0;
	double	M1;
	double	M2;
	double	M3;

	double	tstep;
	float	spectStep;		/* will be converted to Hz */
	int	integStep;		/* number of 1/SR steps per integration step */

	double	vector_pos[CHUA_DIM];

/*
	vars used in itegration
*/
	double	g, REGbp1, REGbp2, REGbph1, REGbph2, REGbpdiff;
	double	g0BPH1, g0BPH2, AA, BB, MA, MB, rho, p5m;


public:
//	access members
	double		getR(void)			{ return ( R ); }
	double		getR0(void)			{ return ( R0 ); }
	double		getC1(void)			{ return ( C1 ); }
	double		getC2(void)			{ return ( C2 ); }
	double		getL(void)			{ return ( L ); }
	double		getBPH1(void)			{ return ( BPH1 ); }
	double		getBPH2(void)			{ return ( BPH2 ); }
	double		getBP1(void)			{ return ( BP1 ); }
	double		getBP2(void)			{ return ( BP2 ); }
	double		getM0(void)			{ return ( M0 ); }
	double		getM1(void)			{ return ( M1 ); }
	double		getM2(void)			{ return ( M2 ); }
	double		getM3(void)			{ return ( M3 ); }
	void		getVector(double * putItHere)
				{ memcpy ( putItHere, vector_pos, CHUA_DIM * sizeof(double)); }

//	parameter update members
	void	setFundamental(float, float);

	void		setR(float newR)			{ R = newR; }
	void		setR0(float newR0)			{ R0 = newR0; }
	void		setC1(float newC1)			{ C1 = newC1; }
	void		setC2(float newC2)			{ C2 = newC2; }
	void		setL(float newL)			{ L = newL; }
	void		setBPH1(float newBPH1)			{ BPH1 = newBPH1; }
	void		setBPH2(float newBPH2)			{ BPH2 = newBPH2; }
	void		setBP1(float newBP1)			{ BP1 = newBP1; }
	void		setBP2(float newBP2)			{ BP2 = newBP2; }
	void		setM0(float newM0)			{ M0 = newM0; }
	void		setM1(float newM1)			{ M1 = newM1; }
	void		setM2(float newM2)			{ M2 = newM2; }
	void		setM3(float newM3)			{ M3 = newM3; }
	void		setVector(double * newVector)
				{ memcpy ( vector_pos, newVector, CHUA_DIM * sizeof(double)); }

//	initialization type stuff
	void		resetChuaNote(void);
	void		resetControlForce(void);
	void		resetVector(void);
	void		resetState(void);

/*
	This is the Runge-Kutta integration stuff. It is living in the class definition
	so that it will be forced to compile in-place, for efficiency.
*/
	void		difeq(double * xx, double * xdot)
	{
	/*
		Define nonlinear resistor function g(v1)
	*/
		g = (xx[0] < REGbph1) ?
				M2 * (xx[0] - REGbph1) + g0BPH1 :
	    (xx[0] > REGbph2) ?
		M3 * (xx[0] - REGbph2) + g0BPH2 :
		MB * xx[0] + p5m * (fabs(xx[0] + REGbp1) -
				 fabs(xx[0] - REGbp2) + REGbpdiff);
	/*
		Vectorfield
	*/
		xdot[0] = AA * ( xx[1] - xx[0] - g);
		xdot[1] = xx[0] - xx[1] + xx[2];
		xdot[2] = - BB * (xx[1] + rho * xx[2]);

	}; /* end of difeq() */

	void		rk4(void)
	{
		int l;
		double k1[CHUA_DIM];
		double k2[CHUA_DIM];
		double k3[CHUA_DIM];
		double xdot[CHUA_DIM];
		double new_vp[CHUA_DIM];

		difeq(vector_pos, xdot);
		for(l=0;l<CHUA_DIM;l++)
			new_vp[l] = vector_pos[l] + (k1[l]=tstep*xdot[l]*0.5);

		difeq(new_vp, xdot);
		for(l=0;l<CHUA_DIM;l++)
			new_vp[l] = vector_pos[l] + (k2[l]=tstep*xdot[l])*0.5;

		difeq(new_vp, xdot);
		for(l=0;l<CHUA_DIM;l++)
			new_vp[l] = vector_pos[l] + (k3[l]=tstep*xdot[l]);

		difeq(new_vp, xdot);
		for(l=0;l<CHUA_DIM;l++)
			vector_pos[l] += (k1[l]+k2[l]+k3[l]+tstep*xdot[l]*0.5)/3.0;

//		printf("R %f, R0 %f, C1 %e, C2 %e, L %f, BP1 %f, BP2 %f, M0 %e, M1 %e, M2 %e, M3 %e, X %f, Y %f, Z %f\n ",
//				R, R0, C1, C2, L, BP1, BP2, M0, M1, M2, M3, 
//				vector_pos[0], vector_pos[1], vector_pos[2]);
	}; /* end of rk4() */

//	sample generation
	void	generateSamples(int);

//	construction/destruction
		chuaAlg(void);
		~chuaAlg();

};	// end of class chuaAlg

//===========================================================================
//		chuaHand 
//
//	class chuaHand is a handler class for chuaAlg.
//
class chuaHand : public VHandler
{
//  Algorithm access:
//  Define a version of getAlg() that returns a pointer to chuaAlg.
protected:
    chuaAlg * getAlg(void) { return (chuaAlg *) VHandler::getAlg(); }

//	modulating parameters of chuaAlg
private:
	float	R;
	float	R0;
	float	C1;
	float	C2;
	float	L;
	float	BPH1;
	float	BPH2;
	float	BP1;
	float	BP2;
	float	M0;
	float	M1;
	float	M2;
	float	M3;

	enum { isetR, isetR0, isetC1, isetC2, isetL, isetBPH1, isetBPH2, isetBP1, isetBP2, isetM0, isetM1, isetM2, isetM3 };

//	parameter access
public:
#ifdef UNUSED
	double		getR(void)			{ return getAlg()->getR() ; }
	double		getR0(void)			{ return getAlg()->getR0() ; }
	double		getC1(void)			{ return getAlg()->getC1() ; }
	double		getC2(void)			{ return getAlg()->getC2() ; }
	double		getL(void)			{ return getAlg()->getL() ; }
	double		getBPH1(void)		{ return getAlg()->getBPH1() ; }
	double		getBPH2(void)		{ return getAlg()->getBPH2() ; }
	double		getBP1(void)		{ return getAlg()->getBP1() ; }
	double		getBP2(void)		{ return getAlg()->getBP2() ; }
	double		getM0(void)			{ return getAlg()->getM0() ; }
	double		getM1(void)			{ return getAlg()->getM1() ; }
	double		getM2(void)			{ return getAlg()->getM2() ; }
	double		getM3(void)			{ return getAlg()->getM3() ; }
#endif

//	parameter modulation
	void		resetChuaState(void);

	void SetAttribute(IParam iParam, float z);

	void setR(float z, float t = timeDefault)
		{ modulate(isetR, R, z, AdjustTime(t)); }
	void setR0(float z, float t = timeDefault)
		{ modulate(isetR0, R0, z, AdjustTime(t)); }
	void setC1(float z, float t = timeDefault)
		{ modulate(isetC1, C1, z, AdjustTime(t)); }
	void setC2(float z, float t = timeDefault)
		{ modulate(isetC2, C2, z, AdjustTime(t)); }
	void setL(float z, float t = timeDefault)
		{ modulate(isetL, L, z, AdjustTime(t)); }
	void setBPH1(float z, float t = timeDefault)
		{ modulate(isetBPH1, BPH1, z, AdjustTime(t)); }
	void setBPH2(float z, float t = timeDefault)
		{ modulate(isetBPH2, BPH2, z, AdjustTime(t)); }
	void setBP1(float z, float t = timeDefault)
		{ modulate(isetBP1, BP1, z, AdjustTime(t)); }
	void setBP2(float z, float t = timeDefault)
		{ modulate(isetBP2, BP2, z, AdjustTime(t)); }
	void setM0(float z, float t = timeDefault)
		{ modulate(isetM0, M0, z, AdjustTime(t)); }
	void setM1(float z, float t = timeDefault)
		{ modulate(isetM1, M1, z, AdjustTime(t)); }
	void setM2(float z, float t = timeDefault)
		{ modulate(isetM2, M2, z, AdjustTime(t)); }
	void setM3(float z, float t = timeDefault)
		{ modulate(isetM3, M3, z, AdjustTime(t)); }

//	chua parameter modification

//	damp amplitude changes
	float	dampingTime(void)	{ return 0.03; }

//	construction
		chuaHand(chuaAlg * alg = new chuaAlg);
		
//	destruction
virtual		~chuaHand() {}

	int receiveMessage(const char * Message);

};	// end of class chuaHand

//===========================================================================
//		chuaActor
//
//	class chuaActor is a generator actor class for chuaAlg
//
class chuaActor : public VGeneratorActor
{
private:
	static int initialized;		// We need to know (and keep track of)
					// whether the CHANT libraries have 
					// been initialized
//	construction/destruction
public:
	chuaActor(void);
virtual	~chuaActor() {}

virtual VHandler * newHandler(void) { return new chuaHand(); }

virtual	void 	sendDefaults(VHandler *);
virtual int	receiveMessage(const char * Message);

//	parameter setting members

	void		resetAllChuaState(void);

	void		setR(float z);
	void		setAllR(float z, float t = 0.);

	void		setR0(float z);
	void		setAllR0(float z, float t = 0.);

	void		setC1(float z);
	void		setAllC1(float z, float t = 0.);

	void		setC2(float z);
	void		setAllC2(float z, float t = 0.);

	void		setL(float z);
	void		setAllL(float z, float t = 0.);

	void		setBPH1(float z);
	void		setAllBPH1(float z, float t = 0.);

	void		setBPH2(float z);
	void		setAllBPH2(float z, float t = 0.);

	void		setBP1(float z);
	void		setAllBP1(float z, float t = 0.);

	void		setBP2(float z);
	void		setAllBP2(float z, float t = 0.);

	void		setM0(float z);
	void		setAllM0(float z, float t = 0.);

	void		setM1(float z);
	void		setAllM1(float z, float t = 0.);

	void		setM2(float z);
	void		setAllM2(float z, float t = 0.);

	void		setM3(float z);
	void		setAllM3(float z, float t = 0.);

//	default parameters
protected:

	float	defaultR ;
	float	defaultR0 ;
	float	defaultC1 ;
	float	defaultC2 ;
	float	defaultL ;
	float	defaultBPH1 ;
	float	defaultBPH2 ;
	float	defaultBP1 ;
	float	defaultBP2 ;
	float	defaultM0 ;
	float	defaultM1 ;
	float	defaultM2 ;
	float	defaultM3 ;

};	// end of class chuaActor

//===========================================================================
//	BOUNDS CHECKING IS VITAL TO OUR SURVIVAL!!!!!!!!!!!!!!!!!!!
//
//	Find reasonable bounds and enforce them.
//

#endif // ndef _CHUA_H_
