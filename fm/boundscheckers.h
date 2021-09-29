#pragma once

static	inline	int	CheckFreq(float f) 	{ return f >= 0. && f < 20000.; }
static	inline	int	CheckCMratio(float f) 	{ return f > 1.0e-6 && f < 1.0e6; }
static	inline	int	CheckIndex(float f)	{ return f >= 0. && f < 1000.; }
static	inline	int	CheckFeedback(float f)	{ return f >= -1. && f <= 1.; }
