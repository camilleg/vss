#ifdef VSS_IRIX

/* This works in C, fails in C++.  (Link errors occur.) */

/*
 *  Set the special "flush zero" bit (FS, bit 24) in the Control Status
 *  Register of the FPU of R4k and beyond so that the result of any
 *  underflowing operation will be clamped to zero, and no exception of
 *  any kind will be generated on the CPU.  This has no effect on an R3000.
 *
 *  The FS bit is inherited by processes fork()ed out of this one,
 *  but it is not inherited across an exec().  So anytime you exec()
 *  a process, you must re-set the FS bit in that process.
 */

#include <sys/fpu.h>

extern void flushme_(void)
{
    union fpc_csr f;
    f.fc_word = get_fpc_csr();
    f.fc_struct.flush = 1;
    set_fpc_csr(f.fc_word);
}

#endif
