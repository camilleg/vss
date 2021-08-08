/* Copyright (c) 1997 The Regents of the University of California.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  The dac~ and adc~ routines.
*/

#include "m_imp.h"

#if 0 // unused
/* ----------------------------- dac~ --------------------------- */
static t_class *dac_class;

typedef struct _dac
{
    t_object x_obj;
    t_int x_n;
    t_int *x_vec;
} t_dac;

static void *dac_new(t_symbol *s, int argc, t_atom *argv)
{
    t_dac *x = (t_dac *)pd_new(dac_class);
    t_atom defarg[2];
    int i;
    if (!argc)
    {
    	argv = defarg;
    	argc = 2;
    	SETFLOAT(&defarg[0], 1);
    	SETFLOAT(&defarg[1], 2);
    }
    x->x_n = argc;
    x->x_vec = (t_int *)getbytes(argc * sizeof(*x->x_vec));
    for (i = 0; i < argc; i++)
    	x->x_vec[i] = atom_getintarg(i, argc, argv);
    for (i = 1; i < argc; i++)
    	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    return (x);
}

static void dac_signal(t_dac *x)
{
    post("dac: signal");
}

/* link error in red hat ;;;;
static void dac_dsp(t_dac *x, t_signal **sp)
{
    int i, *ip;
    t_signal **sp2;
    for (i = x->x_n, ip = x->x_vec, sp2 = sp; i--; ip++, sp2++)
    {
    	int ch = *ip - 1;
    	if ((*sp2)->s_n != DACBLKSIZE)
    	    error("dac~: bad vector size");
    	else if (ch >= 0 && ch < sys_getch())
    	    dsp_add(plus_perform, 4, sys_soundout + DACBLKSIZE*ch,
    	    	(*sp2)->s_vec, sys_soundout + DACBLKSIZE*ch, DACBLKSIZE);
    }    
}
*/

static void dac_free(t_dac *x)
{
    freebytes(x->x_vec, x->x_n * sizeof(*x->x_vec));
}

static void dac_setup(void)
{
    dac_class = class_new(gensym("dac~"), dac_new, dac_free,
    	sizeof(t_dac), 0, A_GIMME, 0);
    class_addmethod(dac_class, dac_signal, gensym("signal"), 0);
/* link error in red hat ;;;;
    class_addmethod(dac_class, dac_dsp, gensym("dsp"), A_CANT, 0);
*/
}

/* ----------------------------- adc~ --------------------------- */
static t_class *adc_class;

typedef struct _adc
{
    t_object x_obj;
    t_int x_n;
    t_int *x_vec;
} t_adc;

static void *adc_new(t_symbol *s, int argc, t_atom *argv)
{
    t_adc *x = (t_adc *)pd_new(adc_class);
    t_atom defarg[2];
    int i;
    if (!argc)
    {
    	argv = defarg;
    	argc = 2;
    	SETFLOAT(&defarg[0], 1);
    	SETFLOAT(&defarg[1], 2);
    }
    x->x_n = argc;
    x->x_vec = (t_int *)getbytes(argc * sizeof(*x->x_vec));
    for (i = 0; i < argc; i++)
    	x->x_vec[i] = atom_getintarg(i, argc, argv);
    for (i = 0; i < argc; i++)
    	outlet_new(&x->x_obj, &s_signal);
    return (x);
}
#endif

t_int *copy_perform(t_int *w)
{
    t_float *in1 = (t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    while (n--) *out++ = *in1++; 
    return (w+4);
}

/*;;;;
static void adc_dsp(t_adc *x, t_signal **sp)
{
    int i, *ip;
    t_signal **sp2;
    for (i = x->x_n, ip = x->x_vec, sp2 = sp; i--; ip++, sp2++)
    {
    	int ch = *ip - 1;
    	if ((*sp2)->s_n != DACBLKSIZE)
    	    error("adc~: bad vector size");
    	else if (ch >= 0 && ch < sys_getch())
    	    dsp_add(copy_perform, 3, sys_soundin + DACBLKSIZE*ch,
    	    	(*sp2)->s_vec, DACBLKSIZE);
    
    }    
}


static void adc_free(t_adc *x)
{
    freebytes(x->x_vec, x->x_n * sizeof(*x->x_vec));
}

static void adc_setup(void)
{
    adc_class = class_new(gensym("adc~"), adc_new, adc_free,
    	sizeof(t_adc), 0, A_GIMME, 0);
    class_addmethod(adc_class, adc_dsp, gensym("dsp"), A_CANT, 0);
}

void d_dac_setup(void)
{
    dac_setup();
    adc_setup();
}
*/
