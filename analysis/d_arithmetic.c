/* Copyright (c) 1997 The Regents of the University of California.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  arithmetic.
*/

#include "m_pd.h"

/* ----------------------------- plus ----------------------------- */
static t_class *plus_class;

typedef struct _plus
{
    t_object x_obj;
} t_plus;

static void *plus_new(t_symbol *s, int argc, t_atom *argv)
{
    t_plus *x = (t_plus *)pd_new(plus_class);
    if (argc > 1) post("+~: extra arguments ignored");
    if (argc) 
    {
    	post("scalar plus: uninplemented");
    }
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

t_int *plus_perform(t_int *w)
{
    t_float *in1 = (t_float *)(w[1]);
    t_float *in2 = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    while (n--) *out++ = *in1++ + *in2++; 
    return (w+5);
}

static void plus_dsp(t_plus *x, t_signal **sp)
{
    dsp_add(plus_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void plus_setup(void)
{
    plus_class = class_new(gensym("+~"), plus_new, 0,
    	sizeof(t_plus), 0, A_GIMME, 0);
    class_addmethod(plus_class, nullfn, gensym("signal"), 0);
    class_addmethod(plus_class, plus_dsp, gensym("dsp"), 0);
}

/* ----------------------------- minus ----------------------------- */
static t_class *minus_class;

typedef struct _minus
{
    t_object x_obj;
} t_minus;

static void *minus_new(t_symbol *s, int argc, t_atom *argv)
{
    t_minus *x = (t_minus *)pd_new(minus_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

static t_int *minus_perform(t_int *w)
{
    t_float *in1 = (t_float *)(w[1]);
    t_float *in2 = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    while (n--) *out++ = *in1++ - *in2++; 
    return (w+5);
}

static void minus_dsp(t_minus *x, t_signal **sp)
{
    dsp_add(minus_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void minus_setup(void)
{
    minus_class = class_new(gensym("-~"), minus_new, 0,
    	sizeof(t_minus), 0, A_GIMME, 0);
    class_addmethod(minus_class, nullfn, gensym("signal"), 0);
    class_addmethod(minus_class, minus_dsp, gensym("dsp"), 0);
}

/* ----------------------------- times ----------------------------- */
static t_class *times_class;

typedef struct _times
{
    t_object x_obj;
} t_times;

static void *times_new(t_symbol *s, int argc, t_atom *argv)
{
    t_times *x = (t_times *)pd_new(times_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

static t_int *times_perform(t_int *w)
{
    t_float *in1 = (t_float *)(w[1]);
    t_float *in2 = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    while (n--) *out++ = *in1++ * *in2++; 
    return (w+5);
}

static void times_dsp(t_times *x, t_signal **sp)
{
    dsp_add(times_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec,
    	sp[0]->s_n);
}

static void times_setup(void)
{
    times_class = class_new(gensym("*~"), times_new, 0,
    	sizeof(t_times), 0, A_GIMME, 0);
    class_addmethod(times_class, nullfn, gensym("signal"), 0);
    class_addmethod(times_class, times_dsp, gensym("dsp"), 0);
}

/* ----------------------------- over ----------------------------- */
static t_class *over_class;

typedef struct _over
{
    t_object x_obj;
} t_over;

static void *over_new(t_symbol *s, int argc, t_atom *argv)
{
    t_over *x = (t_over *)pd_new(over_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

static t_int *over_perform(t_int *w)
{
    t_float *in1 = (t_float *)(w[1]);
    t_float *in2 = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    while (n--) *out++ = *in1++ / *in2++; 
    return (w+5);
}

static void over_dsp(t_over *x, t_signal **sp)
{
    dsp_add(over_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec,
    	sp[0]->s_n);
}

static void over_setup(void)
{
    over_class = class_new(gensym("/~"), over_new, 0,
    	sizeof(t_over), 0, A_GIMME, 0);
    class_addmethod(over_class, nullfn, gensym("signal"), 0);
    class_addmethod(over_class, over_dsp, gensym("dsp"), 0);
}

/* ----------------------- global setup routine ---------------- */
void d_arithmetic_setup(void)
{
    plus_setup();
    minus_setup();
    times_setup();
    over_setup();
}

