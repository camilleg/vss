/* Copyright (c) 1997 The Regents of the University of California.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* this file handles Max-style patchable objects, i.e., objects which
can interconnect via inlets and outlets. */

#include "m_imp.h"

union inletunion
{
    t_symbol *iu_symto;
    t_gpointer *iu_pointerslot;
    t_float *iu_floatslot;
    t_symbol **iu_symslot;
};

struct _inlet
{
    t_pd i_pd;
    struct _inlet *i_next;
    t_object *i_owner;
    t_pd *i_dest;
    t_symbol *i_symfrom;
    union inletunion i_un;
};

#define i_symto i_un.iu_symto
#define i_pointerslot i_un.iu_pointerslot
#define i_floatslot i_un.iu_floatslot
#define i_symslot i_un.iu_symslot

static t_class *inlet_class, *pointerinlet_class, *floatinlet_class,
    *symbolinlet_class;

#define ISINLET(pd) ((*(pd) == inlet_class) || \
    (*(pd) == pointerinlet_class) || \
    (*(pd) == floatinlet_class) || \
    (*(pd) == symbolinlet_class))

/* --------------------- generic inlets ala max ------------------ */

t_inlet *inlet_new(t_object *owner, t_pd *dest, t_symbol *s1, t_symbol *s2)
{
    t_inlet *x = (t_inlet *)pd_new(inlet_class), *y, *y2;
    x->i_owner = owner;
    x->i_dest = dest;
    x->i_symfrom = s1;
    x->i_symto = s2;
    x->i_next = 0;
    if ((y = owner->ob_inlet))
    {
        while ((y2 = y->i_next)) y = y2;
    	y->i_next = x;
    }
    else owner->ob_inlet = x;
    return (x);
}

static void inlet_wrong(t_inlet *x, t_symbol *s)
{
    error("inlet: expected '%s' but got '%s'",
    	x->i_symfrom->s_name, s->s_name);
}

    /* LATER figure out how to make these efficient: */
static void inlet_bang(t_inlet *x)
{
    if (x->i_symfrom == &s_bang) vmess(x->i_dest, x->i_symto, "");
    else if (!x->i_symfrom) pd_bang(x->i_dest);
    else inlet_wrong(x, &s_bang);
}

static void inlet_pointer(t_inlet *x, t_gpointer *gp)
{
    if (x->i_symfrom == &s_pointer) vmess(x->i_dest, x->i_symto, "p", gp);
    else if (!x->i_symfrom) pd_pointer(x->i_dest, gp);
    else inlet_wrong(x, &s_pointer);
}

static void inlet_float(t_inlet *x, t_float f)
{
    if (x->i_symfrom == &s_float) vmess(x->i_dest, x->i_symto, "f",
    	(t_floatarg)f);
    else if (!x->i_symfrom) pd_float(x->i_dest, f);
    else inlet_wrong(x, &s_float);
}

static void inlet_symbol(t_inlet *x, t_symbol *s)
{
    if (x->i_symfrom == &s_symbol) vmess(x->i_dest, x->i_symto, "s", s);
    else if (!x->i_symfrom) pd_symbol(x->i_dest, s);
    else inlet_wrong(x, &s_symbol);
}

static void inlet_list(t_inlet *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->i_symfrom == &s_list) typedmess(x->i_dest, x->i_symto, argc, argv);
    else if (!x->i_symfrom) pd_list(x->i_dest, s, argc, argv);
    else inlet_wrong(x, &s_list);
}

static void inlet_anything(t_inlet *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->i_symfrom == s)
    	typedmess(x->i_dest, x->i_symto, argc, argv);
    else if (!x->i_symfrom)
    	typedmess(x->i_dest, s, argc, argv);
    else inlet_wrong(x, s);
}

void inlet_free(t_inlet *x)
{
    t_object *y = x->i_owner;
    t_inlet *x2;
    if (y->ob_inlet == x) y->ob_inlet = x->i_next;
    else for (x2 = y->ob_inlet; x2; x2 = x2->i_next)
    	if (x2->i_next == x)
    {
    	x2->i_next = x->i_next;
    	break;
    }
    t_freebytes(x, sizeof(*x));
}

/* ----- pointerinlets, floatinlets, syminlets: optimized inlets ------- */

static void pointerinlet_pointer(t_inlet *x, t_gpointer *gp)
{
	//;;;;;;;; simply took this line out, didn't figure it out properly.  hope this is safe.
    //;;;;gpointer_unset(x->i_pointerslot);
    *(x->i_pointerslot) = *gp;
    if (gp->gp_stub) gp->gp_stub->gs_refcount++;
}

t_inlet *pointerinlet_new(t_object *owner, t_gpointer *gp)
{
    t_inlet *x = (t_inlet *)pd_new(pointerinlet_class), *y, *y2;
    x->i_owner = owner;
    x->i_dest = 0;
    x->i_symfrom = &s_pointer;
    x->i_pointerslot = gp;
    x->i_next = 0;
    if ((y = owner->ob_inlet))
    {
        while ((y2 = y->i_next)) y = y2;
    	y->i_next = x;
    }
    else owner->ob_inlet = x;
    return (x);
}

static void floatinlet_float(t_inlet *x, t_float f)
{
    *(x->i_floatslot) = f;
}

t_inlet *floatinlet_new(t_object *owner, t_float *fp)
{
    t_inlet *x = (t_inlet *)pd_new(floatinlet_class), *y, *y2;
    x->i_owner = owner;
    x->i_dest = 0;
    x->i_symfrom = &s_float;
    x->i_floatslot = fp;
    x->i_next = 0;
    if ((y = owner->ob_inlet))
    {
        while ((y2 = y->i_next)) y = y2;
    	y->i_next = x;
    }
    else owner->ob_inlet = x;
    return (x);
}

static void symbolinlet_symbol(t_inlet *x, t_symbol *s)
{
    *(x->i_symslot) = s;
}

t_inlet *symbolinlet_new(t_object *owner, t_symbol **sp)
{
    t_inlet *x = (t_inlet *)pd_new(symbolinlet_class), *y, *y2;
    x->i_owner = owner;
    x->i_dest = 0;
    x->i_symfrom = &s_symbol;
    x->i_symslot = sp;
    x->i_next = 0;
    if ((y = owner->ob_inlet))
    {
        while ((y2 = y->i_next)) y = y2;
    	y->i_next = x;
    }
    else owner->ob_inlet = x;
    return (x);
}

/* ---------------------- routine to handle lists ---------------------- */

void obj_list(t_object *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom *ap;
    int count;
    t_inlet *ip = ((t_object *)x)->ob_inlet;
    if (!argc) return;
    for (count = argc-1, ap = argv+1; ip && count--; ap++, ip = ip->i_next)
    {
    	if (ap->a_type == A_POINTER) pd_pointer(&ip->i_pd, ap->a_w.w_gpointer);
    	else if (ap->a_type == A_FLOAT) pd_float(&ip->i_pd, ap->a_w.w_float);
    	else pd_symbol(&ip->i_pd, ap->a_w.w_symbol);
    }
    if (argv->a_type == A_POINTER) pd_pointer(&x->ob_pd, argv->a_w.w_gpointer);
    else if (argv->a_type == A_FLOAT) pd_float(&x->ob_pd, argv->a_w.w_float);
    else pd_symbol(&x->ob_pd, argv->a_w.w_symbol);
} 

void obj_init(void)
{
    inlet_class = class_new(gensym("inlet"), 0, 0,
    	sizeof(t_inlet), CLASS_PD, 0);
    class_addbang(inlet_class, inlet_bang);
    class_addpointer(inlet_class, inlet_pointer);
    class_addfloat(inlet_class, inlet_float);
    class_addsymbol(inlet_class, inlet_symbol);
    class_addlist(inlet_class, inlet_list);
    class_addanything(inlet_class, inlet_anything);

    pointerinlet_class = class_new(gensym("inlet"), 0, 0,
    	sizeof(t_inlet), CLASS_PD, 0);
    class_addpointer(pointerinlet_class, pointerinlet_pointer);

    floatinlet_class = class_new(gensym("inlet"), 0, 0,
    	sizeof(t_inlet), CLASS_PD, 0);
    class_addfloat(floatinlet_class, (t_method)floatinlet_float);

    symbolinlet_class = class_new(gensym("inlet"), 0, 0,
    	sizeof(t_inlet), CLASS_PD, 0);
    class_addsymbol(symbolinlet_class, symbolinlet_symbol);

}

struct _outconnect
{
    struct _outconnect *oc_next;
    t_pd *oc_to;
};

struct _outlet
{
    t_object *o_owner;
    struct _outlet *o_next;
    t_outconnect *o_connections;
    t_symbol *o_sym;
};

t_outlet *outlet_new(t_object *owner, t_symbol *s)
{
    t_outlet *x = (t_outlet *)getbytes(sizeof(*x)), *y, *y2;
    x->o_owner = owner;
    x->o_next = 0;
    if ((y = owner->ob_outlet))
    {
        while ((y2 = y->o_next)) y = y2;
    	y->o_next = x;
    }
    else owner->ob_outlet = x;
    x->o_connections = 0;
    x->o_sym = s;
    return (x);
}

void outlet_bang(t_outlet *x)
{
    t_outconnect *oc;
    for (oc = x->o_connections; oc; oc = oc->oc_next)
    	pd_bang(oc->oc_to);
}

void outlet_pointer(t_outlet *x, t_gpointer *gp)
{
    t_outconnect *oc;
    t_gpointer gpointer;
    gpointer = *gp; 
    for (oc = x->o_connections; oc; oc = oc->oc_next)
    	pd_pointer(oc->oc_to, &gpointer);
}

void outlet_float(t_outlet *x, t_float f)
{
    t_outconnect *oc;
    for (oc = x->o_connections; oc; oc = oc->oc_next)
    	pd_float(oc->oc_to, f);
}

void outlet_symbol(t_outlet *x, t_symbol *s)
{
    t_outconnect *oc;
    for (oc = x->o_connections; oc; oc = oc->oc_next)
    	pd_symbol(oc->oc_to, s);
}

void outlet_list(t_outlet *x, t_symbol *s, int argc, t_atom *argv)
{
    t_outconnect *oc;
    for (oc = x->o_connections; oc; oc = oc->oc_next)
    	pd_list(oc->oc_to, s, argc, argv);
}

void outlet_anything(t_outlet *x, t_symbol *s, int argc, t_atom *argv)
{
    t_outconnect *oc;
    for (oc = x->o_connections; oc; oc = oc->oc_next)
    	typedmess(oc->oc_to, s, argc, argv);
}

void outlet_free(t_outlet *x)
{
    t_object *y = x->o_owner;
    t_outlet *x2;
    if (y->ob_outlet == x) y->ob_outlet = x->o_next;
    else for (x2 = y->ob_outlet; x2; x2 = x2->o_next)
    	if (x2->o_next == x)
    {
    	x2->o_next = x->o_next;
    	break;
    }
    t_freebytes(x, sizeof(*x));
}

t_outconnect *obj_connect(t_object *source, int outno,
    t_object *sink, int inno)
{
    t_inlet *i;
    t_outlet *o;
    t_pd *to;
    t_outconnect *oc, *oc2;
    
    for (o = source->ob_outlet; o && outno; o = o->o_next, outno--) ;
    if (!o) return (0);
    
    if (sink->ob_pd->c_firstin)
    {
    	if (!inno)
    	{
    	    to = &sink->ob_pd;
    	    goto doit;
    	}
    	else inno--;
    }
    for (i = sink->ob_inlet; i && inno; i = i->i_next, inno--) ;
    if (!i) return (0);
    to = &i->i_pd;
doit:
    oc = (t_outconnect *)t_getbytes(sizeof(*oc));
    oc->oc_next = 0;
    oc->oc_to = to;
    	/* append it to the end of the list */
    	/* LATER we might cache the last "oc" to make this faster. */
    if ((oc2 = o->o_connections))
    {
    	while (oc2->oc_next) oc2 = oc2->oc_next;
    	oc2->oc_next = oc;
    }
    else o->o_connections = oc;
    //;;;;if (o->o_sym == &s_signal) canvas_update_dsp();

    return (oc);
}

void obj_disconnect(t_object *source, int outno, t_object *sink, int inno)
{
    t_inlet *i;
    t_outlet *o;
    t_pd *to;
    t_outconnect *oc, *oc2;
    
    for (o = source->ob_outlet; o && outno; o = o->o_next, outno--)
    if (!o) return; // "Opposite inner 'if' condition leads to a dead code block; outer condition is 'x' and inner condition is '!x'."
    if (sink->ob_pd->c_firstin)
    {
    	if (!inno)
    	{
    	    to = &sink->ob_pd;
    	    goto doit;
    	}
    	else inno--;
    }
    for (i = sink->ob_inlet; i && inno; i = i->i_next, inno--) ;
    if (!i) return;
    to = &i->i_pd;
doit:
    if (!(oc = o->o_connections)) return;
    if (oc->oc_to == to)
    {
    	o->o_connections = oc->oc_next;
    	freebytes(oc, sizeof(*oc));
    	goto done;
    }
    while ((oc2 = oc->oc_next))
    {
    	if (oc2->oc_to == to)
    	{
    	    oc->oc_next = oc2->oc_next;
    	    freebytes(oc2, sizeof(*oc2));
    	    goto done;
    	}
    	oc = oc2;
    }
done:
    //;;;;if (o->o_sym == &s_signal) canvas_update_dsp();
	;
}

/* ------ traversal routines for code that can't see our structures ------ */

int obj_noutlets(t_object *x)
{
    int n;
    t_outlet *o;
    for (o = x->ob_outlet, n = 0; o; o = o->o_next) n++;
    return (n);
}

int obj_ninlets(t_object *x)
{
    int n;
    t_inlet *i;
    for (i = x->ob_inlet, n = 0; i; i = i->i_next) n++;
    if (x->ob_pd->c_firstin) n++;
    return (n);
}

t_outconnect *obj_starttraverseoutlet(t_object *x, t_outlet **op, int nout)
{
    t_outlet *o = x->ob_outlet;
    while (nout-- && o) o = o->o_next;
    *op = o;
    if (o) return (o->o_connections);
    else return (0);
}

t_outconnect *obj_nexttraverseoutlet(t_outconnect *lastconnect,
    t_object **destp, t_inlet **inletp, int *whichp)
{
    t_pd *y;
    y = lastconnect->oc_to;
    if (ISINLET(y))
    {
    	int n;
    	t_inlet *i = (t_inlet *)y, *i2;
    	t_object *dest = i->i_owner;
    	for (n = dest->ob_pd->c_firstin, i2 = dest->ob_inlet;
    	    i2 && i2 != i; i2 = i2->i_next) n++;
    	*whichp = n;
    	*destp = dest;
    	*inletp = i;
    }
    else
    {
    	*whichp = 0;
    	*inletp = 0;
    	*destp = ((t_object *)y);
    }
    return (lastconnect->oc_next);
}

    /* this one checks that a pd is indeed a patchable object, and returns
    it, correctly typed, or zero if the check failed. */
t_object *pd_checkobject(t_pd *x)
{
    if ((*x)->c_patchable) return ((t_object *)x);
    else return (0);
}

    /* move an inlet or outlet to the head of the list */
void obj_moveinletfirst(t_object *x, t_inlet *i)
{
    t_inlet *i2;
    if (x->ob_inlet == i) return;
    else for (i2 = x->ob_inlet; i2; i2 = i2->i_next)
    	if (i2->i_next == i)
    {
    	i2->i_next = i->i_next;
    	i->i_next = x->ob_inlet;
    	x->ob_inlet = i;
    	return;
    }
}

void obj_moveoutletfirst(t_object *x, t_outlet *o)
{
    t_outlet *o2;
    if (x->ob_outlet == o) return;
    else for (o2 = x->ob_outlet; o2; o2 = o2->o_next)
    	if (o2->o_next == o)
    {
    	o2->o_next = o->o_next;
    	o->o_next = x->ob_outlet;
    	x->ob_outlet = o;
    	return;
    }
}

    /* routines for DSP sorting, which are used in d_ugen.c and g_canvas.c */
    /* LATER try to consolidate all the slightly different routines. */

int obj_nsiginlets(t_object *x)
{
    int n;
    t_inlet *i;
    for (i = x->ob_inlet, n = 0; i; i = i->i_next)
    	if (i->i_symfrom == &s_signal) n++;
    if (x->ob_pd->c_firstin && zgetfn(&x->ob_pd, &s_signal)) n++;
    return (n);
}

    /* get the index, among signal inlets, of the mth inlet overall */
int obj_siginletindex(t_object *x, int m)
{
    int n = 0;
    t_inlet *i;
    if (x->ob_pd->c_firstin && zgetfn(&x->ob_pd, &s_signal))
    {
    	if (!m--) return (0);
    	n++;
    }
    for (i = x->ob_inlet; i; i = i->i_next, m--)
    	if (i->i_symfrom == &s_signal)
    {
    	if (m == 0) return (n);
    	n++;
    }
    return (-1);
}

int obj_nsigoutlets(t_object *x)
{
    int n;
    t_outlet *o;
    for (o = x->ob_outlet, n = 0; o; o = o->o_next)
    	if (o->o_sym == &s_signal) n++;
    return (n);
}

int obj_sigoutletindex(t_object *x, int m)
{
    int n;
    t_outlet *o2;
    for (o2 = x->ob_outlet, n = 0; o2; o2 = o2->o_next, m--)
    	if (o2->o_sym == &s_signal)
    {
    	if (m == 0) return (n);
    	n++;
    }
    return (-1);
}

int obj_issignaloutlet(t_object *x, int m)
{
    t_outlet *o2;
    for (o2 = x->ob_outlet; o2 && m--; o2 = o2->o_next);
    return (o2 && (o2->o_sym == &s_signal));
}

/* and these are only used in g_io.c... */

int inlet_getsignalindex(t_inlet *x)
{
    int n = 0;
    t_inlet *i;
    for (i = x->i_owner->ob_inlet, n = 0; i && i != x; i = i->i_next)
    	if (i->i_symfrom == &s_signal) n++;
    return (n);
}

int outlet_getsignalindex(t_outlet *x)
{
    int n = 0;
    t_outlet *o;
    for (o = x->o_owner->ob_outlet, n = 0; o && o != x; o = o->o_next) 
    	if (o->o_sym == &s_signal) n++;
    return (n);
}
