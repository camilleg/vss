/* -*-C++-*-

   !!! NOTE: this version has been hacked up to work with SwitchActor.
   
  This file is part of a set of general purpose libraries to extend C++,
  by Vijendra Jaswal.
  Email: v-jaswal@uiuc.edu

  2003 note: probably better to use std::vector by now.

   array template classes.

   for every "cxArrayB<T,AB>" need to define
   cxaCallDestructor(T) or cxaCallDestructor(T&) to call
   the destructor T::~T() since the Delta C++ is buggy.

   so for every cxArrayB<T,AB> include
   CXA_destructor(T) in the header file.
  
   MACRO PARAMETERS FOR THIS FILE
   
   _CC_NO_INLINE_LOOPS	-> 1 if compiler does NOT inline loops,
			   0 (default) if inlining of loops is done.

  BUGS:
   */
#ifndef _cxArray_h
#define _cxArray_h

#define SA_HACK 1		// 1 to use the hacks for SwitchActor.

#ifndef _CC_NO_INLINE_LOOPS
#define _CC_NO_INLINE_LOOPS	0
#endif
#undef _cxArray_INLINE
#if _CC_NO_INLINE_LOOPS
#   define _cxArray_INLINE   inline
#else
#   define _cxArray_INLINE   /* null */
#endif

/* Utility macro to define a function that will call the destructor
   for a given object.  This is needed because of a bug in DCC
   templates that cannot properly instantiate T::~T() with the type
   T.  Therefore, this also cannot be made into a template.
 */
#define CXA_destructor(T) inline void cxaCallDestructor(T& obj) { obj.~T(); }

/* macro to define various additional things for an array of a given type,
   and behavior */
#define CXA_Array(T,B)	 CXA_destructor(T)
/* macro to create the typedefs with L and LI suffixes. */
#define CXA_ArrayL(T,B,Base)		\
    typedef cxArrayB<T,B> Base##L;	\
    typedef cxArrayIterB<T,B> Base##LI

/* Macro similar to CXA_Array, except this also creates
   typedefs for the array and iterator types by append L and LI. */
#define CXA_ArrayT(T,B,TypeDefBase)		\
    CXA_ArrayL(T,B,TypeDefBase);		\
    CXA_Array(T,B)

using namespace std;
#include <new>		// for operator new(void*)
#include <iostream>		// for ostream stuff
#include <cstring>		// need memmove
#include <cassert>
#ifdef _WIN32
#define __assert(TEST, FILE, LINE)  _assert(TEST, FILE, LINE)
#endif
#include "cxMem.h"

// <summary> Private auxiliary class for cxArrayB.  </summary>  struct
// cxVoidp is a dummy class that exists solely to provide a distinct
// type so that a particular instance of new can be called.
struct cxVoidp
{ void* p;
  cxVoidp(void* q) : p(q) {}
};
inline void* operator new(size_t /*size*/, cxVoidp vp) { return vp.p; }

/* internal use macros, to replace array new and delete so that
   any memory can be used.
   use "CXA_new(ArrayBehavior,T,size)"		to replace "new T[size]"
   use "CXA_delete(ArrayBehavior,T,size,data)"	to replace "delete [] data"
 */
//#define CXA_new(AB,T,count) new(struct cxVoidp((void*)AB::alloc(sizeof(T)*count))) T[count]

#define CXA_new(AB,T,count)  new ((void*)(AB::alloc( sizeof(T)*count))) T[count]

#define CXA_delete(AB,data,T,count) \
    { destructArray(data,count); AB::free(data,count*sizeof(T)); }
#define CXA_Assert(test) ((test)? ((void)0): \
    AB::assertionFailed( # test , __FILE__, __LINE__))
      
#define cxAE(exp)	/* debug && exp */
//extern int debug;

/*
    Do element by element copy from array starting at src to dst, for
    elements starting at start, up to, but not including end.
*/
template <class T>
inline void cxAssignRange(T* dst, T* src, int start, int end)
{
  for (int i=start; i<end; i++) {
    dst[i] = src[i];
  }
}

template <class T>
inline void cxBitCopyRange(T* dst, T* src, int start, int end)
{
  memmove(dst, src, sizeof(T)*(end-start));
}


//----------------------------------------------------------------------
//	class array
//		type safe and access safe array data type
//		permits dynamic modification of array length
//----------------------------------------------------------------------

/* Default array behavior:
   - calls normal assert.
   - call destructors for deleted vectors.
   - use assignment operator to copy elements.
 */
class cxBaseAB
{
  int dummy;
public:
  cxBaseAB()	{}
  // this function should return non-zero if the destructor should be called.
  // when old data arrays are deleted, or when ~cxArrayB is called.
#if 1
  static int  destroyArray()		{ return 1; }
#else
  static const int  destroyArray()	{ return 1; }
#endif
  // static int destroyArray(cxArrayB<T,AB>&)
  
  // this function should return non-zero if the assignment operator
  // should be used to copy elements between old and new arrays.
  // Other bitwise copy (using mem moves) will be used.
#if 1
  static int  assignToCopy()		{ return 1; }
#else
  static const int  assignToCopy()	{ return 1; }
#endif
  
  static void	assertionFailed(char* test, char* file, int line)
	{
#ifdef VSS_IRIX
	__assert(test,file,line);
#endif
	}

  
};
class cxHeapAB :      public cxBaseAB, public cxHeapMB		{ };
class cxNewDeleteAB : public cxBaseAB, public cxNewDeleteMB	{ };
typedef cxNewDeleteAB cxAB;
typedef cxNewDeleteAB cxDefaultAB;

/*--------------------------cxArrayB----------------------------------*/
template <class T,class AB>
class cxArrayB
{
protected:
  // data areas
  T*           data;
  // Total capacity which is size of member variable data.
  unsigned int  size;
  // Current number of elements in this array.
  unsigned int 	numElements;	
  
public:
  // constructors and destructor
  cxArrayB(unsigned int initSize = 10);
  cxArrayB(unsigned int initSize, T initialValue);
  // Copy constructor that deep copies values in array.
  cxArrayB(const cxArrayB<T,AB> & source);
  ~cxArrayB();

  // Access to elements via subscript, with range checking.
  T&           operator[] (unsigned int index) const
    { CXA_Assert(index < numElements); return data[index]; }

  // Assignment
  cxArrayB<T,AB>&   operator= (const cxArrayB<T,AB> &);

  // Return size of data array.  This returns the total capacity of
  // the array.
  unsigned int	capacity() const	    { return size; }
  
  // Return direct pointer to data array.
  T*	_data()				    { return data; }
  // Directly access element at index, without checking array bounds
  // and without resizing.
  T&	_at(unsigned int index)	const	    { return data[index]; }

  // Dynamically change size of array to exactly newSize.  This only
  // changes the capacity of the array, not the count.  If newSize==0,
  // then the memory of data array is also discarded.  Return the new
  // capacity.
  unsigned int	setCapacity(unsigned int newSize);
  // NOT IMPL: Like setCapacity, but does not abort when new data
  // array could not be obtained.  Returns the new size, which is ==newSize
  // if successful.
  unsigned int	trySetCapacity(unsigned int newSize);
  
  // Access element at index and expand array if necessary, if index is
  // beyond the capacity.
  T&  dynamicAt(unsigned int index);
  // Access element at index and expand array if necessary.
  T&  at(unsigned int index) 	{ return dynamicAt(index); }
  // Answer the number of elements in array
  int  count() const	        { return numElements; }
  // Set the number of elements in array.  This adjusts the count of
  // the number of elements, and expands the capacity (using expandTo(int))
  // if needed.  <note> Destructors are not called on elements that are
  // out-of-bounds, ie. between numElements and size. Use truncateAt(int)
  // for that.</note>
  void count(int newCount);
  // Set the count to newCount and erase all the elements after it with
  // the default constructor.  Or name it countTrunc, or truncCount.
  // Add extendAtWith(int,const T&) ?.
  void truncateAt(int newCount)	{ truncateAtWith(newCount,T()); }
  // UNTESTED.  Set the count to newCount and erase all the elements
  // after it with eraseElm.  Use this to set the new count and to call
  // destructors on the out-of-bounds elements.
  void truncateAtWith(int newCount, const T& eraseVal)
  { fill(eraseVal, newCount, count()); count(newCount); }
  // Add elm to end of array and extend count.
  void add(const T& elm)	{ dynamicAt(numElements) = elm; }
  //# void add(T elm)	{ dynamicAt(numElements) = elm; }
  // remove last element, w/o checks
  void _pop()           { --numElements; }
  // remove all elements
  void clear()		{ numElements = 0; }
  // Empty the array, and overwrite old elements with eraseVal.
  void clearWith(const T& eraseVal)	{ fill(eraseVal); clear(); }
  // Remove first element that is equal to elm (by operator==).
  // Return the index of the removed element or -1 if not found.
  int  removeFirst(const T& elm);
  // return last element, array must be non-empty
  T& last()		{ return data[numElements-1]; }
  
  // Copy elements in src starting from index srcStart, up to but NOT
  // INCLUDING srcEnd, into self starting at myStart.  Thus elements
  // start to end-1 are copied inclusively.  Boundaries of the indexes
  // are not checked. Return one plus index of the last element
  // copied, (i.e. end).  Use atReplaceFromTo(...) to check the boundaries.
  int _atReplaceFromTo(int myStart,
		       const cxArrayB<T,AB>& src, unsigned int srcStart, unsigned int srcEnd);

  // Same as above but bounds checking on the indices are done.
  int atReplaceFromTo(int myStart,
		      const cxArrayB<T,AB>& src, unsigned int srcStart, unsigned int srcEnd);
  // Same as _atReplaceFromTo(...) but copy data in from T*.
  int _atReplaceFromTo(int myStart,
		       const T* p, unsigned int srcStart, unsigned int srcEnd);
  // Same as above but bounds checking on the indices are done.
  int atReplaceFromTo(int myStart,
		       const T* p, unsigned int srcStart, unsigned int srcEnd);

  // add or delete elements,
  // numToShift is negative to delete, pos to insert space
  void shift(int idx, int numToShift);
  // Insert elements into array at idx.  Old values are not erased.
  void atInsert(int idx, int numToIns=1)   { shift(idx, numToIns); }
  // Delete numToDel elements 
  void atDelete(int idx, int numToDel=1)   { shift(idx, -numToDel); }
  // Like atDeleteWith, but additionally copy elm over the elements
  // past numElements. (This copies elm over the previously numToDel
  // last elements.
  void atDeleteWith(int idx, int numToDel, const T& fillVal);

  // Delete element at idx, and return it.
  T    atExtractElm(int idx)
    { T tmp = at(idx); atDelete(idx); return tmp; }
  // Insert element elm into array at idx.
  void atInsertElm(int idx, const T& elm)
    { atInsert(idx); _at(idx) = elm; }
  // UNTESTED: insert a numToIns copies of elm starting at idx.
  void atInsertElm(int idx, const T& elm, int numToIns)
    { atInsert(idx,numToIns); fill(elm, idx, idx+numToIns); }

  // Fill a range of elements i..j-1 with the val fillVal.  NOTE: that
  // the upperbound is one PAST the last indexable element.  If j is
  // -1, then set all elements from i to the last element to val.
  // This method will expand both the capacity and count of the array,
  // if j extends beyond the number of elements or size of the data
  // space.  Thus all filled elements will be legally accessable.
  // Only j can extend the range.  If i>=j and i>=count, then the
  // range is not extended, and no modifications are made.  <src>
  // fill(aT,0,-1); to write aT over all current elements. </src>
  // However, if i<j, and i>count, then j will extend the range,
  // and then i..j-1 will be filled with fillVal.
  void fill(const T& fillVal, int i=0, int j=-1);

  // Comparison functions.  
  // <group>
  // NOTE: for the comparison functions, to treat array as a sorted
  // list, with duplicates added before first occurrence, use <.
  // e.g. int cmpL(const int& a, const int& b) { return a < b; }
  // To add duplicates after last element, use <=.
  // e.g. int cmpLE(const int& a, const int& b) { return a <= b; }
  // Use > and >= respectively to sort in descending order.
  // </group>
  // This typedef defines the type of the comparison function.
  // It is used by the sorted search methods and by find.
  typedef int (*BinCmpFN)(const T& a,const T& b);

  // Search the array, to find the correct location for an item,
  // assuming it is ordered by some comparison function.  Start the
  // search at idx, and find the first location when cmp(a[i], val) is
  // NOT TRUE (i.e. returns 0), where idx <= i < count().  <note> This
  // version is inline and does not check the index values, or if the
  // data array itself is empty. </note> PREC: 0 <= idx < numElements,
  // otherwise results are undefined.  Basically, answer an index, idx, s.t.
  // 0 <= idx <= count(), where val should be inserted into the sorted list.
  inline int _linearSearchSorted(const T val, BinCmpFN cmp, int idx=0);
  // non-inline version.  Bounds are NOT checked.
  _cxArray_INLINE
  int linearSearchSorted(const T& val, BinCmpFN cmp, int idx=0);
  // Search the array in reverse from the end position, where the end is
  // 1 PAST the first element to check.  i.e. to search from the very end,
  // use endIdx==count().  if endIdx < 0 then start from last element.
  // Search backward until cmp(a[i],val) is TRUE.  Bounds are NOT checked.
  inline int _linearSearchSortedRev(const T val, BinCmpFN cmp, int endIdx=-1);
  // non-inline version, Bounds are NOT checked.
  _cxArray_INLINE
  int linearSearchSortedRev(const T& val, BinCmpFN cmp, int endIdx=-1);

  // Search the array, same as linearSearchSorted and linearSearchSortedRev,
  // but choose the correction direction automatically.  Idx must be
  // valid array index, i.e. 0 <= idx < count().  Bounds are NOT checked.
  inline int _linearSearchSortedBi(const T val, BinCmpFN cmp, int idx=0);
  // non-inline version, Bounds are NOT checked.
  _cxArray_INLINE
  int linearSearchSortedBi(const T& val, BinCmpFN cmp, int idx=0);

  // Do sorted insertion (searching forward only), searching starting
  // from idx.  If array is sorted before the insertion, then the
  // array will also be sorted afterwards.
  int insertSorted(const T& elm, BinCmpFN cmp, int idx=0);

#if !SA_HACK
  // HACK inline function.  Returns a==b.
  static inline int opEq(const T& a, const T& b);
  
  // Search for searchVal in array starting at location startIdx and
  // up to but NOT including endIdx, until equal returns true.  Answer
  // the index, or endIdx if searchVal was not found.  If endIdx==-1,
  // then this->count() is used (and this->count() is returned if
  // searchVal was not found).  Otherwise, bounds checking is not
  // done.  (Segmentation faults could occur if indices are not
  // correct.)  PREC: if endIdx != -1, then startIdx <= endIdx should
  // be true, otherwise startIdx will be returned if searchVal is not
  // found.
  // BASICALLY: for a return value, returnIdx, if startIdx <= returnIdx < endIdx
  // is true,
  // then returnIdx is the location of searchVal.
  // Otherwise returnIdx will be the greater of startIdx and endIdx, which
  // should be endIdx in normal cases.
  inline int _findEqual(const T searchVal, BinCmpFN equal,
		   int startIdx=0, int endIdx=-1);
  // Non-inline versions.
  _cxArray_INLINE
  inline int findEqual(const T& searchVal, BinCmpFN equal,
			int startIdx=0, int endIdx=-1);

  // Identical to above but operator== is used for the comparison.
  // This is named after the eq test in Lisp which does a bitwise comparison,
  // like operator== does by default.
  inline int _findEq(const T searchVal, int startIdx=0, int endIdx=-1);
  // Non-inline version.
  _cxArray_INLINE
  inline int findEq(const T& searchVal, int startIdx=0, int endIdx=-1);

  // Answer true if searchVal occurs in array.  This simply delegates to findEq.
  inline int isMember(const T& searchVal);
#endif

  // Increase the current capacity until it is equal to or greater
  // than minSize.  
  void	expandTo(unsigned int minSize);

  static void	destructArray(T* array,const size_t sz);

  // uninline this
  void	dumpAll();
  void	dump();
  
  ostream& dumpOn(ostream& os, unsigned int printCount=10);
};


/*------------------------------------------------------------
  This function will call the destructors for every element
  of the array.
 */
//template <class T> void destructArray(T* array,const size_t sz)
template <class T,class AB> void cxArrayB<T,AB>::destructArray(T* array,const size_t sz)
{
  if (AB::destroyArray()) {
    for (int i=sz-1; i >= 0; --i)
      {
	// explicitly call destructors for each element
	//array[i].T::~T();  this works with g++
	//array[i].destroy(); //ONLY this works with DCC. also with g++
#if 1
	array[i].T::~T();
	//cxaCallDestructor(array[i]);
#else
	cxaCallDestructor(array[i]);
#endif
      };
  }
}

//----------------------------------------------------------------------
//	class cxArrayIterB
//		iterator protocol used to loop over array elements
//----------------------------------------------------------------------

/* Usage: forward iteration
   for (cxArrayIterB<X,AB> vi(anX); !vi; ++vi)
   {
      x = vi();			// return current value with bounds check
      x = *vi;			// return withouth check index
      anX[vi.key()];		// return index of current value
      vi.ref() = newVal;	// return reference to allow modify
      vi._ref() = newVal;	// return ref w/o checking bounds
   }

   // iterate in reverse, start index -1 will wrap around to end
   for (cxArrayIterB<X,AB> vi(anX,-1); !vi.notLow(); --vi)
   { }
   
   // initialize iterator starting at any index
   cxArrayIterB<X,AB> vi(anX,startIndex);

   */

template <class T,class AB> class cxArrayIterB
{
protected:
  // data fields
  int		    currentKey;
  cxArrayB<T,AB>&   vec;
    
public:
  // constructor
  inline cxArrayIterB(cxArrayB<T,AB>& array);
  // copy constructor
  inline cxArrayIterB(const cxArrayIterB& array);
  // iterator starting for a certain index
  cxArrayIterB(cxArrayB<T,AB>&, int startIdx);
    
  // assignment operator
  cxArrayIterB& operator=(const cxArrayIterB& vi)
	{ currentKey = vi.currentKey; vec = vi.vec; return *this; }

  // Iteration protocol
  // <group>

  // Reinitialize an iterator with any array.
  int init(cxArrayB<T,AB>& array)	{ vec = array; return init(); }
  // Reinitialize an iterator with any array at any index.
  int init(cxArrayB<T,AB>& array, int startIdx);
  
  // Reinitialize iterator to start.  Answer true if iterator is in
  // range.  This does not need to be called, unless an iterator
  // specifically needs to be reset to the beginning of the array.
  int  init()			{ currentKey = 0; return operator!(); }
  
  // Used to initialize at any index.
  void initIdx(int startIdx);
  
  // Increment the iterator to next array element.
  int  operator++()		{ currentKey++; return operator!(); }
  // Increment iterator by inc elements. inc must be positive or else
  // test will fail
  int  operator+=(int inc)	{ currentKey += inc; return operator!(); }
  // Answer true if iterator is still valid.
  int  operator!()		{ return currentKey < vec.count(); }
  // </group>
  
  // Array value accessors.  These must only be called when
  // this::operator!() returns true.
  // <group>
  
  // Return array element, with bounds checking.
  T&    operator()()	      	{ return vec[currentKey]; }
  // Direct reference access to element, without bounds checking
  T&	operator*()		{ return vec._at(currentKey); }
  // Return array element with copy by value.
  T	val()	    	    	{ return vec[currentKey]; }
  // Return array element as reference.
  T&	ref()	    	    	{ return vec[currentKey]; }
  // direct access reference to element
  //T&	 _ref()	    	    	{ return vec[currentKey]; }
  int	key()	    		{ return currentKey; }
  // </group>

  //#T    operator()()	      	{ return vec[currentKey]; }
  // use for reverse iteration, to test for start
  int  notLow()		{ return currentKey >= 0; }
  // return 1 if index is inbounds
  int  in()	    { return (0 <= currentKey && currentKey < vec.count()); }
  T& operator=(const T& newValue)
	{ vec[currentKey] = newValue; return vec[currentKey]; }

  // new methods specific to array iterators
  /* these decrementing iterators only work if used with
     notLow() to test for the low boundary */
  inline int	operator--()
  {
    if (currentKey >= 0)
      currentKey--;
    return operator!();	// meaningful?, or use in() ?
  }
  // !!! UNTESTED
  inline int	operator-=(int dec)
  {
    if (currentKey >= 0)
      currentKey -= dec;
    return operator !();	// meaningful?, or use in() ? 
  }

  // return array pointer
  cxArrayB<T,AB> *_vecp()	    	    	{ return &vec; }
  void    dumpAll()
  {
    fprintf(stderr, "cxArrayIterB: this=%x\t currentKey:%d\tvec:",
	    this, currentKey);
    if (_vecp())
      vec.dumpAll();
    else
      fprintf(stderr, "\a*** NULL ***\a\n");
  }
};





//----------------------------------------------------------------------
//	class cxArray implementation
//----------------------------------------------------------------------

template <class T,class AB> cxArrayB<T,AB>::cxArrayB(unsigned int initSize)
{
  numElements = 0;
  size = 0; data = 0;
  setCapacity(initSize);
}


template <class T,class AB> cxArrayB<T,AB>::cxArrayB(unsigned int initSize,
    T initialValue)
{
  numElements = 0;
  size = 0; data = 0;
  setCapacity(initSize);
  
  // set each element to the initial value
  // use assignToCopy() ??
  for (unsigned int i = 0; i < size; i++)
    data[i] = initialValue;
}

// Copy constructor. Deep copy another array.
template <class T,class AB> cxArrayB<T,AB>::cxArrayB(const cxArrayB<T,AB> & source)
    : size(source.size), numElements(source.numElements)
{
    // create and initialize a new array
    // allocate the space for the elements

    data = CXA_new(AB,T,size);

    CXA_Assert(data != 0);

    // copy values from old array
    // use assignToCopy()
    for (unsigned int i = 0; i < size; i++)
	data[i] = source.data[i];
}



template <class T,class AB> cxArrayB<T,AB>::~cxArrayB()
{
    // free the dynamic memory buffer
    CXA_delete(AB,data,T,size);
    //data = 0;
}


template <class T,class AB> cxArrayB<T,AB> & cxArrayB<T,AB>::operator=
    (const cxArrayB<T,AB> & right)
{
    // match sizes
    if (size != right.size)
	setCapacity(right.size);

    numElements = right.numElements;

    // copy the elements
    // if (assignToCopy()) 
    for (unsigned int i = 0; i < right.size; i++)
	data[i] = right.data[i];

    // return current value
    return *this;
}



#if 1				// attempting trySetCapacity
// Set the capacity of array to exactly newSize.  Create new array
// if the new size is different from the requested size, and copy
// all the old elements to the new array.  If the new array cannot
// be allocated, then simply return with the current size but do not
// abort using assert.  If successful, return with the new size.
template <class T, class AB>
unsigned int cxArrayB<T,AB>::trySetCapacity(unsigned int newSize)
{
  cxAE(fprintf(stderr,"\nsetCapacity(%d)\n", newSize));
  // dynamically alter the size of the array
  if (size == newSize)
    return size;

  // first create the new data area
  T* newData = 0;
  if (newSize > 0) {
    
#if 1
    newData = CXA_new(AB, T, newSize);
#else
    cxVoidp mp = AB::alloc(sizeof(T)*newSize);
    newData = new(mp) T[newSize];
#endif
    
    if (newData==0) {
      return size;
      // !!! Change this? to some out-of-mem check or error handler?
      //CXA_Assert(newData != 0);
    }
  }

  // Copy old data if needed.
  if (data != 0)
    {
      unsigned int minSize = (newSize <= size) ? newSize : size;
      // Use assignToCopy??  COPY ONLY  (i < numElements) ??
      for (unsigned int i = 0; i < minSize; i++)
	newData[i] = data[i];
      
      // delete the old data buffer
      CXA_delete(AB,data,T,size);
      if (numElements > newSize)
	numElements = newSize;
    }
  else {
    // if old array was empty, then so is new one.
    numElements = 0;
  }
  // update the data member fields
  size = newSize;
  data = newData;

  cxAE(fprintf(stderr,"(size:%d numElements:%d)\n", size, numElements));
  // return new size
  return size;
}
// Set the capacity requested and return the new size.  If the
// capacity could not be set (from lack of memory), then abort using
// assert.
template <class T,class AB>
unsigned int cxArrayB<T,AB>::setCapacity(unsigned int newSize)
{
  if (trySetCapacity(newSize) < newSize) {
    CXA_Assert(size != newSize);
  }
  return size;
}


#else
// Set the capacity of array to exactly newSize.  Create new array
// if the new size is different from the requested size, and copy
// all the old elements to the new array.
template <class T,class AB>
unsigned int cxArrayB<T,AB>::setCapacity(unsigned int newSize)
{
  cxAE(fprintf(stderr,"\nsetCapacity(%d)\n", newSize));
  // dynamically alter the size of the array
  if (size == newSize)
    return size;

  // first create the new data area
  T* newData = 0;
  if (newSize > 0) {
    newData = CXA_new(AB,T,newSize);
    if (newData==0) {
      // !!! Change this? to some out-of-mem check or error handler?
      CXA_Assert(newData != 0);
    }
  }

  // Copy old data if needed.
  if (data != 0)
    {
      unsigned int minSize = (newSize <= size) ? newSize : size;
      // Use assignToCopy??  COPY ONLY  (i < numElements) ??
      for (int i = 0; i < minSize; i++)
	newData[i] = data[i];
      
      // delete the old data buffer
      CXA_delete(AB,data,T,size);
      if (numElements > newSize)
	numElements = newSize;
    }
  else {
    // if old array was empty, then so is new one.
    numElements = 0;
  }
  // update the data member fields
  size = newSize;
  data = newData;

  cxAE(fprintf(stderr,"(size:%d numElements:%d)\n", size, numElements));
  // return new size
  return size;
}
#endif



// Set the count and expand the capacity of the array if needed.
template <class T,class AB> void cxArrayB<T,AB>::count(int newCount)
{
  if ((unsigned)newCount >= size) {
    expandTo(newCount); }
  numElements = newCount;
}


// This increases the array capacity, if needed, by successively doubling
// the current size, until it meets or exceeds the minSize.
template <class T,class AB> void cxArrayB<T,AB>::expandTo(unsigned int minSize)
{
  unsigned sz = (size >= 1) ? size : 1;
  while (sz < minSize) {
    sz *= 2;
  }
  cxAE(fprintf(stderr,"Expanding from:%d to %d\n", size, sz));
  setCapacity(sz);
}

//----------------------------------------
// access element and expand array if necessary.
template <class T,class AB> T &  cxArrayB<T,AB>::dynamicAt(unsigned int index)
{
  if (index >= numElements)
    {
      cxAE(fprintf(stderr,"\ndynamicAt:overindexed @%d with only %d elements.  Need to expand...\n",
	    index, numElements));
      // grow array if necessary
      if (index >= size)
	{
	  expandTo(index+1);	// NOTE this changes data, size and numElements
	}
      // update number of elements when indexed past end.
      numElements = index+1;
      cxAE(fprintf(stderr,"dynamicAt: using size:%d numElements:%d\n", size, numElements));
    } 
  return data[index];
}

/*
   Shift elements of the array.
   idx is point of insertion or deletion.
   numToShift is pos for inserting, neg for deleting.
   */
template <class T,class AB> void cxArrayB<T,AB>::shift(int idx, int numToShift)
{
  // use assignToCopy() or something.
  // insert 
  if (numToShift > 0)
    {
      // expand array if necessary to allow shift
      if (numElements + numToShift >= size) {
	expandTo(numElements + numToShift); }
      // shift elements to right
      for (int i = numElements-1; i >= idx; i--) {
	data[i + numToShift] = data[i]; }
    }
  // delete elements
  else // numToShift is 0 or neg
    {
      // adjust numToShift to be in range
      if (idx + -numToShift > (signed)numElements) {
	numToShift = idx - numElements; }
      if (numToShift < 0)   {
         const int maxIdx = (signed)numElements - -numToShift;
	for (int i = idx; i < maxIdx; i++) {
	  data[i] = data[i + -numToShift];  }
      }
    }
  numElements += numToShift;
}


/*
  Fill a range of positions with a value.
  j points one PAST the last index to be filled.
  So the elements i..(j-1) are filled.

  If j is negative, then just fill to current end, of numElements.
 */
template <class T,class AB>
void cxArrayB<T,AB>::fill(const T& fillVal, int i, int j)
{
  if (j < 0)
    j = numElements;
  if ((unsigned)j > size)
    expandTo(j);
  for (; i<j; i++) 
    data[i] = fillVal;
  if ((unsigned)j > numElements)
    numElements = j;
}

template <class T,class AB>
void cxArrayB<T,AB>::atDeleteWith(int idx, int numToDel, const T& fillVal)
{
  shift(idx, -numToDel);
  int j = numElements + numToDel;
  for (int i=numElements; i<j; i++)
    data[i] = fillVal;
}


//extern void *memmove(void *, const void *, size_t sz);

/* Copy elements from src, from indexes srcStart to srcEnd-1,
   into self starting at myStart.

   The boundaries are not checked.
  */
template <class T,class AB>
int cxArrayB<T,AB>::_atReplaceFromTo(int myStart,
				   const cxArrayB<T,AB>& src,
				   unsigned int srcStart, unsigned int srcEnd)
{
#if SA_HACK
    for (unsigned int j=srcStart; j<srcEnd; j++, myStart++) {
      data[myStart] = src.data[j];
    }
#else
  if (AB::assignToCopy()) {
    for (unsigned int j=srcStart; j<srcEnd; j++, myStart++) {
      data[myStart] = src.data[j];
    }
  }
  else {
    // Use bit wise copy
    memmove(&data[myStart], &src.data[srcStart], sizeof(T)*(srcEnd-srcStart));
  }
#endif
  return srcEnd;
}

template <class T,class AB>
int cxArrayB<T,AB>::atReplaceFromTo(int myStart,
				   const cxArrayB<T,AB>& src,
				   unsigned int srcStart, unsigned int srcEnd)
{
  if (myStart+(srcEnd-srcStart) <= numElements &&
      srcEnd <= src.numElements)
    return _atReplaceFromTo(myStart, src, srcStart, srcEnd);
  else
    return srcStart;
}

//------------------------------------------------------------
template <class T,class AB>
int cxArrayB<T,AB>::_atReplaceFromTo(int myStart,
				     const T* p,
				     unsigned int srcStart, unsigned int srcEnd)
{
#if SA_HACK
    for (unsigned int j=srcStart; j<srcEnd; j++, myStart++) {
      data[myStart] = p[j];
    }
#else
  if (AB::assignToCopy()) {
    for (unsigned int j=srcStart; j<srcEnd; j++, myStart++) {
      data[myStart] = p[j];
    }
  }
  else {
    // Use bit wise copy
    memmove(&data[myStart], &p[srcStart], sizeof(T)*(srcEnd-srcStart));
  }
#endif
  return srcEnd;
}

template <class T,class AB>
int cxArrayB<T,AB>::atReplaceFromTo(int myStart,
				    const T* p, unsigned int srcStart, unsigned int srcEnd)
{
  if (myStart+(srcEnd-srcStart) <= numElements)
    return _atReplaceFromTo(myStart, p, srcStart, srcEnd);
  else
    return srcStart;
}

/*------------------------------------------------------------
  Output operations.
 */
template <class T,class AB> ostream& operator<<(ostream& os, cxArrayB<T,AB>& A)
{
  //os << "#" << A.numElements << " ";
  os << "{#" << A.count() << ": ";
  for (cxArrayIterB<T,AB> it(A); !it; ++it)
    { os << *it << " "; }
  os << "}";
  return os;
}

template <class T,class AB> ostream& cxArrayB<T,AB>::dumpOn(ostream& os, unsigned int printCount)
{
  printCount = (printCount > numElements) ? numElements : printCount;
  /* os << form("Array::this: &%x capacity: %d  data: &%x {#%d: ",
	     this, size, data, numElements); */
  os << "Array::this: &" << (void*)this << " capacity: " << size
     << "  data: &" << (void*)data << " {#" << numElements << ": ";
  for (unsigned int i=0; i<printCount; i++) 
    { os << data[i] << " "; }
  os << "...}";
  return os;
}


template <class T,class AB>
void cxArrayB<T,AB>::dumpAll()
{
  fprintf(stderr, "this: &%x size: %d, numElements: %d data: &%x <",
	  this, size, numElements, data);
  for (unsigned int i=0; i<size; i++) {
    fprintf(stderr, "[%d]: %x, ", i, data[i]); }
  fprintf(stderr, ">\n");
}

template <class T,class AB>
void cxArrayB<T,AB>::dump()
{
  fprintf(stderr, "this: &%x size: %d, numElements: %d data: &%x\n",
	  this, size, numElements, data);
}


#if !SA_HACK
/*
  Search forward from startIdx, up to and not including endIdx,
  for searchVal, that evaluates to true using function equal.
  Answer the index of the element or else this->count() if its not
  found.
 */
template <class T,class AB>
int cxArrayB<T,AB>::_findEqual(const T searchVal, BinCmpFN equal,
			  int startIdx, int endIdx)
{
  if (endIdx==-1)
    endIdx = numElements;
  for (register int i=startIdx; i < endIdx && !equal(searchVal,data[i]); ++i)
    ;
  return i;
}

template <class T,class AB>
int cxArrayB<T,AB>::findEqual(const T& searchVal, BinCmpFN equal,
			  int startIdx, int endIdx)
{
  return _findEqual(searchVal, equal, startIdx, endIdx);
}

template <class T,class AB>
inline int cxArrayB<T,AB>::opEq(const T& a, const T& b)
{
  return a==b;
}   

template <class T,class AB>
int cxArrayB<T,AB>::_findEq(const T searchVal, int startIdx, int endIdx)
{
  return _findEqual(searchVal, opEq, startIdx, endIdx);
}   

template <class T,class AB>
int cxArrayB<T,AB>::findEq(const T& searchVal, int startIdx, int endIdx)
{
  return _findEq(searchVal, startIdx, endIdx);
}

/* Answer true if search value occurs in array, using the operator== test */
template <class T,class AB>
int cxArrayB<T,AB>::isMember(const T& searchVal)
{
  return _findEq(searchVal) < numElements;
}

/* Find the first occurence in the array that is operator== to
   elm and remove it.
   Return the location of the element if it was found.
   return -1 if not found.
   */
template <class T,class AB>
int cxArrayB<T,AB>::removeFirst(const T& elm)
{
  int delPoint = -1;
  for (cxArrayIterB<T,AB> it(*this); !it; ++it)
    {
      if (it() == elm)		// g++ cannot do this, so take explicit operator==, later
	{
	  delPoint = it.key();
	  break;
	}
    }
  if (delPoint >= 0)
    atDelete(delPoint);
  
  return delPoint;
}
#endif



//----------------------------------------------------------------------
//	class cxArrayIterB implementation
//----------------------------------------------------------------------

// normal constructor
template <class T,class AB> inline cxArrayIterB<T,AB>::cxArrayIterB(cxArrayB<T,AB> & v)
    : vec(v)
{
    // create and initialize a array iterator
    init();
}

// copy constructor
template <class T,class AB> inline cxArrayIterB<T,AB>::
	cxArrayIterB(const cxArrayIterB<T,AB> & x)
		: vec(x.vec), currentKey(x.currentKey)
{
	// no further initialization
}

/* iterate starting at an arbitrary index.
   If negative, then index from end,
   so -1 is last element, -2 is second from last, etc.
   If index is too large, then it will wrap around as needed.
   */
template <class T,class AB> cxArrayIterB<T,AB>::cxArrayIterB(cxArrayB<T,AB> & v,
							     int startIdx)
  : vec(v)
{
  int count = vec.count();
  initIdx(startIdx);
}

template <class T,class AB> int cxArrayIterB<T,AB>::init(cxArrayB<T,AB>& array,
							 int startIdx)
{
  vec = array;
  initIdx(startIdx);
  return operator!();
}

template <class T,class AB> void cxArrayIterB<T,AB>::initIdx(int startIdx)
{
  //vec = v;
  int count = vec.count();
  if (startIdx >= 0)
    {
      if (startIdx < count)
	currentKey = startIdx;
      else {
	// wrap index around, startIdx is zero if numElements is 0
	currentKey = startIdx % (count ? count : 1);
      }
    }
  else
    {
      if (-startIdx <= count)
	currentKey = count + startIdx;
      else {
	currentKey =
	  count - ((-startIdx-1) % (count ? count : 1)) -1;
      }
    }
}


// *******************************************************
//	template functions 
//	Cfront demands these are in a different file,
//	g++ and borland want them in this file.
// *******************************************************

# ifdef __GNUG__

//# include <array.c>

# endif

# ifndef __GNUG__

template <class VecType, class EleType>
int binarySearch(VecType data, EleType ele, unsigned int max);

template <class T,class AB> void swap( cxArrayB<T,AB> & data, int i, int j);

template <class T,class AB> void bubbleSort(cxArrayB<T,AB> & data);

template <class T,class AB> void selectionSort(cxArrayB<T,AB> & data);

template <class T,class AB> void insertionSort(cxArrayB<T,AB> & data);

template <class T,class AB>
int partition(cxArrayB<T,AB> & v, int low, int high, int pivotIndex);

template <class T,class AB>
T findElement(cxArrayB<T,AB> & v, int N, int low, int high);

template <class T,class AB>
void quackSort(cxArrayB<T,AB> & v, int low, int high);

template <class T,class AB>
void quackSort(cxArrayB<T,AB> & v);

# endif


/* instantiation of cxArray with allocation from the heap.
   This will probably be most common instantiation.
   */
template <class T>
class cxArray : public cxArrayB<T,cxDefaultAB>
{
public:
  typedef cxArrayB<T,cxDefaultAB> base;
  cxArray(unsigned int initSize = 10)
    : base(initSize) {}
  cxArray(unsigned int initSize, T initialValue)
    : base(initSize, initialValue) {}
  cxArray(const cxArray<T> & source)
    : base(source) {}
};

template <class T>
class cxArrayIter : public cxArrayIterB<T,cxDefaultAB>
{
public:
  typedef cxArrayIterB<T,cxDefaultAB> base;
  
  // constructor
  cxArrayIter(cxArray<T>& anArray)
    : base(anArray) {}
  // copy constructor
  cxArrayIter(const cxArrayIter &aVI)
    : base(aVI)	{}
  // iterator starting for a certain index
  cxArrayIter(cxArray<T>& anArray, int startIdx)
    : base(anArray, startIdx) {}
};

#endif /* _cxArray_h */
