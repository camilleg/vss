#include "mapActor.h"

//	set sound source
void PlaneMapActor::setRegistry(int id, float x, float y, int mg)
{
  if ( mg >= 0 ) // new registry
    {
      if (pmReg[id].valid)
	printf("PlaneMapper warning: registry %d already set. Discard old message group\n",id);
      pmReg[id].id = id;
      pmReg[id].mg = mg;
      pmReg[id].x = x;
      pmReg[id].y = y;
      pmReg[id].valid = true;
      printf("Registry %d: %.1f, %.1f, %d\n", id, pmReg[id].x, pmReg[id].y, pmReg[id].mg);
    }
  else // update
    {
      pmReg[id].x = x;
      pmReg[id].y = y;
    }
  doit(pmReg[id].x, pmReg[id].y, pmReg[id].mg);
}

// set VR position
void PlaneMapActor::setPosition(float px, float py, float pt)
{
  ux = px;
  uy = py;
  ut = pt;
  for (int i=0; i<MAX_REG; i++)
    {
      if (pmReg[i].valid)
	doit(pmReg[i].x, pmReg[i].y, pmReg[i].mg);
    }
}

// calculate azimuth and distance and send out
void PlaneMapActor::doit(float sx, float sy, int mg)
{
  float st, d;
  float dx = sx-ux, dy = sy-uy;
  if ( dx==0 && dy==0 ) // coincide
    { 
      d = 0.; 
      st = 0.; 
    }
  else if ( dy == 0 ) // horizontal
    { 
      d = abs(dx); 
      st = (ux>sx) ? 90. : 270.; 
    }
  else if ( dx == 0 ) // vertical
    { 
      d = abs(dy); 
      st = (uy>sy) ? 180. : 0.; }
  else
    {
      st = atan( dy/dx ) * 57.295788;
      d = sqrt( dx*dx + dy*dy );
      if ( dx < 0. ) // sound is in quadrant 2 or 3
	st += 180.;
      st -= 90.; // because azimuth counts from Y+ axis, not X+ axis
    }
  st -= ut; 

//    printf("s: %.2f %.2f %.2f, u: %.2f %.2f %.2f, d %.2f %.2f %.2f\n",
//  	 sx,sy,st,ux,uy,ut,d,dx,dy);
  char msg[100];
  sprintf(msg, "SendData %d [%f %f]", mg, st, d);
  actorMessageHandler(msg);
}

int PlaneMapActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetRegistry"))
	{
		ifDFFD(id,x,y,mg, setRegistry(id,x,y,mg) );
		ifDFF(id,x,y, setRegistry(id,x,y,-1) ); // update
		return Uncatch();
	}
	
	if (CommandIs("SetPosition"))
	{
		ifFFF(x,y,t, setPosition(x,y,t) );
		ifFF(x,y, setPosition(x,y,ut) );
		return Uncatch();
	}
	
	if (CommandIs("SetOrientation"))
	{
		ifF(t, setPosition(ux,uy,t) );
		return Uncatch();
	}
	
	return MapActor::receiveMessage(Message);
}
