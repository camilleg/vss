/* Copyright (c) 1997 The Regents of the University of California.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_imp.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static t_class *newclass;
static void pd_defaultfloat(t_pd *x, t_float f);
static void pd_defaultlist(t_pd *x, t_symbol *s, int argc, t_atom *argv);

static void pd_defaultanything(t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    error("%s: no method for '%s'", (*x)->c_name->s_name, s->s_name);
}

static void pd_defaultbang(t_pd *x)
{
    (*(*x)->c_anymethod)(x, &s_bang, 0, 0);
}

static void pd_defaultpointer(t_pd *x, t_gpointer *gp)
{
    if (*(*x)->c_listmethod != pd_defaultlist)
    {
    	t_atom at;
    	SETPOINTER(&at, gp);
    	(*(*x)->c_listmethod)(x, 0, 1, &at);
    }
    else
    {
    	t_atom at;
    	SETPOINTER(&at, gp);
    	(*(*x)->c_anymethod)(x, &s_pointer, 1, &at);
    }
}

static void pd_defaultfloat(t_pd *x, t_float f)
{
    if (*(*x)->c_listmethod != pd_defaultlist)
    {
    	t_atom at;
    	SETFLOAT(&at, f);
    	(*(*x)->c_listmethod)(x, 0, 1, &at);
    }
    else
    {
	t_atom at;
	SETFLOAT(&at, f);
	(*(*x)->c_anymethod)(x, &s_float, 1, &at);
    }
}

static void pd_defaultsymbol(t_pd *x, t_symbol *s)
{
    if (*(*x)->c_listmethod != pd_defaultlist)
    {
    	t_atom at;
    	SETSYMBOL(&at, s);
    	(*(*x)->c_listmethod)(x, 0, 1, &at);
    }
    else
    {
    	t_atom at;
    	SETSYMBOL(&at, s);
    	(*(*x)->c_anymethod)(x, &s_symbol, 1, &at);
    }
}

void obj_list(t_object *x, t_symbol *s, int argc, t_atom *argv);

static void pd_defaultlist(t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if ((*x)->c_patchable) obj_list((t_object *)x, s, argc, argv);
    else (*(*x)->c_anymethod)(x, &s_list, argc, argv);
}

    /* for now we assume that all "gobjs" are text unless explicitly
    overridden later by calling class_setbehavior().  I'm not sure
    how to deal with Pds that aren't gobjs; shouldn't there be a
    way to check that at run time?  Perhaps the presence of a "newmethod"
    should be our cue, or perhaps the "tiny" flag.  */

    /* another matter.  This routine does two unrelated things: it creates
    a Pd class, but also adds a "new" method to create an instance of it.
    These are combined for historical reasons and for brevity in writing
    objects.  To avoid adding a "new" method send a null function pointer.
    To add additional ones, use class_addcreator below.  Some "classes", like
    "select", are actually two classes of the same name, one for the single-
    argument form, one for the multiple one; see select_setup() to find out
    how this is handled.  */

//;;;; extern t_widgetbehavior text_widgetbehavior;

t_class *class_new(t_symbol *s, t_newmethod newmethod, t_method freemethod,
    size_t size, int flags, t_atomtype type1, ...)
{
    va_list ap;
    t_atomtype vec[MAXPDARG+1], *vp = vec;
    int count = 0;
    t_class *c;
    int typeflag = flags & CLASS_TYPEMASK;
    if (!typeflag) typeflag = CLASS_PATCHABLE;
    *vp = type1;

    va_start(ap, type1);
    while (*vp)
    {
	if (count == MAXPDARG)
	{
    	    error("class %s: sorry: only %d creation args allowed",
    		s->s_name, MAXPDARG);
    	    break;
	}
	vp++;
	count++;
	*vp = va_arg(ap, t_atomtype);
    }
    va_end(ap);
    if (newclass && newmethod)
    	class_addmethod(newclass, (t_method)newmethod, s,
    	    vec[0], vec[1], vec[2], vec[3], vec[4], vec[5]);
    c = (t_class *)t_getbytes(sizeof(*c));
    c->c_name = s;
    c->c_size = size;
    c->c_methods = t_getbytes(0);
    c->c_nmethod = 0;
    c->c_freemethod = (t_freemethod)freemethod;
    c->c_bangmethod = pd_defaultbang;
    c->c_pointermethod = pd_defaultpointer;
    c->c_floatmethod = pd_defaultfloat;
    c->c_symbolmethod = pd_defaultsymbol;
    c->c_listmethod = pd_defaultlist;
    c->c_anymethod = pd_defaultanything;
    //;;;; c->c_wb = (typeflag == CLASS_PATCHABLE ? &text_widgetbehavior : 0);
    c->c_wb = 0; //;;;;
    c->c_pwb = 0;
    c->c_firstin = ((flags & CLASS_NOINLET) == 0);
    c->c_patchable = (typeflag == CLASS_PATCHABLE);
    c->c_gobj = (typeflag >= CLASS_GOBJ);
    c->c_drawcommand = 0;
#if 0
    post("class: %s", c->c_name->s_name);
#endif
    return(c);
}

    /* add a creation method, which is a function that returns a Pd object
    suitable for putting in an object box.  We presume you've got a class it
    can belong to, but this won't be used until the newmethod is actually
    called back (and the new method explicitly takes care of this.) */

void class_addcreator(t_newmethod newmethod, t_symbol *s, 
    t_atomtype type1, ...)
{
    va_list ap;
    t_atomtype vec[MAXPDARG+1], *vp = vec;
    int count = 0;
    *vp = type1;

    va_start(ap, type1);
    while (*vp)
    {
	if (count == MAXPDARG)
	{
    	    error("class %s: sorry: only %d creation args allowed",
    		s->s_name, MAXPDARG);
    	    break;
	}
	vp++;
	count++;
	*vp = va_arg(ap, t_atomtype);
    }
    va_end(ap);
    class_addmethod(newclass, (t_method)newmethod, s,
    	vec[0], vec[1], vec[2], vec[3], vec[4], vec[5]);
}

void class_addmethod(t_class *c, t_method fn, t_symbol *sel,
    t_atomtype arg1, ...)
{
    va_list ap;
    t_methodentry *m;
    t_atomtype t;
    int nargs;
    
    	/* check for special cases.  "Pointer" is missing here so that
    	"#N"'s pointer method can be typechecked differently.  */
    if (sel == &s_bang) class_addbang(c, fn);
    else if (sel == &s_float) class_addfloat(c, fn);
    else if (sel == &s_symbol) class_addsymbol(c, fn);
    else if (sel == &s_list) class_addlist(c, fn);
    else if (sel == &s_anything) class_addanything(c, fn);
    else
    {
	va_start(ap, arg1);
	c->c_methods = t_resizebytes(c->c_methods,
    	    c->c_nmethod * sizeof(*c->c_methods),
    	    (c->c_nmethod + 1) * sizeof(*c->c_methods));
	m = c->c_methods +  c->c_nmethod;
	c->c_nmethod++;
	m->me_name = sel;
	m->me_fun = (t_gotfn)fn;
	nargs = 0;
	t = arg1;
	while (t != A_NULL && nargs < MAXPDARG)
	{
    	    m->me_arg[nargs++] = t;
    	    t = va_arg(ap, t_atomtype);
	}
	va_end(ap);
	m->me_arg[nargs] = A_NULL;
    }
}

void class_addbang(t_class *c, t_method fn)
{
    c->c_bangmethod = (t_bangmethod)fn;
}

void class_addpointer(t_class *c, t_method fn)
{
    c->c_pointermethod = (t_pointermethod)fn;
}

    /* it's better to use the "class_addfloat" macro in m_pd.h */
void class_doaddfloat(t_class *c, t_method fn)
{
    c->c_floatmethod = (t_floatmethod)fn;
}

void class_addsymbol(t_class *c, t_method fn)
{
    c->c_symbolmethod = (t_symbolmethod)fn;
}

void class_addlist(t_class *c, t_method fn)
{
    c->c_listmethod = (t_listmethod)fn;
}

void class_addanything(t_class *c, t_method fn)
{
    c->c_anymethod = (t_anymethod)fn;
}

void class_setwidget(t_class *c, t_widgetbehavior *w)
{
    c->c_wb = w;
}

void class_setparentwidget(t_class *c, t_parentwidgetbehavior *pw)
{
    c->c_pwb = pw;
}

t_symbol *class_symbol(t_class *c)
{
    return (c->c_name);
}

t_parentwidgetbehavior *pd_getparentwidget(t_pd *x)
{
    return ((*x)->c_pwb);
}

void class_setdrawcommand(t_class *c)
{
    c->c_drawcommand = 1;
}

int class_isdrawcommand(t_class *c)
{
    return (c->c_drawcommand);
}

/* ---------------- the symbol table ------------------------ */

#define HASHSIZE 1024

static t_symbol *symhash[HASHSIZE];

t_symbol *dogensym(char *s, t_symbol *oldsym)
{
    t_symbol **sym1, *sym2;
    unsigned int hash1 = 0,  hash2 = 0;
    int length = 0;
    char *s2 = s;
    while (*s2)
    {
	hash1 += *s2;
	hash2 += hash1;
	length++;
	s2++;
    }
    sym1 = symhash + (hash2 & (HASHSIZE-1));
    while ((sym2 = *sym1) != NULL)
    {
	if (!strcmp(sym2->s_name, s)) return(sym2);
	sym1 = &sym2->s_next;
    }
    if (oldsym) sym2 = oldsym;
    else
    {
    	sym2 = (t_symbol *)t_getbytes(sizeof(*sym2));
    	sym2->s_name = t_getbytes(length+1);
    	sym2->s_next = 0;
    	sym2->s_thing = 0;
    	strcpy(sym2->s_name, s);
    }
    *sym1 = sym2;
    return (sym2);
}

t_symbol *gensym(char *s)
{
    return(dogensym(s, 0));
}

/*
static t_symbol *addfileextent(t_symbol *s)
{
    char namebuf[MAXPDSTRING], *str = s->s_name;
    int ln = strlen(str);
    if (!strcmp(str + ln - 3, ".pd")) return (s);
    strcpy(namebuf, str);
    strcpy(namebuf+ln, ".pd");
    return (gensym(namebuf));
}

static int tryingalready;
*/

void canvas_popabstraction(t_canvas *x);
extern t_pd *newest;

/*;;;;
void new_anything(void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    t_pd *current;
    t_symbol *dir = canvas_getcurrentdir();
    if (tryingalready) return;
    newest = 0;
    if (sys_loademup(s->s_name, dir->s_name))
    {
    	tryingalready = 1;
    	typedmess(dummy, s, argc, argv);
    	tryingalready = 0;
    	return;
    }
    current = s__X.s_thing;
    canvas_setargs(argc, argv);
    binbuf_evalfile(addfileextent(s), dir);
    if (s__X.s_thing != current)
    	canvas_popabstraction((t_canvas *)(s__X.s_thing));
    else newest = 0;
}
*/

t_symbol  s_pointer =   {"pointer"};
t_symbol  s_float = 	{"float"};
t_symbol  s_symbol =    {"symbol"};
t_symbol  s_bang =  	{"bang"};
t_symbol  s_list =  	{"list"};
t_symbol  s_anything =	{"anything"};
t_symbol  s_signal =	{"signal"};
t_symbol  s__N =	{"#N"};
t_symbol  s__X =	{"#X"};
t_symbol  s_ = 	    	{""};

static t_symbol *symlist[] = { &s_pointer, &s_float, &s_symbol, &s_bang,
    &s_list, &s_anything, &s_signal, &s__N, &s__X, &s_};

void mess_init(void)
{
    t_symbol **sp;
    int i;

    if (newclass) return;    
    for (i = sizeof(symlist)/sizeof(*symlist), sp = symlist; i--; sp++)
    	(void) dogensym((*sp)->s_name, *sp);
    newclass = class_new(&s__N, 0, 0, sizeof(t_pd), CLASS_DEFAULT, A_NULL);
    pd_bind(&newclass, &s__N);
    //;;;;causes link error with canvases.  class_addanything(newclass, new_anything);
}

t_pd *newest;

    /* horribly, we need prototypes for each of the artificial function
    calls in typedmess(), to keep the compiler quiet. */
typedef t_pd *(*t_newgimme)(t_symbol *s, int argc, t_atom *argv);
typedef void(*t_messgimme)(t_pd *x, t_symbol *s, int argc, t_atom *argv);

typedef t_pd *(*t_fun0)(
    t_floatarg d1, t_floatarg d2, t_floatarg d3, t_floatarg d4, t_floatarg d5);
typedef t_pd *(*t_fun1)(t_int i1,
    t_floatarg d1, t_floatarg d2, t_floatarg d3, t_floatarg d4, t_floatarg d5);
typedef t_pd *(*t_fun2)(t_int i1, int i2,
    t_floatarg d1, t_floatarg d2, t_floatarg d3, t_floatarg d4, t_floatarg d5);
typedef t_pd *(*t_fun3)(t_int i1, int i2, int i3,
    t_floatarg d1, t_floatarg d2, t_floatarg d3, t_floatarg d4, t_floatarg d5);
typedef t_pd *(*t_fun4)(t_int i1, int i2, int i3, int i4,
    t_floatarg d1, t_floatarg d2, t_floatarg d3, t_floatarg d4, t_floatarg d5);
typedef t_pd *(*t_fun5)(t_int i1, int i2, int i3, int i4, int i5,
    t_floatarg d1, t_floatarg d2, t_floatarg d3, t_floatarg d4, t_floatarg d5);
typedef t_pd *(*t_fun6)(t_int i1, int i2, int i3, int i4, int i5, int i6,
    t_floatarg d1, t_floatarg d2, t_floatarg d3, t_floatarg d4, t_floatarg d5);

void pd_typedmess(t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    t_class *c = *x;
    t_methodentry *m;
    t_atomtype *wp, wanttype;
    int i;
    t_int ai[MAXPDARG+1], *ap = ai;
    t_floatarg ad[MAXPDARG+1], *dp = ad;
    int narg = 0;
    t_pd *bonzo = NULL;
    
    	/* check for messages that are handled by fixed slots in the class
    	structure.  We don't catch "pointer" though; so sending "pointer" to
    	"#N" doesn't require that we supply a pointer value. */
    if (s == &s_float)
    {
    	if (!argc) (*c->c_floatmethod)(x, 0.);
    	else if (argv->a_type == A_FLOAT)
    	    (*c->c_floatmethod)(x, argv->a_w.w_float);
    	else goto badarg;
    	return;
    }
    if (s == &s_bang)
    {
    	(*c->c_bangmethod)(x);
    	return;
    }
    if (s == &s_list)
    {
    	(*c->c_listmethod)(x, s, argc, argv);
    	return;
    }
    if (s == &s_symbol)
    {
    	if (!argc) goto badarg;
    	else if (argv->a_type == A_SYMBOL)
    	    (*c->c_symbolmethod)(x, argv->a_w.w_symbol);
    	else goto badarg;
    	return;
    }
    for (i = c->c_nmethod, m = c->c_methods; i--; m++)
	if (m->me_name == s)
    {
	wp = m->me_arg;
	if (*wp == A_GIMME)
	{
	    if (x == &newclass)
		newest = (*((t_newgimme)(m->me_fun)))(s, argc, argv);
	    else (*((t_messgimme)(m->me_fun)))(x, s, argc, argv);
	    return;
	}
	if (argc > MAXPDARG) argc = MAXPDARG;
	if (x != &newclass) *(ap++) = (t_int)x, narg++;
	while (wanttype = *wp++)
	{
	    switch (wanttype)
	    {
	    case A_POINTER:
		if (!argc) goto badarg;
		else
		{
	    	    if (argv->a_type == A_POINTER)
	    	    	*ap = (t_int)(argv->a_w.w_gpointer);
	    	    else goto badarg;
		    argc--;
		    argv++;
		}
		narg++;
		ap++;
		break;
	    case A_FLOAT:
		if (!argc) goto badarg;
    	    case A_DEFFLOAT:
    		if (!argc) *dp = 0;
		else
		{
	    	    if (argv->a_type == A_FLOAT) *dp = argv->a_w.w_float;
		    else goto badarg;
		    argc--;
		    argv++;
		}
		dp++;
		break;
	    case A_SYMBOL:
		if (!argc) goto badarg;
    	    case A_DEFSYM:
    		if (!argc) *ap = (t_int)(&s_);
    		else
    		{
	    	    if (argv->a_type == A_SYMBOL) *ap = (t_int)(argv->a_w.w_symbol);
		    else goto badarg;
		    argc--;
		    argv++;
		}
		narg++;
		ap++;
    	    }
	}
	switch (narg)
	{
    	case 0 : bonzo = (*(t_fun0)(m->me_fun))
	    (ad[0], ad[1], ad[2], ad[3], ad[4]); break;
    	case 1 : bonzo = (*(t_fun1)(m->me_fun))
	    (ai[0], ad[0], ad[1], ad[2], ad[3], ad[4]); break;
    	case 2 : bonzo = (*(t_fun2)(m->me_fun))
	    (ai[0], ai[1], ad[0], ad[1], ad[2], ad[3], ad[4]); break;
    	case 3 : bonzo = (*(t_fun3)(m->me_fun))
	    (ai[0], ai[1], ai[2], ad[0], ad[1], ad[2], ad[3], ad[4]); break;
    	case 4 : bonzo = (*(t_fun4)(m->me_fun))
	    (ai[0], ai[1], ai[2], ai[3],
	    	ad[0], ad[1], ad[2], ad[3], ad[4]); break;
    	case 5 : bonzo = (*(t_fun5)(m->me_fun))
	    (ai[0], ai[1], ai[2], ai[3], ai[4],
	    	ad[0], ad[1], ad[2], ad[3], ad[4]); break;
    	case 6 : bonzo = (*(t_fun6)(m->me_fun))
	    (ai[0], ai[1], ai[2], ai[3], ai[4], ai[5],
	    	ad[0], ad[1], ad[2], ad[3], ad[4]); break;
	}
	if (x == &newclass) newest = bonzo;
	return;
    }
    (*c->c_anymethod)(x, s, argc, argv);
    return;
badarg:
    error("Bad arguments for message '%s' to object '%s'",
    	s->s_name, c->c_name->s_name);
}

void vmess(t_pd *x, t_symbol *sel, char *fmt, ...)
{
    va_list ap;
    t_atom arg[MAXPDARG], *at =arg;
    int nargs = 0;
    char *fp = fmt;

    va_start(ap, fmt);
    while (1)
    {
    	if (nargs >= MAXPDARG)
    	{
    	    error("pd_vmess: only %d allowed", MAXPDARG);
    	    break;
    	}
    	switch(*fp++)
    	{
    	case 'f': SETFLOAT(at, va_arg(ap, double)); break;
    	case 's': SETSYMBOL(at, va_arg(ap, t_symbol *)); break;
    	case 'i': SETFLOAT(at, va_arg(ap, t_int)); break;	
    	case 'p': SETPOINTER(at, va_arg(ap, t_gpointer *)); break;
    	default: goto done;
    	}
    	at++;
    	nargs++;
    }
done:
    va_end(ap);
    typedmess(x, sel, nargs, arg);
}

void pd_forwardmess(t_pd *x, int argc, t_atom *argv)
{
    if (argc)
    {
    	t_atomtype t = argv->a_type;
    	if (t == A_SYMBOL) pd_typedmess(x, argv->a_w.w_symbol, argc-1, argv+1);
    	else if (t == A_POINTER)
    	{
    	    if (argc == 1) pd_pointer(x, argv->a_w.w_gpointer);
    	    else pd_list(x, &s_list, argc, argv);
    	}
    	else if (t == A_FLOAT)
    	{
    	    if (argc == 1) pd_float(x, argv->a_w.w_float);
    	    else pd_list(x, &s_list, argc, argv);
    	}
    	else bug("pd_forwardmess");
    }

}

void nullfn(void) {}

t_gotfn getfn(t_pd *x, t_symbol *s)
{
    t_class *c = *x;
    t_methodentry *m;
    int i;

    for (i = c->c_nmethod, m = c->c_methods; i--; m++)
	if (m->me_name == s) return(m->me_fun);
    error("%s: no method for message '%s'", c->c_name->s_name, s->s_name);
    return((t_gotfn)nullfn);
}

t_gotfn zgetfn(t_pd *x, t_symbol *s)
{
    t_class *c = *x;
    t_methodentry *m;
    int i;

    for (i = c->c_nmethod, m = c->c_methods; i--; m++)
	if (m->me_name == s) return(m->me_fun);
    return(0);
}
