// Exact duplicate of PD's m_pd.h (version 0.23 patchlevel 2, from
// pd-0_23p2_tar.tgz), except for "#define EXTERN_STRUCT struct".

/* Copyright (c) 1997 The Regents of the University of California.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifdef _LANGUAGE_C_PLUS_PLUS
extern "C" {
#ifdef NT
#pragma warning( disable : 4091 ) 
#endif /* NT */
#endif

    /* the external storage class is "extern" in UNIX; in NT it's ugly. */
#ifdef NT
#ifdef PD_INTERNAL
#define EXTERN __declspec(dllexport) extern
#else
#define EXTERN __declspec(dllimport) extern
#endif /* PD_INTERNAL */
#else
#define EXTERN extern
#endif /* NT */

    /* and depending on the compiler, hidden data structures are
    declared differently: */
//#ifdef __GNUC__
//#define EXTERN_STRUCT struct
//#else
//#define EXTERN_STRUCT extern struct
//#endif
#define EXTERN_STRUCT struct


#if !defined(_SIZE_T) && !defined(_SIZE_T_)
#include <stddef.h> 	/* just for size_t -- how lame! */
#endif

#define MAXPDSTRING 1000	/* use this for anything you want */
#define MAXPDARG 5    	    	/* max number of args we can typecheck today */

    /* an integer type the size of a pointer:  */
typedef long t_int; // Everything is 64-bit, by 2022.

typedef float t_float;	/* a floating-point number at most the same size */
typedef float t_floatarg;  /* floating-point type for function calls */

typedef struct _symbol
{
    char *s_name;
    struct _class **s_thing;
    struct _symbol *s_next;
} t_symbol;

EXTERN_STRUCT _array;
#define t_array struct _array 	    /* g_canvas.h */

/* pointers to glist and array elements go through a "stub" which sticks
around after the glist or array is freed.  The stub itself is deleted when
both the glist/array is gone and the refcount is zero, ensuring that no
gpointers are pointing here. */

#define GP_NONE 0  	/* the stub points nowhere (has been cut off) */
#define GP_GLIST 1  	/* the stub points to a glist element */
#define GP_ARRAY 2  	/* ... or array */

typedef struct _gstub
{
    union
    {
    	struct _glist *gs_glist;    /* glist we're in */
    	struct _array *gs_array;    /* array we're in */
    } gs_un;
    int gs_which;   	    	    /* GP_GLIST/GP_ARRAY */
    int gs_refcount;   	    	    /* number of gpointers pointing here */
} t_gstub;

typedef struct _gpointer 	   /* pointer to a gobj in a glist */
{
    union
    {	
    	struct _scalar *gp_scalar;  /* scalar we're in (if glist) */
    	union word *gp_w;  	    /* raw data (if array) */
    } gp_un;
    int gp_valid;   	    	    /* number which must match gpointee */
    t_gstub *gp_stub;	    	    /* stub which points to glist/array */
} t_gpointer;

typedef union word
{
    t_float w_float;
    t_symbol *w_symbol;
    t_gpointer *w_gpointer;
    t_array *w_array;
    struct _glist *w_list;
    int w_index;
} t_word;

typedef enum
{
    A_NULL,
    A_FLOAT,
    A_SYMBOL,
    A_POINTER,
    A_SEMI,
    A_COMMA,
    A_DEFFLOAT,
    A_DEFSYM,
    A_DOLLAR, 
    A_DOLLSYM,
    A_GIMME,
    A_CANT
}  t_atomtype;

typedef struct _atom
{
    t_atomtype a_type;
    union word a_w;
} t_atom;

EXTERN_STRUCT _class;
#define t_class struct _class

EXTERN_STRUCT _outlet;
#define t_outlet struct _outlet

EXTERN_STRUCT _inlet;
#define t_inlet struct _inlet

EXTERN_STRUCT _binbuf;
#define t_binbuf struct _binbuf

EXTERN_STRUCT _clock;
#define t_clock struct _clock

EXTERN_STRUCT _canvas;
#define t_canvas struct _canvas

typedef t_class *t_pd;	    /* pure datum: nothing but a class pointer */

typedef struct _gobj	    /* a graphical object */
{
    t_pd g_pd;
    struct _gobj *g_next;
} t_gobj;

typedef struct _scalar	    /* a graphical object holding data */
{
    t_gobj x_gobj;
    t_symbol *x_template;
    t_word x_vec[1];
} t_scalar;

typedef struct _text	    /* a graphical object with at least one textfield */
{
    t_gobj te_g;
    t_binbuf *te_binbuf;
    t_outlet *te_outlet;
    t_inlet *te_inlet;
    short te_xpos;
    short te_ypos;
    short te_width;	/* in length units but should be in chars */
    unsigned int te_type:2; 	/* from defs below */
} t_text;

#define T_TEXT 0    	/* just a textual comment */
#define T_OBJECT 1  	/* a MAX style patchable object */
#define T_MESSAGE 2    	/* a MAX stype message */
#define T_ATOM 3    	/* a cell to display a number or symbol */

#define te_pd te_g.g_pd

   /* patchable objects ala Max.  At the moment, this is no different from any
   "text-bearing" object; later we might want text objects without
   inlets/outlets; it's not clear yet how to do that.  */

typedef struct _text t_object;

#define ob_outlet te_outlet
#define ob_inlet te_inlet
#define ob_binbuf te_binbuf
#define ob_pd te_g.g_pd
#define ob_g te_g

typedef void (*t_method)();
typedef void *(*t_newmethod)();
typedef void (*t_freemethod)(void *x);
typedef void (*t_gotfn)(void *x, ...);

/* ---------------- pre-defined symbols --------------*/
EXTERN t_symbol s_pointer;
EXTERN t_symbol s_float;
EXTERN t_symbol s_symbol;
EXTERN t_symbol s_bang;
EXTERN t_symbol s_list;
EXTERN t_symbol s_anything;
EXTERN t_symbol s_signal;
EXTERN t_symbol s__N;
EXTERN t_symbol s__X;
EXTERN t_symbol s_;

/* --------- prototypes from the central message system ----------- */
EXTERN void pd_typedmess(t_pd *x, t_symbol *s, int argc, t_atom *argv);
EXTERN void pd_forwardmess(t_pd *x, int argc, t_atom *argv);
EXTERN t_symbol *gensym(char *s);
EXTERN t_gotfn getfn(t_pd *x, t_symbol *s);
EXTERN t_gotfn zgetfn(t_pd *x, t_symbol *s);
EXTERN void nullfn(void);
EXTERN void vmess(t_pd *x, t_symbol *s, char *fmt, ...);
#define mess0(x, s) ((*getfn((x), (s)))((x)))
#define mess1(x, s, a) ((*getfn((x), (s)))((x), (a)))
#define mess2(x, s, a,b) ((*getfn((x), (s)))((x), (a),(b)))
#define mess3(x, s, a,b,c) ((*getfn((x), (s)))((x), (a),(b),(c)))
#define mess4(x, s, a,b,c,d) ((*getfn((x), (s)))((x), (a),(b),(c),(d)))
#define mess5(x, s, a,b,c,d,e) ((*getfn((x), (s)))((x), (a),(b),(c),(d),(e)))

/* --------------- memory management -------------------- */
EXTERN void *getbytes(size_t nbytes);
EXTERN void *copybytes(void *src, size_t nbytes);
EXTERN void freebytes(void *x, size_t nbytes);
EXTERN void *resizebytes(void *x, size_t oldsize, size_t newsize);

/* -------------------- atoms ----------------------------- */

#define SETSEMI(atom) ((atom)->a_type = A_SEMI, (atom)->a_w.w_index = 0)
#define SETCOMMA(atom) ((atom)->a_type = A_COMMA, (atom)->a_w.w_index = 0)
#define SETPOINTER(atom, gp) ((atom)->a_type = A_POINTER, \
    (atom)->a_w.w_gpointer = (gp))
#define SETFLOAT(atom, f) ((atom)->a_type = A_FLOAT, (atom)->a_w.w_float = (f))
#define SETSYMBOL(atom, s) ((atom)->a_type = A_SYMBOL, \
    (atom)->a_w.w_symbol = (s))
#define SETDOLLAR(atom, n) ((atom)->a_type = A_DOLLAR, \
    (atom)->a_w.w_index = (n))
#define SETDOLLSYM(atom, s) ((atom)->a_type = A_DOLLSYM, \
    (atom)->a_w.w_symbol= (s))

EXTERN t_float atom_getfloat(t_atom *a);
EXTERN t_int atom_getint(t_atom *a);
EXTERN t_symbol *atom_getsymbol(t_atom *a);
EXTERN t_symbol *atom_gensym(t_atom *a);
EXTERN t_float atom_getfloatarg(int which, int argc, t_atom *argv);
EXTERN t_int atom_getintarg(int which, int argc, t_atom *argv);
EXTERN t_symbol *atom_getsymbolarg(int which, int argc, t_atom *argv);

EXTERN void atom_string(t_atom *a, char *buf, int bufsize);

/* ------------------  binbufs --------------- */

EXTERN t_binbuf *binbuf_new(void);
EXTERN void binbuf_free(t_binbuf *x);

EXTERN void binbuf_text(t_binbuf *x, char *text, size_t size);
EXTERN void binbuf_gettext(t_binbuf *x, char **bufp, int *lengthp);
EXTERN void binbuf_clear(t_binbuf *x);
EXTERN void binbuf_add(t_binbuf *x, int argc, t_atom *argv);
EXTERN void binbuf_addv(t_binbuf *x, char *fmt, ...);
EXTERN void binbuf_addbinbuf(t_binbuf *x, t_binbuf *y);
EXTERN void binbuf_restore(t_binbuf *x, int argc, t_atom *argv);
EXTERN void binbuf_print(t_binbuf *x);
EXTERN int binbuf_getnatom(t_binbuf *x);
EXTERN t_atom *binbuf_getvec(t_binbuf *x);
EXTERN void binbuf_eval(t_binbuf *x, t_pd *target, int argc, t_atom *argv);
EXTERN int binbuf_read(t_binbuf *b, char *filename, char *dirname);
EXTERN int binbuf_write(t_binbuf *x, char *filename, char *dir);
EXTERN void binbuf_evalfile(t_symbol *name, t_symbol *dir);

/* ------------------  clocks --------------- */

EXTERN t_clock *clock_new(void *owner, t_method fn);
EXTERN void clock_set(t_clock *x, double systime);
EXTERN void clock_delay(t_clock *x, double delaytime);
EXTERN void clock_unset(t_clock *x);
EXTERN double clock_getsystime(void);
EXTERN double clock_gettimesince(double prevsystime);
EXTERN double clock_getsystimeafter(double delaytime);
EXTERN void clock_free(t_clock *x);

/* ----------------- pure data ---------------- */
EXTERN t_pd *pd_new(t_class *cls);
EXTERN void pd_free(t_pd *x);
EXTERN void pd_bind(t_pd *x, t_symbol *s);
EXTERN void pd_unbind(t_pd *x, t_symbol *s);
EXTERN t_pd *pd_findbyclass(t_symbol *s, t_class *c);
EXTERN void pd_pushsym(t_pd *x);
EXTERN void pd_popsym(t_pd *x);
EXTERN t_symbol *pd_getfilename(void);
EXTERN t_symbol *pd_getdirname(void);
EXTERN void pd_bang(t_pd *x);
EXTERN void pd_pointer(t_pd *x, t_gpointer *gp);
EXTERN void pd_float(t_pd *x, t_float f);
EXTERN void pd_symbol(t_pd *x, t_symbol *s);
EXTERN void pd_list(t_pd *x, t_symbol *s, int argc, t_atom *argv);
EXTERN void pd_anything(t_pd *x, t_symbol *s, int argc, t_atom *argv);
#define pd_class(x) (*(x))

/* ----------------- pointers ---------------- */
void gpointer_init(t_gpointer *gp);
void gpointer_unset(t_gpointer *gp);
int gpointer_check(t_gpointer *gp, int headok);

/* ----------------- patchable "objects" -------------- */
EXTERN_STRUCT _inlet;
#define t_inlet struct _inlet
EXTERN_STRUCT _outlet;
#define t_outlet struct _outlet

EXTERN t_inlet *inlet_new(t_object *owner, t_pd *dest, t_symbol *s1,
    t_symbol *s2);
EXTERN t_inlet *pointerinlet_new(t_object *owner, t_gpointer *gp);
EXTERN t_inlet *floatinlet_new(t_object *owner, t_float *fp);
EXTERN t_inlet *symbolinlet_new(t_object *owner, t_symbol **sp);
EXTERN void inlet_free(t_inlet *x);

EXTERN t_outlet *outlet_new(t_object *owner, t_symbol *s);
EXTERN void outlet_bang(t_outlet *x);
EXTERN void outlet_pointer(t_outlet *x, t_gpointer *gp);
EXTERN void outlet_float(t_outlet *x, t_float f);
EXTERN void outlet_symbol(t_outlet *x, t_symbol *s);
EXTERN void outlet_list(t_outlet *x, t_symbol *s, int argc, t_atom *argv);
EXTERN void outlet_anything(t_outlet *x, t_symbol *s, int argc, t_atom *argv);
EXTERN void outlet_free(t_outlet *x);

/* -------------------- canvases -------------- */

EXTERN void glob_setfilename(void *dummy, t_symbol *name, t_symbol *dir);

EXTERN void canvas_logerror(t_object *x);
EXTERN void canvas_setargs(int argc, t_atom *argv);
EXTERN t_atom *canvas_getarg(int which);
EXTERN t_symbol *canvas_getcurrentdir(void);
EXTERN t_canvas *canvas_getcurrent(void);
EXTERN void canvas_makefilename(t_canvas *c, char *file,
    char *result, int resultsize);
t_symbol *canvas_getdir(t_canvas *x);

/* ---------------- widget behaviors ---------------------- */

EXTERN_STRUCT _widgetbehavior;
#define t_widgetbehavior struct _widgetbehavior

EXTERN_STRUCT _parentwidgetbehavior;
#define t_parentwidgetbehavior struct _parentwidgetbehavior
EXTERN t_parentwidgetbehavior *pd_getparentwidget(t_pd *x);

/* -------------------- classes -------------- */

#define CLASS_DEFAULT 0 	/* flags for new classes below */
#define CLASS_PD 1
#define CLASS_GOBJ 2
#define CLASS_PATCHABLE 3
#define CLASS_NOINLET 8

#define CLASS_TYPEMASK 3


EXTERN t_class *class_new(t_symbol *name, t_newmethod newmethod,
    t_method freemethod, size_t size, int flags, t_atomtype arg1, ...);
EXTERN void class_addcreator(t_newmethod newmethod, t_symbol *s, 
    t_atomtype type1, ...);
EXTERN void class_addmethod(t_class *c, t_method fn, t_symbol *sel,
    t_atomtype arg1, ...);
EXTERN void class_addbang(t_class *c, t_method fn);
EXTERN void class_addpointer(t_class *c, t_method fn);
EXTERN void class_doaddfloat(t_class *c, t_method fn);
#define class_addfloat(x, y) class_doaddfloat((x), (t_method)(y))
EXTERN void class_addsymbol(t_class *c, t_method fn);
EXTERN void class_addlist(t_class *c, t_method fn);
EXTERN void class_addanything(t_class *c, t_method fn);
EXTERN void class_setwidget(t_class *c, t_widgetbehavior *w);
EXTERN void class_setparentwidget(t_class *c, t_parentwidgetbehavior *w);
EXTERN t_parentwidgetbehavior *class_parentwidget(t_class *c);
EXTERN t_symbol *class_symbol(t_class *c);
EXTERN void class_setdrawcommand(t_class *c);
EXTERN int class_isdrawcommand(t_class *c);

/* ------------   printing --------------------------------- */
EXTERN void post(const char *fmt, ...);
EXTERN void startpost(const char *fmt, ...);
EXTERN void poststring(const char *s);
EXTERN void postatom(int argc, t_atom *argv);
EXTERN void endpost(void);
EXTERN void error(const char *fmt, ...);
EXTERN void bug(const char *fmt, ...);

EXTERN void sys_logerror(char *object, char *s);
EXTERN void sys_unixerror(char *object);
EXTERN void sys_ouch(void);

/* ------------  system interface routines ------------------- */
EXTERN int sys_isreadablefile(char *name);
EXTERN void sys_bashfilename(char *from, char *to);

/* --------------- signals ----------------------------------- */

typedef float t_sample;
#define MAXLOGSIG 13
#define MAXSIGSIZE (1 << MAXLOGSIG)

typedef struct _signal
{
    int s_n;
    t_sample *s_vec;
    float s_sr;
    int s_refcount;
    struct _signal *s_next;
    struct _signal *s_nextused;
} t_signal;

typedef t_int *(*t_perfroutine)(t_int *args);

EXTERN t_int *plus_perform(t_int *args);
EXTERN t_int *zero_perform(t_int *args);
EXTERN t_int *copy_perform(t_int *args);

EXTERN int sys_getblksize(void);
EXTERN float sys_getsr(void);
EXTERN int sys_getch(void);

EXTERN void dsp_add(t_perfroutine f, int n, ...);
EXTERN void pd_fft(float *buf, int npoints, int inverse);
EXTERN int ilog2(int n);

EXTERN void mayer_fht(float *fz, int n);
EXTERN void mayer_fft(int n, float *real, float *imag);
EXTERN void mayer_ifft(int n, float *real, float *imag);
EXTERN void mayer_realfft(int n, float *real);
EXTERN void mayer_realifft(int n, float *real);

EXTERN float *cos_table;
#define LOGCOSTABSIZE 9
#define COSTABSIZE (1<<LOGCOSTABSIZE)

int canvas_suspend_dsp(void);
void canvas_resume_dsp(int oldstate);
void canvas_update_dsp(void);

/* ----------------------- utility functions for signals -------------- */
EXTERN float mtof(float);
EXTERN float ftom(float);
EXTERN float rmstodb(float);
EXTERN float powtodb(float);
EXTERN float dbtorms(float);
EXTERN float dbtopow(float);

EXTERN float sgicompatible_qsqrt(float); //;;;;
EXTERN float qrsqrt(float);

/* --------------------- data --------------------------------- */

    /* graphical arrays */
EXTERN_STRUCT _garray;
#define t_garray struct _garray

EXTERN t_class *garray_class;
EXTERN int garray_getfloatarray(t_garray *x, int *size, t_float **vec);
EXTERN float garray_get(t_garray *x, t_symbol *s, t_int index);
EXTERN void garray_redraw(t_garray *x);
EXTERN int garray_npoints(t_garray *x);
EXTERN char *garray_vec(t_garray *x);

EXTERN t_class *scalar_class;

/* ------- GUI interface - functions to send strings to TK --------- */
void sys_vgui(char *fmt, ...);
void sys_gui(char *s);

/*-------------  Max 0.26 compatibility --------------------*/

/* the following reflects the new way classes are laid out, with the class
   pointing to the messlist and not vice versa. Externs shouldn't feel it. */
typedef t_class *t_externclass;

EXTERN void c_extern(t_externclass *cls, t_newmethod newroutine,
    t_method freeroutine, t_symbol *name, size_t size, int tiny, \
    t_atomtype arg1, ...);
EXTERN void c_addmess(t_method fn, t_symbol *sel, t_atomtype arg1, ...);

#define t_getbytes getbytes
#define t_freebytes freebytes
#define t_resizebytes resizebytes
#define typedmess pd_typedmess

#ifdef _LANGUAGE_C_PLUS_PLUS
}
#endif
