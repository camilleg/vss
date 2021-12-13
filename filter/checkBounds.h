#pragma once

static	inline	int	CheckFrequency(float f)	{ return (f >= 0. && f <= 20.0e3); }
static	inline	int	CheckResonance(float Q)	{ return (Q >= 0. && Q <= 100.); }
static	inline	int	CheckGain(float A)	{ return (A >= -10. && A <= 10.); }
