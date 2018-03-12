#ifndef _testActor_h
#define _testActor_h

#include "VActor.h"

/* A TestActor will eventually include functionality to support
   automated testing of DSO.
 */
class TestActor : public VActor
{
public:
  typedef VActor baseClass;
protected:
  
public:
  TestActor();
  ~TestActor();

  //	actor behavior
  virtual	int receiveMessage(const char*);
};

#endif /* _testActor_h */
