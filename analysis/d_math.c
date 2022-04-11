/* Copyright (c) 1997 The Regents of the University of California.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  mathematical functions and other transfer functions
*/

#include "m_pd.h"
#include <math.h>

/* ------------------------- clip~ -------------------------- */
static t_class *clip_class;

typedef struct _clip
{
    t_object x_obj;
    t_sample x_lo;
    t_sample x_hi;
} t_clip;

static void *clip_new(t_floatarg lo, t_floatarg hi)
{
    t_clip *x = (t_clip *)pd_new(clip_class);
    x->x_lo = lo;
    x->x_hi = hi;
    outlet_new(&x->x_obj, gensym("signal"));
    floatinlet_new(&x->x_obj, &x->x_lo);
    floatinlet_new(&x->x_obj, &x->x_hi);
    return (x);
}

static t_int *clip_perform(t_int *w)
{
    t_clip *x = (t_clip *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    while (n--)
    {
    	float f = *in++;
    	if (f < x->x_lo) f = x->x_lo;
    	if (f > x->x_hi) f = x->x_hi;
    	*out++ = f;
    }
    return (w+5);
}

static void clip_dsp(t_clip *x, t_signal **sp)
{
    dsp_add(clip_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void clip_setup(void)
{
    clip_class = class_new(gensym("clip~"), (t_newmethod)clip_new, 0,
    	sizeof(t_clip), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(clip_class, nullfn, gensym("signal"), 0);
    class_addmethod(clip_class, clip_dsp, gensym("dsp"), 0);
}

/* sigrsqrt - reciprocal square root good to 8 mantissa bits  */

#define DUMTAB1SIZE 256
#define DUMTAB2SIZE 1024

static float rsqrt_exptab[DUMTAB1SIZE], rsqrt_mantissatab[DUMTAB2SIZE];

static void init_rsqrt(void)
{
    int i;
    for (i = 0; i < DUMTAB1SIZE; i++)
    {
	float f;
	long l = (i ? (i == DUMTAB1SIZE-1 ? DUMTAB1SIZE-2 : i) : 1)<< 23;
	*(long *)(&f) = l; // Casting from float * to signed long * is not portable due to different binary data representations on different platforms.
	rsqrt_exptab[i] = 1./sqrt(f);	
    }
    for (i = 0; i < DUMTAB2SIZE; i++)
    {
	float f = 1 + (1./DUMTAB2SIZE) * i;
	rsqrt_mantissatab[i] = 1./sqrt(f);	
    }
}

float qrsqrt(float f)
{
    long l = *(long *)(&f); // Casting from float * to signed long * is not portable due to different binary data representations on different platforms.
    if (f < 0) return (0);
    else return (rsqrt_exptab[(l >> 23) & 0xff] *
	    rsqrt_mantissatab[(l >> 13) & 0x3ff]);
}

float sgicompatible_qsqrt(float f)
{
    long l = *(long *)(&f);
    if (f < 0) return (0);
    else return (f * rsqrt_exptab[(l >> 23) & 0xff] *
	    rsqrt_mantissatab[(l >> 13) & 0x3ff]);
}

typedef struct sigrsqrt
{
    t_object x_obj;
} t_sigrsqrt;

static t_class *sigrsqrt_class;

static void *sigrsqrt_new()
{
    t_sigrsqrt *x = (t_sigrsqrt *)pd_new(sigrsqrt_class);
    outlet_new(&x->x_obj, gensym("signal"));
    return (x);
}

static t_int *sigrsqrt_perform(t_int *w)
{
    float *in = *(t_float **)(w+1), *out = *(t_float **)(w+2);
    t_int n = *(t_int *)(w+3);
    while (n--)
    {	
	float f = *in;
	long l = *(long *)(in++); // Casting from float * to signed long * is not portable due to different binary data representations on different platforms.
	if (f < 0) *out++ = 0;
	else *out++ =
	    (rsqrt_exptab[(l >> 23) & 0xff] *
	    	rsqrt_mantissatab[(l >> 13) & 0x3ff]);
    }
    return (w + 4);
}

static void sigrsqrt_dsp(t_clip *x, t_signal **sp)
{
    dsp_add(sigrsqrt_perform, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void sigrsqrt_setup(void)
{
    init_rsqrt();
    sigrsqrt_class = class_new(gensym("rsqrt~"), sigrsqrt_new, 0,
    	sizeof(t_sigrsqrt), 0, 0);
    class_addmethod(sigrsqrt_class, nullfn, gensym("signal"), 0);
    class_addmethod(sigrsqrt_class, sigrsqrt_dsp, gensym("dsp"), 0);
}


/* sigsqrt -  square root good to 8 mantissa bits  */

typedef struct sigsqrt
{
    t_object x_obj;
} t_sigsqrt;

static t_class *sigsqrt_class;

static void *sigsqrt_new()
{
    t_sigsqrt *x = (t_sigsqrt *)pd_new(sigsqrt_class);
    outlet_new(&x->x_obj, gensym("signal"));
    return (x);
}

t_int *sigsqrt_perform(t_int *w)    /* not static; also used in d_fft.c */
{
    float *in = *(t_float **)(w+1), *out = *(t_float **)(w+2);
    t_int n = *(t_int *)(w+3);
    while (n--)
    {	
	float f = *in;
	long l = *(long *)(in++);
	if (f < 0) *out++ = 0;
	else *out++ = f *
	    (rsqrt_exptab[(l >> 23) & 0xff] *
	    rsqrt_mantissatab[(l >> 13) & 0x3ff]);
    }
    return (w + 4);
}

static void sigsqrt_dsp(t_clip *x, t_signal **sp)
{
    dsp_add(sigsqrt_perform, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void sigsqrt_setup(void)
{
    sigsqrt_class = class_new(gensym("sqrt~"), sigsqrt_new, 0,
    	sizeof(t_sigsqrt), 0, 0);
    class_addmethod(sigsqrt_class, nullfn, gensym("signal"), 0);
    class_addmethod(sigsqrt_class, sigsqrt_dsp, gensym("dsp"), 0);
}

/* ------------------------------ wrap~ -------------------------- */

typedef struct wrap
{
    t_object x_obj;
} t_sigwrap;

t_class *sigwrap_class;

static void *sigwrap_new(void)
{
    t_sigwrap *x = (t_sigwrap *)pd_new(sigwrap_class);
    outlet_new(&x->x_obj, gensym("signal"));
    return (x);
}

static t_int *sigwrap_perform(t_int *w)
{
    float *in = *(t_float **)(w+1), *out = *(t_float **)(w+2);
    t_int n = *(t_int *)(w+3);
    while (n--)
    {	
	float f = *in++;
	int k = f;
	if (f > 0) *out++ = f-k;
	else *out++ = f - (k-1);
    }
    return (w + 4);
}

static void sigwrap_dsp(t_clip *x, t_signal **sp)
{
    dsp_add(sigwrap_perform, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void sigwrap_setup(void)
{
    sigwrap_class = class_new(gensym("wrap~"), sigwrap_new, 0,
    	sizeof(t_sigwrap), 0, 0);
    class_addmethod(sigwrap_class, nullfn, gensym("signal"), 0);
    class_addmethod(sigwrap_class, sigwrap_dsp, gensym("dsp"), 0);
}

/* ------------------------ global setup routine ------------------------- */

void d_math_setup(void)
{
    clip_setup();
    sigrsqrt_setup();
    sigsqrt_setup();
    sigwrap_setup();
}

