#ifndef _SM_H_
#define _SM_H_

#include "VActor.h"
#include "VHandler.h"
#include <deque>

#define MAX_NODE 100
#define MAX_TERM 5
#define MAX_EDGE 5
#define MAX_DOOR 5


struct Node;

struct EdgeList
{
  Node * node;
  float time;
  float tp; // transition probability
};

struct Terminal
{
  float time;
  float tmdur;
  int numEdge; // number of outgoing edges
  EdgeList edge[MAX_EDGE]; // list of edges
};

struct Node
{
  int name; // node ID
  int soundfile; // soundfile associated
  int numTerm; // number of terminals
  float start; // start time
  float stop;  // end time
  float dur;   // stop-start
  Terminal term[MAX_TERM];
};

struct Door
{
  bool active;
  bool inited;
  bool ended;
  float range; // ???
  float startTime;
  float elapsTime;
  float accumTime;
  Node * doorNode; // the node at which the door is
};


class smActor : public VActor 
{
 public:
  smActor(void);
  virtual ~smActor() {}
  
  void act(void);
  virtual int receiveMessage(const char * Message);

  void setMessageGroup(char* name)
    { strncpy(szMG, name, sizeof(szMG)-2); }

  void setDirectory(const char* dirname);

  void setPresetFile(char * prefile);

  void setDoorNum(const int Num);

  void initDoor(const int Num, const int iNode);

  void triggerDoor(const int iDoor);

  void setRange(const int iDoor, const float range);

  void setTimelimit(const float timelimit)
    { TIME_NoEvent = timelimit; }

  void setMinRepeatTime(const float mintime)
    { MinRepeatTime = mintime; }

  void setCoupling(const int coup) 
    { coupling = bool(coup); }


 private:

  char szMG[100];
  char szFilename[180];
  bool initGraph;
  bool coupling;
  bool whileInit;
  int doorNum;
  int InitDoorNode[MAX_DOOR];
  int MaxNodeNum;
  float TIME_NoEvent; //???
  float MinRepeatTime; //??? 

  Node node[MAX_NODE];
  Door door[MAX_DOOR];
  Door* MostRecentDoor;
};

#endif
