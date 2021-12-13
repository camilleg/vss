/* -*-C++-*-
  This file is part of a set of general purpose libraries to extend C++,
  by Vijendra Jaswal.
  Email: v-jaswal@uiuc.edu

  This file contains classes to help allocate/deallocate memory in
  fixed size blocks and from shared memory.

  Class cxArena used to be SharedMemArena which was lifted most
  recently from John Shalf.

  NOTE:: this is currently a grab bag of different memory allocation
  strategies.  These can be consolidated into a few classes later.

  Define common memory behavior classes.
   */

#pragma once
#ifndef VSS_MAC
#include <malloc.h>
#endif

class cxBaseMB
{
public:
  /* need protocol to try to obtain more memory when out-of-mem,
     and to handle/signal failure when out-of-mem. */

  // These are currently UNUSED and the semantics are illdefined.
  // These are only approximate stubs for future functions.
  // <group>
  // INTENT: Give up and signal that I'm out of memory.  Normally 0 means
  // that memory couldn't be obtained.
  static void* signalOutOfMem(size_t /*sz*/)    { return 0; }
  
  // INTENT: getMem should be called when alloc or allocObj returns
  // 0.  
  static void* retryAlloc(size_t sz)	    { return signalOutOfMem(sz); }
  // </group>
};

class cxHeapMB : public cxBaseMB
{
public:
  static void* alloc(const size_t size)
	    { return ::malloc(size); }
  static void  free(void* p, const size_t /*size*/)
	    { ::free(p); }
  static void* allocObj(const size_t size)
	    { return alloc(size); }
  static void freeObj(void* p, const size_t size)
	    { free(p,size); }
};

/*
  This uses new/delete for allocation
 */
class cxNewDeleteMB : public cxBaseMB
{
public:
  static void* alloc(const size_t size)
	    { return ::operator new(size); }
  static void  free(void* p, const size_t /*size*/)
	    { ::operator delete(p); }
  static void* allocObj(const size_t size)
	    { return alloc(size); }
  static void freeObj(void* p, const size_t size)
	    { free(p,size); }
};

#if 0
/*
  Memory behavior class, that forwards block allocations/deallocations
  to global functions, which are easier to redefine.
 */
template <class T>
class cxBlockMB
{
public:
  static void* alloc(const size_t sz)
  {
    T* p;
    // of type cxAllocBlock(T*& dst, size_t sz);
    cxAllocBlock(p,sz);
    return p;
  }
  static void free(void* p, const size_t sz) 
  {
    cxFreeBlock(p, sz);
  }
};

/* default impl is to use global operator new.
   Any class may override, by overloading.
 */
void cxAllocBlock(void*& p, size_t sz)
{
  p = ::operator new(sz);
}

#endif
