#include "crowdActor.h"

extern const char* actorlist[]; const char* actorlist[] = { "CrowdActor", "" };
ACTOR_SETUP(CrowdActor, CrowdActor)

extern VActor* newActor(const char* ActorType);

CrowdActor::CrowdActor() :
	mgDelete(newActor("MessageGroup")), // for DeleteWhenDoneMG
	sampActor(newActor("SampleActor")),
	hSampActor(sampActor->handle()),
	hMGDelete(mgDelete->handle()),
	ih(0), ihMax(1),
	dB(0.),
	zRateMin(0.), zRateMax(0.),
	idMomentary(1.)
{
	setTypeName("CrowdActor");
	*szFile = '\0';
	memset(rgxyz, 0, sizeof rgxyz);

	for (int i=0; i<iSndMax; i++)
		rgh[i].Init(hNil, 0.);

	char szCmd[200];
	sprintf(szCmd, "AddMessage %g NotifyDeletion %g *0 *1",
		hMGDelete, handle());
	actorMessageHandler(szCmd);
}

CrowdActor::~CrowdActor()
{
	// Wait for VActor::flushActorList() to delete these,
	// to avoid heap-use-after-free.
#if 0
	delete mgDelete;
	delete sampActor;
#endif
}

void CrowdActor::setDirectory(const char* sz)
{
	char szCmd[1100];
	sprintf(szCmd, "SetDirectory %g %s", hSampActor, sz);
	actorMessageHandler(szCmd);
}

void CrowdActor::setFile(const char* sz)
{
	strncpy(szFile, sz, cchFileMax);
	char szCmd[1100];
	sprintf(szCmd, "LoadFile %g %s", hSampActor, szFile);
	actorMessageHandler(szCmd);
}

void CrowdActor::setRate(float rMin, float rMax)
{
	if (rMin <= 0. || rMax <= 0.)
		{
		fprintf(stderr, "vss error: CrowdActor::SetRate args must be > 0\n");
		return;
		}
	zRateMin = log(rMin);
	zRateMax = log(rMax);
}

void CrowdActor::setMaxSimultaneous(int c)
{
	if (c > iSndMax)
		{
		fprintf(stderr, "vss warning: setMaxSimultaneous rounded down to %d\n", iSndMax);
		c = iSndMax;
		}
	ihMax = c;
}

static float sq(float _) { return _*_; }
static const XYZ xyzListener = { -1.,   0.,5.,0. }; // center of cave
static float DistSquaredXYZ(const XYZ* p1, const XYZ* p2 = &xyzListener) {
	return sq(p1->x-p2->x) + sq(p1->y-p2->y) + sq(p1->z-p2->z);
}

// The return value may be modified.
float& CrowdActor::PhFromId(float id)
{
	static float _ = hNil;
	for (int i=0; i<ihMax; i++)
		if (id == rgh[i].id)
			return rgh[i].h;
	printf("CrowdActor::PhFromId error!! id=%g, ihMax=%d\n\n", id, ihMax);
	return _;
}

// The return value may be modified.
XYZ& CrowdActor::XYZFromId(float id)
{
	static XYZ _ = { -1., 0.,0.,0. };
	for (int i=0; i<cxyz; i++)
		if (id == rgxyz[i].id)
			return rgxyz[i];
	printf("CrowdActor::XYZFromId error!! id=%g, cxyz=%d\n\n", id, cxyz);
	return _;
}

void CrowdActor::play(float x, float y, float z)
{
/*{
printf("___________ bgn play(%g %g %g)___________ \n", x,y,z);;
for (int i=0; i<ihMax; i++)
	printf("\t\t\th=%g id=%g\n", rgh[i].h, rgh[i].id);;
printf("XYZs:\n");
for (i=0; i<ihMax; i++)
	printf("\t\t\tid=%g, %g %g %g\n", rgxyz[i].id, rgxyz[i].x, rgxyz[i].y, rgxyz[i].z);
}*/

	// This is a Request to play another sound.
	// Let's see if we should accommodate it.

	int ihNewGuy = -1;
	char szCmd[1100];

	// Is there a spot free in rgh?  If so, just take it.
	for (ihNewGuy=0; ihNewGuy<ihMax; ihNewGuy++)
		{
		if (rgh[ihNewGuy].h == hNil)
			goto LDoit;
		}

	// No spots were free.
	// Find xyz farthest from listener, from rgh U {new one}.

	{
	float distMax = -1e9;
	ihNewGuy = -1;
	for (int i=0; i<ihMax; i++)
		{
		float dist = DistSquaredXYZ(&rgxyz[i]);
		if (dist > distMax)
			{
			distMax = dist;
			ihNewGuy = i;
			}
		}
	XYZ xyzNew; xyzNew.Init(-1., x,y,z);
	float distNew = DistSquaredXYZ(&xyzNew);
	if (distNew > distMax)
		// The farthest one *is* the new one, so deny the request.
		return;
	}

	// End the farthest one.
	// It'd be nice to fade out first with "Delete %g 0.1".
	sprintf(szCmd, "Delete %g", rgh[ihNewGuy].h);
	actorMessageHandler(szCmd);
	rgh[ihNewGuy].h = hNil;
	XYZFromId(rgh[ihNewGuy].id).id = -1;
	ih--;
	cxyz = ihMax;

	// Replace its slot in rgh with the new one.
	// (ihNewGuy already has the correct value.)

LDoit:
	// Now ihNewGuy is the place in rgh where to insert (idMomentary,x,y,z).

	sprintf(szCmd,
		"BeginSound %g SetFile %s SetGain %.2f SetXYZ %.2f %.2f %.2f",
		hSampActor, szFile, dB, x,y,z);
	if (zRateMin != 0. && zRateMax != 0.)
		{
		float logRate = zRateMin + drand48() * (zRateMax - zRateMin);
		// fancy strcat-with-args
		sprintf(szCmd + strlen(szCmd), " SetPlaybackRate %f", exp(logRate));
		}
	actorMessageHandler(szCmd);

	// VGeneratorActor::receiveMessage calls ReturnFloatToClient() to store handle,
	// which value is retrieved by ClientReturnVal().
	float hSnd = ClientReturnVal();
	sprintf(szCmd, "DeleteWhenDoneMG %g %g", hSnd, hMGDelete);
	actorMessageHandler(szCmd);
	rgh[ihNewGuy].Init(hSnd, idMomentary);

	// Add new guy to rgxyz in a free slot (i.e., with id <= 0.).
	for (int i=0; i<ihMax; i++)
		if (rgxyz[i].id <= 0.)
			{
			rgxyz[i].Init(idMomentary, x,y,z);
			break;
			}
	ih++;
	cxyz = ihMax;
	idMomentary++;
/*{
printf("___________ end play(%g %g %g)___________ \n", x,y,z);;
for (int i=0; i<ihMax; i++)
	printf("\t\t\th=%g id=%g\n", rgh[i].h, rgh[i].id);;
printf("XYZs:\n");
for (i=0; i<ihMax; i++)
	printf("\t\t\tid=%g, %g %g %g\n", rgxyz[i].id, rgxyz[i].x, rgxyz[i].y, rgxyz[i].z);
}*/
}

void CrowdActor::autostop(float h)
{
	printf("\t\t\tDel %s h=%g: play %d/%d.\n",
		szFile,
		h,
		ih-1, ihMax);
/*{
printf("___________ bgn autostop ____________________\n");
for (int i=0; i<ihMax; i++)
	printf("\t\t\th=%g id=%g\n", rgh[i].h, rgh[i].id);;
printf("XYZs:\n");
for (i=0; i<ihMax; i++)
	printf("\t\t\tid=%g, %g %g %g\n", rgxyz[i].id, rgxyz[i].x, rgxyz[i].y, rgxyz[i].z);
}*/
	// Remove "h" from rgh[ 0 .. ih-1 ].
	int j;
	for (j=0; j<ihMax; j++)
		{
		if (h == rgh[j].h)
			{
			rgh[j].h = hNil; // Found h.  Nil it, to free up its slot.
			break;
			}
		}
	// (Assert that we did in fact find h.)

	// Remove him from rgxyz too, by setting his id to -1.
	XYZFromId(rgh[j].id).id = -1;
	ih--;
	cxyz = ihMax;
}

int CompareXYZ(const void* pv1, const void* pv2)
{
	// For qsort to compare distances, don't need to take the square root.
	const auto d1 = DistSquaredXYZ((const XYZ*)pv1);
	const auto d2 = DistSquaredXYZ((const XYZ*)pv2);
	return d1<d2 ? -1 : d1>d2 ? 1 : 0;
}

int CrowdActor::FTopN(float id) const
{
	for (int i=0; i<ih; i++)
		if (id == rgidTopN[i])
			return 1;
	return 0;
}

int CrowdActor::FTopNPrev(float id) const
{
	for (int i=0; i<ihPrev; i++)
		if (id == rgidTopNPrev[i])
			return 1;
	return 0;
}

void CrowdActor::xyzArray(float* rgz, int cz)
{
/*{
printf("___________ bgn xyzArray___________ \n");;
for (int i=0; i<ihMax; i++)
	printf("\t\t\th=%g id=%g\n", rgh[i].h, rgh[i].id);;
printf("XYZs:\n");
for (i=0; i<ihMax; i++)
	printf("\t\t\tid=%g, %g %g %g\n", rgxyz[i].id, rgxyz[i].x, rgxyz[i].y, rgxyz[i].z);
}*/
	int i;
	if (cz % 4 != 0)
		{
		fprintf(stderr, "vss error: CrowdActor::XYZArray needs a multiple of 4 elements in its array.\n");
		return;
		}

	// Save current data
	cxyzPrev = cxyz;
	memcpy(rgxyzPrev, rgxyz, sizeof(XYZ) * cxyz);
	memcpy(rgidTopNPrev, rgidTopN, sizeof(float) * ih);
	ihPrev = ih;

	// Copy current data into rgxyz.
	cxyz = cz / 4;
	memset(rgxyz, 0, sizeof(XYZ) * ihMax); // just to be clean.
	memcpy(rgxyz, rgz, sizeof(XYZ) * cxyz);

	// Find (up to) ihMax nearest id's, by sorting rgxyz.
	// (Stuff rgidTopN and ih.)
	qsort(rgxyz, cxyz, sizeof(XYZ), CompareXYZ);
	ih = ihMax;
	if (ih > cxyz)
		ih = cxyz;
	for (i=0; i<ih; i++)
		rgidTopN[i] = rgxyz[i].id;

/*{
printf("___________ mid xyzArray___________ \n");;
printf("cz=%d cxyz=%d ihMax=%d ih=%d\n", cz, cxyz, ihMax, ih);;;;
for (int i=0; i<ihPrev; i++)
	printf("\t\t\tTopNPrev %g\n", rgidTopNPrev[i]);
for (i=0; i<ih; i++)
	printf("\t\t\tTopN %g\n", rgidTopN[i]);
printf("XYZs:\n");
for (i=0; i<ihMax; i++)
	printf("\t\t\tid=%g, %g %g %g\n", rgxyz[i].id, rgxyz[i].x, rgxyz[i].y, rgxyz[i].z);
}*/

	// Fade out handlers of any ids in rgidTopNPrev but not in rgidTopN.
	// For each id in rgidTopN,
	// if it's also in rgidTopNPrev, move its handler with SetXYZ.
	// Otherwise, create a new handler at its XYZ position.

	for (i=0; i<ihPrev; i++)
		{
		float id = rgidTopNPrev[i];
		if (!FTopN(id))
			{
			// Delete id's handler, then take that id,h out of the list.
			// Yes, h will be overwritten.
			float& h = PhFromId(id);
			char szCmd[200];
			// It'd be nice to fade out first with "Delete %g 0.1".
			sprintf(szCmd, "Delete %g", h);
			actorMessageHandler(szCmd);
			h = hNil;
			}
		}

/*{
printf("___________ after delete, xyzArray___________ \n");;
for (int i=0; i<ihPrev; i++)
	printf("\t\t\tTopNPrev %g\n", rgidTopNPrev[i]);
for (i=0; i<ih; i++)
	printf("\t\t\tTopN %g\n", rgidTopN[i]);
}*/

	for (i=0; i<ih; i++)
		{
		float id = rgidTopN[i];
		if (FTopNPrev(id))
			{
			// Move id's handler.
			const XYZ& xyz = XYZFromId(id);
			char szCmd[200];
			sprintf(szCmd, "SetXYZ %g %.2f %.2f %.2f .003",
				PhFromId(id), xyz.x,xyz.y,xyz.z);
				// Only .003, because long interpolations waste CPU.
			actorMessageHandler(szCmd);
			}
		else
			{
			// Create a new handler for id at its XYZ position.
			//		;;Nice: start the sound randomly somewhere in the loop.
			// Add this id,h to the list.
			const XYZ& xyz = XYZFromId(id);

			char szCmd[2400];
			sprintf(szCmd, "BeginSound %g SetFile %s SetGain %.2f SetXYZ %.2f %.2f %.2f SetLoop 1",
				hSampActor, szFile, dB, xyz.x,xyz.y,xyz.z);
			if (zRateMin != 0. && zRateMax != 0.)
				{
				float logRate = zRateMin + drand48() * (zRateMax - zRateMin);
				// fancy strcat-with-args
				sprintf(szCmd + strlen(szCmd),
					" SetPlaybackRate %f", exp(logRate));
				}
			actorMessageHandler(szCmd);

			// VGeneratorActor::receiveMessage calls ReturnFloatToClient() to store handle,
			// which value is retrieved by ClientReturnVal().
			float hSnd = ClientReturnVal();

		//;;;;	sprintf(szCmd, "DeleteWhenDoneMG %g %g", hSnd, hMGDelete);
		//;;;;	actorMessageHandler(szCmd);

			// There should be an empty spot (h == hNil) in rgh.
			for (int j=0; j<ih; j++)
				{
				if (rgh[j].h == hNil)
					{
					rgh[j].Init(hSnd, id);
					printf("\t\t\tNew %s h=%g, id %f: play %d/%d.\n",
						szFile, hSnd, id, ih, ihMax);
					break;
					}
				}
			}
		}
/*{
printf("___________ end xyzArray___________ \n");;
for (int i=0; i<ihMax; i++)
	printf("\t\t\th=%g id=%g\n", rgh[i].h, rgh[i].id);;
printf("XYZs:\n");
for (i=0; i<ihMax; i++)
	printf("\t\t\tid=%g, %g %g %g\n", rgxyz[i].id, rgxyz[i].x, rgxyz[i].y, rgxyz[i].z);
}*/
}

/*
void CrowdActor::act()
{
	VActor::act();
//;;	float now = currentTime();
	// Maybe use this to send SetXYZ's to sounds when frame of reference changes.
}
*/

int CrowdActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetDirectory"))
		{
		ifS( sz, setDirectory(sz) );
		return Uncatch();
		}

	if (CommandIs("SetFile"))
		{
		ifS( sz, setFile(sz) );
		return Uncatch();
		}

	if (CommandIs("SetGain"))
		{
		ifF( z, dB=z );
		return Uncatch();
		}

	if (CommandIs("SetRate"))
		{
		ifFF( rMin, rMax, setRate(rMin, rMax) );
		return Uncatch();
		}

	if (CommandIs("SetMaxNum"))
		{
		ifD( c, setMaxSimultaneous(c) );
		return Uncatch();
		}

	if (CommandIs("Play"))
		{
		ifFFF( x,y,z, play(x,y,z) );
		ifNil(        play() );
		}

	if (CommandIs("PlayCrowd"))
		{
		ifFloatArray( data, count, xyzArray(data, count) );
		return Uncatch();
		}

	if (CommandIs("NotifyDeletion")) // sent only by hMGDelete
		{
		ifF( h, autostop(h) );
		return Uncatch();
		}

	return VActor::receiveMessage(Message);
}
