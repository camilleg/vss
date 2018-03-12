/* Copyright (c) 1998 The Regents of the University of California.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"

/* ------------------------ fft~ and ifft~ -------------------------------- */
static t_class *sigfft_class, *sigifft_class;

typedef struct fft
{
    t_object x_obj;
} t_sigfft;

static void *sigfft_new(void)
{
    t_sigfft *x = (t_sigfft *)pd_new(sigfft_class);
    outlet_new(&x->x_obj, gensym("signal"));
    outlet_new(&x->x_obj, gensym("signal"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    return (x);
}

static void *sigifft_new(void)
{
    t_sigfft *x = (t_sigfft *)pd_new(sigifft_class);
    outlet_new(&x->x_obj, gensym("signal"));
    outlet_new(&x->x_obj, gensym("signal"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    return (x);
}

static t_int *sigfft_swap(t_int *w)
{
    float *in1 = (t_float *)(w[1]);
    float *in2 = (t_float *)(w[2]);
    int n = w[3];
    for (;n--; in1++, in2++)
    {	
	float f = *in1;
	*in1 = *in2;
	*in2 = f;
    }
    return (w+4);    
}

static t_int *sigfft_perform(t_int *w)
{
    float *in1 = (t_float *)(w[1]);
    float *in2 = (t_float *)(w[2]);
    int n = w[3];
    mayer_fft(n, in1, in2);
    return (w+4);
}

/*
static t_int *sigifft_perform(t_int *w)
{
    float *in1 = (t_float *)(w[1]);
    float *in2 = (t_float *)(w[2]);
    int n = w[3];
    mayer_ifft(n, in1, in2);
    return (w+4);
}
*/

static void sigfft_dspx(t_sigfft *x, t_signal **sp, t_int *(*f)(t_int *w))
{
    int n = sp[0]->s_n;
    float *in1 = sp[0]->s_vec;
    float *in2 = sp[1]->s_vec;
    float *out1 = sp[2]->s_vec;
    float *out2 = sp[3]->s_vec;
    if (out1 == in2 && out2 == in1)
    	dsp_add(sigfft_swap, 3, out1, out2, n);
    else if (out1 == in2)
    {
    	dsp_add(copy_perform, 3, in2, out2, n);
    	dsp_add(copy_perform, 3, in1, out1, n);
    }
    else
    {
    	if (out1 != in1) dsp_add(copy_perform, 3, in1, out1, n);
    	if (out2 != in2) dsp_add(copy_perform, 3, in2, out2, n);
    }
    dsp_add(f, 3, sp[2]->s_vec, sp[3]->s_vec, n);
}

static void sigfft_dsp(t_sigfft *x, t_signal **sp)
{
    sigfft_dspx(x, sp, sigfft_perform);
}

static void sigifft_dsp(t_sigfft *x, t_signal **sp)
{
    sigfft_dspx(x, sp, sigfft_perform);
}

static void sigfft_setup(void)
{
    sigfft_class = class_new(gensym("fft~"), sigfft_new, 0,
    	sizeof(t_sigfft), 0, 0);
    class_addmethod(sigfft_class, nullfn, gensym("signal"), 0);
    class_addmethod(sigfft_class, sigfft_dsp, gensym("dsp"), 0);

    sigifft_class = class_new(gensym("ifft~"), sigifft_new, 0,
    	sizeof(t_sigfft), 0, 0);
    class_addmethod(sigifft_class, nullfn, gensym("signal"), 0);
    class_addmethod(sigifft_class, sigifft_dsp, gensym("dsp"), 0);
}

/* ----------------------- rfft~ -------------------------------- */

static t_class *sigrfft_class;

typedef struct rfft
{
    t_object x_obj;
} t_sigrfft;

static void *sigrfft_new(void)
{
    t_sigrfft *x = (t_sigrfft *)pd_new(sigrfft_class);
    outlet_new(&x->x_obj, gensym("signal"));
    outlet_new(&x->x_obj, gensym("signal"));
    return (x);
}

static t_int *sigrfft_flip(t_int *w)
{
    float *in = (t_float *)(w[1]);
    float *out = (t_float *)(w[2]);
    int n = w[3];
    while (n--) *(--out) = *in++;
    *(--out) = 0;	    	    /* to hell with it */
    return (w+4);
}

static t_int *sigrfft_perform(t_int *w)
{
    float *in = (t_float *)(w[1]);
    int n = w[2];
    mayer_realfft(n, in);
    return (w+3);
}

static void sigrfft_dsp(t_sigrfft *x, t_signal **sp)
{
    int n = sp[0]->s_n, n2 = (n>>1);
    float *in1 = sp[0]->s_vec;
    float *out1 = sp[1]->s_vec;
    float *out2 = sp[2]->s_vec;
    if (n < 4)
    {
    	error("fft: minimum 4 points");
    	return;
    }
    if (in1 == out2)	/* this probably never happens */
    {
    	dsp_add(sigrfft_perform, 2, out2, n);
    	dsp_add(copy_perform, 3, out2, out1, n2);
    	dsp_add(sigrfft_flip, 3, out2 + (n2+1), out2 + n2, n2-1);
    }
    else
    {
    	if (in1 != out1) dsp_add(copy_perform, 3, in1, out1, n);
    	dsp_add(sigrfft_perform, 2, out1, n);
    	dsp_add(sigrfft_flip, 3, out1 + (n2+1), out2 + n2, n2-1);
    }
    dsp_add(zero_perform, 2, out1 + n2, n2);
    dsp_add(zero_perform, 2, out2 + n2, n2);
}

static void sigrfft_setup(void)
{
    sigrfft_class = class_new(gensym("rfft~"), sigrfft_new, 0,
    	sizeof(t_sigrfft), 0, 0);
    class_addmethod(sigrfft_class, nullfn, gensym("signal"), 0);
    class_addmethod(sigrfft_class, sigrfft_dsp, gensym("dsp"), 0);
}

/* ----------------------- rifft~ -------------------------------- */

static t_class *sigrifft_class;

typedef struct rifft
{
    t_object x_obj;
} t_sigrifft;

static void *sigrifft_new(void)
{
    t_sigrifft *x = (t_sigrifft *)pd_new(sigrifft_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, gensym("signal"));
    return (x);
}

static t_int *sigrifft_perform(t_int *w)
{
    float *in = (t_float *)(w[1]);
    int n = w[2];
    mayer_realifft(n, in);
    return (w+3);
}

static void sigrifft_dsp(t_sigrifft *x, t_signal **sp)
{
    int n = sp[0]->s_n, n2 = (n>>1);
    float *in1 = sp[0]->s_vec;
    float *in2 = sp[1]->s_vec;
    float *out1 = sp[2]->s_vec;
    if (n < 4)
    {
    	error("fft: minimum 4 points");
    	return;
    }
    if (in2 == out1)
    {
    	dsp_add(sigrfft_flip, 3, out1+1, out1 + n, (n2-1));
    	dsp_add(copy_perform, 3, in1, out1, n2);
    }
    else
    {
    	if (in1 != out1) dsp_add(copy_perform, 3, in1, out1, n2);
    	dsp_add(sigrfft_flip, 3, in2+1, out1 + n, n2-1);
    }
    dsp_add(sigrifft_perform, 2, out1, n);
}

static void sigrifft_setup(void)
{
    sigrifft_class = class_new(gensym("rifft~"), sigrifft_new, 0,
    	sizeof(t_sigrifft), 0, 0);
    class_addmethod(sigrifft_class, nullfn, gensym("signal"), 0);
    class_addmethod(sigrifft_class, sigrifft_dsp, gensym("dsp"), 0);
}

/* ----------------------- framp~ -------------------------------- */

static t_class *sigframp_class;

typedef struct framp
{
    t_object x_obj;
} t_sigframp;

static void *sigframp_new(void)
{
    t_sigframp *x = (t_sigframp *)pd_new(sigframp_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, gensym("signal"));
    outlet_new(&x->x_obj, gensym("signal"));
    return (x);
}

static t_int *sigframp_perform(t_int *w)
{
    float *inreal = (t_float *)(w[1]);
    float *inimag = (t_float *)(w[2]);
    float *outfreq = (t_float *)(w[3]);
    float *outamp = (t_float *)(w[4]);
    float lastreal = 0, currentreal = inreal[0], nextreal = inreal[1];
    float lastimag = 0, currentimag = inimag[0], nextimag = inimag[1];
    int n = w[5];
    int m = n + 1;
    float fbin = 1, oneovern2 = 1.f/((float)n * (float)n);
    
    inreal += 2;
    inimag += 2;
    *outamp++ = *outfreq++ = 0;
    n -= 2;
    while (n--)
    {
    	float re, im, pow, freq;
    	lastreal = currentreal;
    	currentreal = nextreal;
    	nextreal = *inreal++;
    	lastimag = currentimag;
    	currentimag = nextimag;
    	nextimag = *inimag++;
    	re = currentreal - 0.5f * (lastreal + nextreal);
    	im = currentimag - 0.5f * (lastimag + nextimag);
    	pow = re * re + im * im;
    	if (pow > 1e-19)
    	{
    	    float detune = ((lastreal - nextreal) * re +
    	    	    (lastimag - nextimag) * im) / (2.0f * pow);
    	    {
    	    	 /* if (fbin == 3)
    	    	 {
    	    	    post("detune %f cr = %f pr = %f nr = %f",
    	    	    	detune, currentreal, lastreal, nextreal);
    	    	    post("ci = %f pi = %f ni = %f",
    	    	    	currentimag, lastimag, nextimag);
    	    	    post("x1 %f x2 %f pow %f",
    	    	    	(2.0 * currentreal - lastreal - nextreal),
    	    	    	(2.0 * currentimag - lastimag - nextimag),
    	    	    	pow);
    	    	} */
    	    }
    	    if (detune > 2 || detune < -2) freq = pow = 0;
    	    else freq = fbin + detune;
    	}
    	else freq = pow = 0;
    	*outfreq++ = freq;
    	*outamp++ = oneovern2 * pow;
    	fbin += 1.0f;
    }
    while (m--) *outamp++ = *outfreq++ = 0;
    return (w+6);
}

t_int *sigsqrt_perform(t_int *w);

static void sigframp_dsp(t_sigframp *x, t_signal **sp)
{
    int n = sp[0]->s_n, n2 = (n>>1);
    if (n < 4)
    {
    	error("framp: minimum 4 points");
    	return;
    }
    dsp_add(sigframp_perform, 5, sp[0]->s_vec, sp[1]->s_vec,
    	sp[2]->s_vec, sp[3]->s_vec, n2);
    dsp_add(sigsqrt_perform, 3, sp[3]->s_vec, sp[3]->s_vec, n2);
}

static void sigframp_setup(void)
{
    sigframp_class = class_new(gensym("framp~"), sigframp_new, 0,
    	sizeof(t_sigframp), 0, 0);
    class_addmethod(sigframp_class, nullfn, gensym("signal"), 0);
    class_addmethod(sigframp_class, sigframp_dsp, gensym("dsp"), 0);
}

/* ------------------------ global setup routine ------------------------- */

void d_fft_setup(void)
{
    sigfft_setup();
    sigrfft_setup();
    sigrifft_setup();
    sigframp_setup();
}
