/* Copyright (c) 1997 The Regents of the University of California.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_imp.h"

void g_canvas_setup(void);
void g_graph_setup(void);
void g_text_setup(void);
void g_array_setup(void);
void g_io_setup(void);
void g_scalar_setup(void);
void g_template_setup(void);
void g_traversal_setup(void);
void bindlist_setup(void);
void x_acoustics_setup(void);
void x_interface_setup(void);
void x_connective_setup(void);
void x_time_setup(void);
void x_arithmetic_setup(void);
void x_midi_setup(void);
void x_misc_setup(void);
void x_net_setup(void);
void x_qlist_setup(void);
void d_dac_setup(void);
void d_ctl_setup(void);
void d_arithmetic_setup(void);
void d_osc_setup(void);
void d_filter_setup(void);
void d_math_setup(void);
void d_misc_setup(void);
void d_array_setup(void);
void d_global_setup(void);
void d_delay_setup(void);
void d_ugen_setup(void);
void d_fft_setup(void);

void conf_init(void)
{
//;;;; I hope we can get by with incomplete initialization.
//  g_canvas_setup();
//  g_graph_setup();
//  g_text_setup();
//  g_array_setup();
//  g_io_setup();
//  g_scalar_setup();
//  g_template_setup();
//  g_traversal_setup();
//  bindlist_setup();
//  x_acoustics_setup();
//  x_interface_setup();
//  x_connective_setup();
//  x_time_setup();
//  x_arithmetic_setup();
//  x_midi_setup();
//  x_misc_setup();
//  x_net_setup();
//  x_qlist_setup();
//  d_dac_setup();
//  d_ctl_setup();
//  d_arithmetic_setup(); // maybe this one?
//  d_osc_setup();
//  d_filter_setup();
//  d_math_setup();
//  d_misc_setup();
//  d_array_setup();
//  d_global_setup();
//  d_delay_setup();
//  d_ugen_setup();
    d_fft_setup();
}

