#include "sm.h"
ACTOR_SETUP(smActor, SmActor)

smActor::smActor():
  VActor(),
  initGraph(false),
  coupling(false),
  whileInit(true),
  doorNum(1),
  MaxNodeNum(1),
  TIME_NoEvent(100.),
  MinRepeatTime(0.)
{
  *szMG = '\0';
  memcpy(szFilename, "./\0", 3);
  setTypeName("SmActor");
}

int smActor::receiveMessage(const char* Message)
{
  CommandFromMessage(Message);
	
  if (CommandIs("setMessageGroup"))
    {
      ifS( name, setMessageGroup(name) );
      return Uncatch();
    }

  if (CommandIs("setDirectory"))
    {
      ifS( s, setDirectory(s) );
      return Uncatch();
    }

  if (CommandIs("setPresetFile"))
    {
	ifS( s, setPresetFile(s) );
	return Uncatch();
    }

  if (CommandIs("initDoor"))
    {
	ifDD( d, d1, initDoor(d,d1) );
	return Uncatch();
    }

  if (CommandIs("setDoorNum"))
    {
	ifD( d, setDoorNum(d) );
	return Uncatch();
    }

  if (CommandIs("triggerDoor"))
    {
	ifD( d, triggerDoor(d) );
	return Uncatch();
    }
  if (CommandIs("setRange"))
    {
      ifDF( d, f, setRange(d,f) );
      return Uncatch();
    }

  if (CommandIs("setTimelimit"))
    {
      ifF( f, setTimelimit(f) );
      return Uncatch();
    }

  if (CommandIs("setMinRepeatTime"))
    {
      ifF( f, setMinRepeatTime(f) );
      return Uncatch();
    }

  if (CommandIs("setCoupling"))
    {
      ifD( d, setCoupling(d) );
      return Uncatch();
    }

  return VActor::receiveMessage(Message);
}

void smActor::setDirectory(const char* dirname)
{
  strncpy(szFilename, dirname, strlen(dirname)+1);
  printf("directory = %s\n", szFilename);
}

void smActor::setPresetFile(char * prefile)
{
  whileInit = true;
  int i;
  for (i=0; i<MAX_DOOR; i++)
    door[i].inited = false;
  {
    const char* temp = strrchr(szFilename, '/');
    const int dirLen = strlen(szFilename) - (strlen(temp)-1);
    memcpy(szFilename+dirLen, prefile, strlen(prefile)+1);
    printf("filename = %s\n", szFilename);
  }

  ifstream inFile(szFilename, ios::in);
  if (!inFile)
    {
      printf("Error in opening preset file %s\n",szFilename);
      initGraph = false;
      return;
    }

  char Line[60];	
  int NodeId, NxNodeId, fileName;
  float NodeStart, NodeEnd, TmTime, NxNodeTime, tp;
  int NodeIndex = -1;
  int TermIndex = 0;
  int EdgeIndex = 0;

  while( !inFile.eof() )
    {
      inFile.getline(Line,60);
      // printf("%s",Line);
      if( strlen(Line) == 0 ) continue;
      if( 1 == sscanf(Line,"End") ) break;

      // Catch a New Node
      if (4 == sscanf(Line,"%d\t%d\t%f\t%f",&NodeId, &fileName, &NodeStart, &NodeEnd))
	{
	  ++NodeIndex;
	  TermIndex=-1;
	  node[NodeIndex].name=NodeId;
	  if (NodeId>MaxNodeNum) MaxNodeNum=NodeId;
	  node[NodeIndex].soundfile=fileName;
	  //printf("%d\t%d\t%f\t%f\n",NodeId, node[NodeIndex].soundfile, NodeStart,NodeEnd);
	  //printf("Soundfile now is %d\n",node[NodeIndex].soundfile);
	  node[NodeIndex].numTerm=0;
	  node[NodeIndex].start=NodeStart;
	  node[NodeIndex].stop=NodeEnd;
	  node[NodeIndex].dur = node[NodeIndex].stop - node[NodeIndex].start;
	  continue;
	}	

      //Catch a New Terminal & a New Edge
      if (4==sscanf(Line,"\t%f\t(%d\t%f\t%f",&TmTime, &NxNodeId,&NxNodeTime,&tp))
	{
	  //printf("teminal postion(time)  = %f\n", TmTime);
	  EdgeIndex=0;
	  ++TermIndex;
	  node[NodeIndex].term[TermIndex].time=TmTime;
	  node[NodeIndex].term[TermIndex].tmdur=TmTime-node[NodeIndex].start;
	  ++node[NodeIndex].numTerm;
	  //printf("NxNodeId=%d\tNxNodeTime=%f\ttp=%f\n",NxNodeId, NxNodeTime,tp);
	  node[NodeIndex].term[TermIndex].edge[EdgeIndex].node=&node[NxNodeId-1];
	  node[NodeIndex].term[TermIndex].edge[EdgeIndex].time=NxNodeTime;
	  node[NodeIndex].term[TermIndex].edge[EdgeIndex].tp=tp;
	  node[NodeIndex].term[TermIndex].numEdge=1;
	  continue;
	}

      if (3==sscanf(Line,"\t%f\t(%d\t%f",&TmTime, &NxNodeId,&NxNodeTime))
	{
	  //printf("teminal postion(time)  = %f\n", TmTime);
	  //printf("NxNodeId=%d\tNxNodeTime=%f\ttp=%f\n",NxNodeId, NxNodeTime,1.);
	  ++TermIndex;
	  ++node[NodeIndex].numTerm;
	  EdgeIndex=0;
	  node[NodeIndex].term[TermIndex].numEdge=1;
	  node[NodeIndex].term[TermIndex].time=TmTime;
	  node[NodeIndex].term[TermIndex].tmdur=TmTime-node[NodeIndex].start;
	  node[NodeIndex].term[TermIndex].edge[EdgeIndex].node=&node[NxNodeId-1];
	  node[NodeIndex].term[TermIndex].edge[EdgeIndex].time=NxNodeTime;
	  node[NodeIndex].term[TermIndex].edge[EdgeIndex].tp=1;
	  node[NodeIndex].term[TermIndex].numEdge=1;
	  continue;
	}

      //Catch a new Edge
      if (3==sscanf(Line,"\t\t(%d\t%f\t%f", &NxNodeId,&NxNodeTime,&tp))
	{
	  //printf("NxNodeId=%d\tNxNodeTime=%f\ttp=%f\n",NxNodeId, NxNodeTime,tp);
	  ++EdgeIndex;
	  ++node[NodeIndex].term[TermIndex].numEdge;
	  node[NodeIndex].term[TermIndex].edge[EdgeIndex].node=&node[NxNodeId-1];
	  node[NodeIndex].term[TermIndex].edge[EdgeIndex].time=NxNodeTime;
	  node[NodeIndex].term[TermIndex].edge[EdgeIndex].tp=tp;
	  continue;
	}

      if (2==sscanf(Line,"\t\t(%d\t%f",&NxNodeId,&NxNodeTime))
	{
	  //printf("NxNodeId=%d\tNxNodeTime=%f\ttp=%f\n",NxNodeId, NxNodeTime,1.);
	  ++EdgeIndex;
	  ++node[NodeIndex].term[TermIndex].numEdge;
	  node[NodeIndex].term[TermIndex].edge[EdgeIndex].node=&node[NxNodeId-1];
 	  node[NodeIndex].term[TermIndex].edge[EdgeIndex].time=NxNodeTime;
	  for (int pp=0; pp<=EdgeIndex; pp++)
	    {
	      node[NodeIndex].term[TermIndex].edge[pp].tp=float(1./(EdgeIndex+1));
	    }
	  continue;
	}
    }
  inFile.close();

  printf("//////////////////Read In//////////////////////////\n");
  for (i=0; i<=NodeIndex; i++)
    {
      int numTerm = node[i].numTerm;
      printf("Node %d from soundfile %d, start %.3f, stop %.3f, dur %.3f\n",
	     node[i].name,node[i].soundfile,node[i].start,node[i].stop,node[i].dur);
      for (int j=0; j<numTerm; j++)
	for (int k=0; k<node[i].term[j].numEdge; k++)
	  printf("\tEdge from term %d dur %.3f at %.3f to node %d time %.3f with p=%.3f\n",
		 j,
		 node[i].term[j].tmdur,
		 node[i].term[j].time,
		 node[i].term[j].edge[k].node->name,
		 node[i].term[j].edge[k].time,
		 node[i].term[j].edge[k].tp);
      printf("\n\n");
    }
  initGraph = true;
}

void smActor::setDoorNum(const int Num)
{
  doorNum = Num;
  printf("We'll deal with %d doors.\n", Num);
}

void
smActor::initDoor(const int Num, const int iNode)
{
  if (!initGraph)
    {
      printf("Can't initDoor! Preset file not set yet!\n");
      return;
    }

  if (Num>doorNum)
    {
      printf("invalid doorNum!!!");
      return;
    }

  if (iNode<1 || iNode>MaxNodeNum)
    {
      printf("Wrong Node Number,  out of range!!!");
      return;
    }
  door[Num].doorNode=&node[iNode-1];
  door[Num].active = 0;
  // door[Num]=new Door(iNode);

  door[Num].startTime=currentTime();
  door[Num].elapsTime=0.;
  door[Num].accumTime=0.;
  door[Num].ended=0;
  door[Num].inited=1;
  door[Num].range=1;
  InitDoorNode[Num]=iNode;

  whileInit = false;
  for (int i=0; i<doorNum; i++)
    if (door[i].inited == 0)
      whileInit = true;
  printf("Door %d initialized to Node %d\n", Num, iNode);
  /*
    printf("Node %d: start %.1f, stop %.1f, dur %.1f\n",
    door[Num].doorNode->name, door[Num].doorNode->start, door[Num].doorNode->stop, door[Num].doorNode->dur);

    for (int j=0; j<door[Num].doorNode->numTerm; j++)
    for (int k=0; k<door[Num].doorNode->term[j].numEdge; k++)
    printf("\tedge from term %d to node %d time %.1f with p=%.1f\n",
    j, door[Num].doorNode->term[j].edge[k].node->name,  door[Num].doorNode->term[j].edge[k].time, door[Num].doorNode->term[j].edge[k].tp);
  */
}

void smActor::setRange(const int iDoor, const float rang)
{
  door[iDoor].range=rang;
  printf("set Door %d range to %f\n", iDoor, rang);
}

void smActor::triggerDoor(const int iDoor)
{
  if (!door[iDoor].inited)
    {
      printf("This door hasn't been initialized!\n");
      return;
    }
  if (door[iDoor].ended)
    {
      printf("This door has reached its end state!");
      return;
    }
  char sz[256];
  if(!door[iDoor].active)
    {
      door[iDoor].active=1;
      printf("Door %d activated\n", iDoor);
      sprintf(sz, "SendData %s [%d %d %d %.3f %.3f]", szMG, iDoor, door[iDoor].doorNode->name, door[iDoor].doorNode->soundfile, door[iDoor].doorNode->start, door[iDoor].doorNode->dur );  //playback from the very beginning of the soundfile.
      printf("SendData %s [Door:%d Node:%d Soundfile:%d Start:%.3f Dur:%.3f]\n", szMG, iDoor, door[iDoor].doorNode->name, door[iDoor].doorNode->soundfile,  door[iDoor].doorNode->start, door[iDoor].doorNode->dur);
      actorMessageHandler(sz);
    }
  else
    {
      //printf("Door %d begins transition\n",iDoor);
      door[iDoor].elapsTime = currentTime() - door[iDoor].startTime;
      if (door[iDoor].elapsTime < MinRepeatTime) return;

      //printf("elapseTime = %.3f\n",door[iDoor].elapsTime);
      for (int it=(door[iDoor].doorNode->numTerm-1); it>=0; it--)
	{
	  //  printf("Term %d Dur %.3f\n",it,door[iDoor].doorNode->term[it].tmdur);
	  if (door[iDoor].elapsTime >= door[iDoor].doorNode->term[it].tmdur) // state transition may take place at this terminal
	    {
	      door[iDoor].accumTime += door[iDoor].elapsTime;
	      Terminal * currentTerm = &(door[iDoor].doorNode->term[it]);
	      int numEdge = currentTerm->numEdge;
	      int OriNodeId = door[iDoor].doorNode->name;
	      if ( numEdge == 0 ) // no outlet
		{
		  printf("Door %d reached end state %d at time %.3f %.3f\n", iDoor, door[iDoor].doorNode->name, door[iDoor].accumTime, door[iDoor].elapsTime);
		  door[iDoor].ended = 1;
		}
	      else if ( numEdge == 1 ) // no multiple outlets
		{
		  door[iDoor].doorNode=currentTerm->edge[0].node;
		  sprintf(sz, "SendData %s [%d %d %d %.3f %.3f]", szMG, iDoor, door[iDoor].doorNode->name, door[iDoor].doorNode->soundfile, currentTerm->edge[0].time, currentTerm->edge[0].node->dur);
		  printf("SendData %s [Door:%d Node:%d Soundfile:%d Start:%.3f Dur:%.3f]\n", szMG, iDoor, door[iDoor].doorNode->name, door[iDoor].doorNode->soundfile, currentTerm->edge[0].time, currentTerm->edge[0].node->dur);
		  actorMessageHandler(sz);
		}
	      else // multiple outlets: need to use transition probability
		{
		  float atp[numEdge]; // accumulative transition probability
		  atp[0] = currentTerm->edge[0].tp;
		  // if (atp[0]<door[iDoor].range) atp[0]=0.;
		  int i;
		  for (i=1; i<numEdge; i++)
		    {
		      // if (currentTerm->edge[i].tp>range)
		      atp[i] = atp[i-1] + currentTerm->edge[i].tp;
		    }
		  int choice = 0; // is it OK to initialize with this?
		  float temp = float(drand48());
		  for (i=0; i<numEdge; i++)
		    if ( temp < atp[i] )
		      {
			choice = i;
			break;
		      }
		  door[iDoor].doorNode=currentTerm->edge[choice].node;
		  sprintf(sz, "SendData %s [%d %d %d %.3f %.3f]", szMG, iDoor,door[iDoor].doorNode->name, door[iDoor].doorNode->soundfile, currentTerm->edge[choice].time, currentTerm->edge[choice].node->dur);
		  printf("SendData %s [Door:%d Node:%d Soundfile:%d Start:%.3f Dur:%.3f]\n", szMG, iDoor, door[iDoor].doorNode->name, door[iDoor].doorNode->soundfile, currentTerm->edge[choice].time, currentTerm->edge[choice].node->dur);
		  actorMessageHandler(sz);
		}

	      printf("Door %d transition from Node %d to Node %d which start at %.3f dur %.3f\n", iDoor, OriNodeId, door[iDoor].doorNode->name, door[iDoor].doorNode->start, door[iDoor].doorNode->dur);
	      door[iDoor].startTime = currentTime();
	      break;
	    }  // if
	}  // for (int it=0; it<door[iDoor].doorNode->numTerm; it++)
    }  // else
}

void smActor::act(void)
{
  VActor::act();
  if (whileInit) return;
  //printf("MostRecentdoorNode=%d\n", MostRecentDoor->doorNode->name);
  for (int id=0; id<doorNum; id++)
    {
      if (!door[id].active)
	{
	  door[id].startTime=currentTime();
	  if (coupling &&(MostRecentDoor!=NULL) )
	    {
	      for (int it=(MostRecentDoor->doorNode->numTerm-1); it>=0; it--)
		{
		  if (MostRecentDoor->elapsTime >= MostRecentDoor->doorNode->term[it].tmdur)
		    // inherit may take place at this terminal
		    {
		      Terminal * currentTerm = &(MostRecentDoor->doorNode->term[it]);
		      int numEdge = currentTerm->numEdge;
		      int OriNodeId = MostRecentDoor->doorNode->name;
		      if ( numEdge == 0 ) // no outlet
			{
			  printf("Can't couple to the most recent active Door node, which has reached end state (node %d)\n", OriNodeId);
			  break;
			}
		      else if ( numEdge == 1 ) // no multiple outlets
			{
			  door[id].doorNode=currentTerm->edge[0].node;
			}
		      else // multiple outlets: need to use transition probability
			{
			  float atp[numEdge]; // accumulative transition probability
			  atp[0] = currentTerm->edge[0].tp;
			  // if (atp[0]<MostRecentDoor->range) atp[0]=0.;
			  for (int i=1; i<numEdge; i++)
			    {
			      // if (currentTerm->edge[i].tp>range)
			      atp[i] = atp[i-1] + currentTerm->edge[i].tp;
			    }
			  int choice = 0;
			  float temp = float(drand48());
			  for (int j=0; j<numEdge; j++)
			    if ( temp < atp[j] )
			      {
				choice = j;
				break;
			      }
			  door[id].doorNode=currentTerm->edge[choice].node;
			}
		      //printf("Door %d inherit from Node %d to %d, in coupling mode\n", id, OriNodeId, door[id].doorNode->name);
		      break;
		    }  // if
		}  // for (int it=(MostRecentDoor->doorNode->numTerm-1); it>=0; it--)
	    }  // if coupling ...
	} // if (!door[id].active)

      else
	{
	  door[id].elapsTime = currentTime() - door[id].startTime;
	  door[id].accumTime += door[id].elapsTime;
	  if (door[id].elapsTime > (door[id].doorNode->dur + TIME_NoEvent))
	    {
	      // If the soundfile is finished and no further trigger after TIME_NoEvent,
	      // reset the door to inactive.
	      door[id].active=0;
	      printf("Door %d becomes inactive\n",id);
	    }
	  if (door[id].ended && (door[id].elapsTime > (door[id].doorNode->dur+TIME_NoEvent)))
	    {
	      printf("Door %d reset to Node %d\n", id, InitDoorNode[id]);
	      initDoor(id,InitDoorNode[id]);
	    }
	}
    }  // for (int id=0; id<doorNum; id++)
}
