/* Copyright (c) 1997 The Regents of the University of California.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* This file contains function prototypes and data types used to implement
Pd, but not shared with Pd objects. */

#include "m_pd.h"

EXTERN_STRUCT _outconnect;
#define t_outconnect struct _outconnect

/* LATER consider whether to use 'char' for method arg types to save space */

/* the structure for a method handler ala Max */
typedef struct _methodentry
{
    t_symbol *me_name;
    t_gotfn me_fun;
    t_atomtype me_arg[MAXPDARG+1];
} t_methodentry;

EXTERN_STRUCT _widgetbehavior;

typedef void (*t_bangmethod)(t_pd *x);
typedef void (*t_pointermethod)(t_pd *x, t_gpointer *gp);
typedef void (*t_floatmethod)(t_pd *x, t_float f);
typedef void (*t_symbolmethod)(t_pd *x, t_symbol *s);
typedef void (*t_listmethod)(t_pd *x, t_symbol *s, int argc, t_atom *argv);
typedef void (*t_anymethod)(t_pd *x, t_symbol *s, int argc, t_atom *argv);

struct _class
{
    t_symbol *c_name;
    size_t c_size;
    t_methodentry *c_methods;
    int c_nmethod;
    t_freemethod c_freemethod;
    t_bangmethod c_bangmethod;
    t_pointermethod c_pointermethod;
    t_floatmethod c_floatmethod;
    t_symbolmethod c_symbolmethod;
    t_listmethod c_listmethod;
    t_anymethod c_anymethod;
    struct _widgetbehavior *c_wb; 	/* only filled in for "gobjs" */
    struct _parentwidgetbehavior *c_pwb;/* widget behavior in parent */
    char c_gobj;	    		/* true if is a gobj */
    char c_patchable;	    	/* true if has slots for inlets and outlets */
    char c_firstin; 	    	/* for patchables; true if draw first inlet */
    char c_drawcommand; 	/* a drawing command for a template */
};

/* s_file.c */
extern int sys_debuglevel;
extern int sys_verbose;
#define DEBUG_MESSUP 1	    /* messages up from pd to pd-gui */
#define DEBUG_MESSDOWN 2    /* messages down from pd-gui to pd */

/* in s_main.c */
int sys_nearestfontsize(int fontsize);
int sys_fontwidth(int fontsize);
int sys_fontheight(int fontsize);

extern t_symbol *sys_progdir;	/* directory pd was found in */
extern float sys_dacsr;
extern int sys_schedadvance;
extern int sys_defaultfont;

/* s_loader.c */
int sys_loademup(char *filename, char *dirname);

/* s_unix.c */
typedef void (*t_sighandler)();
int sys_signal(int signo, t_sighandler sigfun);

void sys_microsleep(int microsec);
#ifdef NT
#include <wtypes.h>
typedef LARGE_INTEGER t_systime;
#else
typedef struct systime
{
    int st_sec;
    int st_microsec;
} t_systime;
#endif

void sys_gettime(t_systime *tv);
double sys_microsecsince(t_systime *tv);
double sys_secsince(t_systime *tv);

/* s_sgi.c, s_nt.c, s_linux.c */
#define MAXCH 4
#define DACBLKSIZE 64

extern t_sample sys_soundout[MAXCH*DACBLKSIZE];
extern t_sample sys_soundin[MAXCH*DACBLKSIZE];

void sys_set_dacs(int chans, int rate);
void sys_close_audio(void);
int sys_send_dacs(void);
int sys_initdacs(void);
void sys_poll_midi(void);
int sys_init_sysdep(char *config, int srate);
void sys_audiobuf(int nbufs);

#ifdef NT
void nt_listdevs(void);
void nt_soundindev(int which);
void nt_soundoutdev(int which);
void nt_midiindev(int which);
void nt_midioutdev(int which);
#endif

/* s_inter.c */

void sys_bail(void);
int sys_pollgui(void);

EXTERN_STRUCT _socketreceiver;
#define t_socketreceiver struct _socketreceiver

t_socketreceiver *socketreceiver_new(void);
void socketreceiver_free(t_socketreceiver *x);
void socketreceiver_read(t_socketreceiver *x, int fd);
void sys_sockerror(char *s);
void sys_closesocket(int fd);

typedef void (*t_fdpollfn)(void *ptr, int fd);
void sys_addpollfn(int fd, t_fdpollfn fn, void *ptr);
void sys_rmpollfn(int fd);

/* m_obj.c */
t_object *pd_checkobject(t_pd *x);

int obj_noutlets(t_object *x);
int obj_ninlets(t_object *x);
t_outconnect *obj_starttraverseoutlet(t_object *x, t_outlet **op, int nout);
t_outconnect *obj_nexttraverseoutlet(t_outconnect *lastconnect,
    t_object **destp, t_inlet **inletp, int *whichp);
t_outconnect *obj_connect(t_object *source, int outno,
    t_object *sink, int inno);
void obj_disconnect(t_object *source, int outno, t_object *sink, int inno);

/* misc */
void glob_evalfile(t_pd *ignore, t_symbol *name, t_symbol *dir);
