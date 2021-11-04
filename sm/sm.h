#pragma once
#include "VActor.h"
#include "VHandler.h"
#include <deque>

// State machine.

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
  smActor();
  ~smActor() {}
  
  void act();
  int receiveMessage(const char*);

  void setMessageGroup(const char* name) { strncpy(szMG, name, sizeof(szMG)-2); }
  void setDirectory(const char* dirname);
  void setPresetFile(char* prefile);
  void setDoorNum(int Num);
  void initDoor(int Num, int iNode);
  void triggerDoor(int iDoor);
  void setRange(int iDoor, float range);
  void setTimelimit(float timelimit) { TIME_NoEvent = timelimit; }
  void setMinRepeatTime(float mintime) { MinRepeatTime = mintime; }
  void setCoupling(int coup) { coupling = bool(coup); }

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
