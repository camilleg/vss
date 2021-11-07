#include "messageGroup.h"

// #define _extremely_verbose_

//===========================================================================
//	Delimiters for building messages.
//
//	These should _not_ be the same, or else the message building
//	will fail (strtok is used to find tokens delimited by IndexDelimStr,
//	and will find extra tokens if HandleDelim is the same character.).
// 
const char * MessageGroup::IndexDelimStr = "*";
const char MessageGroup::HandleDelim = '?';

MessageGroup::MessageGroup() :
#ifdef VSS_MATH_HACK
	fMathHack(0),
#endif
	recentHandle( hNil )
{
	setTypeName("MessageGroup");
#ifdef VSS_MATH_HACK
	szMathPrefix[0] = '\0';
#endif
}

MessageGroup::~MessageGroup()
{
	DelayedDataList::iterator it;
	for (it = dataList.begin(); it != dataList.end(); it++)
	//	{
	//	printf("\t%x <= %x <= %x\n",
	//		(int)&(*(dataList.begin())), (int)&(*it), (int)&(*(dataList.end())));
		delete (*it);
	//	}
}

//	For each parameterized message in the messageList, build up a real
//	message and send it. If a handle was created by sending that message,
//	store it. Those handles are only valid for this data array, reset it
//	to hNil when done.
void MessageGroup::receiveData(float* data, int dataSize)
{
	if (!data)
		fprintf(stderr, "MessageGroup::receiveData(NULL, %d): crash imminent.\n", dataSize);
	// What should happen if dataSize <= 0?  How might that happen?

#ifdef VSS_MATH_HACK
	if (fMathHack)
		{
		// code in here which is just too darn slow for interpreted bc
		// data[3] should be in the range 0 to 1.
		// 0 = almost no correction, 1 = almost total correction.
		// Beyond that is valid, but doesn't cause any more change.

		// map data[3] linearly from [0,1] to [2,-3], then take exp().
		const float k = exp(data[3]*-5.+2.);
		const float a = log(220.);
		float x = log(data[1]) - a;
		float r = log(2.) / 12.;
		float lopitch = floor(x/r);
		float xlo = lopitch * r;
		float xmed = xlo + r * .5;
		float b = sqrt(k + .25);
		float xNew = (x<xmed) ?
			xlo -k*r*.5*(1/(b+.5) + 1/((x-xlo )*2/r-b-.5)) :
			xmed+k*r*.5*(1/(b-.5) - 1/((x-xmed)*2/r+b-.5));
		data[1] = exp(xNew+a);
		}
	if (*szMathPrefix)
		{
		if (!strcmp(szMathPrefix+1, "v5=v0*2-1 ;"))
			data[5]=data[0]*2.-1.;
		else if (!strcmp(szMathPrefix+1, "v4=160+850*sqrt((v0-v2)^2 + (v1-v3)^2) ;"))
			{
			#define sq(_) ((_)*(_))
			data[4]=160+850*sqrt(sq(data[0]-data[2]) + sq(data[1]-data[3]));
			}
		else if (!strcmp(szMathPrefix+1, "x=v3-.2; if (x<0) x=0; v3=x ;"))
			{
			data[3] -= .2; if (data[3]<0) data[3]=0;
			}
		else if (!strcmp(szMathPrefix+1, "x=4; if (v2<.97) x=1; if (v2<.6) x=.25; v2=x ;"))
			{
			float x=4; if (data[2]<.97) x=1; if (data[2]<.6) x=.25; data[2]=x;
			}
		else if (!strcmp(szMathPrefix+1, "v3=v1*v1/293.333 ;"))
			data[3] = sq(data[1]) / 293.33333;
		else if (!strcmp(szMathPrefix+1, "v0+=.2; v4=1.01 + 100*((v2-.03)^2) ;"))
			{
			data[0]+=.2; data[4]=1.01 + 100*(sq(data[2]-.03));
			}
		else if (!strcmp(szMathPrefix+1, "v3=(v0-.1)/.7; v4=(v2-.2)/.8 ;"))
			{
			data[3]=(data[0]-.1)/.7; data[4]=(data[2]-.2)/.8;
			}
		else
			printf("vss error: unrecognized math hook <%s>\n",
				szMathPrefix+1);;
		}
#endif

	{
	//printf("\n\n\t\t\t\tsecret ninja: %d floats\n\n\n", dataSize);;
	// Stuff data array into a secret ninja global array vrgzMG,
	// so other things like thresholdactors can use
	// "*4" to get at these values.
	memcpy(VrgzMG(), data, dataSize * sizeof(float));
	}

	char * message;
	ParamMsgList::iterator it;
	for (it = messageList.begin(); it != messageList.end(); it++)
	{
		if (NULL == (message = buildMessage((*it).msg, data, dataSize)))
		{
			printf("vss error: MessageGroup failed to build a message.\n");
			continue;
		}
 //printf("\tMessageGroup sending <%s>\n", message);		
		actorMessageHandler( message );
		delete [] message;
		
//	look for a new handle
		float hTmp = ClientReturnVal();
		if ( hNil != hTmp )
		{
#ifdef _extremely_verbose_
			printf("MessageGroup saving handle: %f\n", hTmp);
#endif
			recentHandle = hTmp;
			*PvzMessageGroupRecentHandle() = recentHandle; // for passing back to ... to AUDupdate() on the client side.
		}
	}
	
	recentHandle = hNil;
}

//	Construct a message by replacing indices in a parameterized message
//	with data from a data array: look for the delimiter character,
//	followed by an integer, representing an index into the data array.
//	Build up a message by replacing these indices with data from the
//	array. If an index exceeds the size of the data, complain bitterly, 
//	and don't send the message. 
char *
MessageGroup::buildMessage(const char * pmsg, float * data, int dataSize)
{
	char copy[5000], szT[5000];
	static char message[5000];
	strncpy(copy, pmsg, 4999);
#ifdef _extremely_verbose_
	printf("\n\tMessageGroup building message from %s\n", copy);
#endif

//	find the first token and write it into the message
//	the first token is everything before a delimiter
	char * ch = strtok(copy, IndexDelimStr);
	
//	a null char has been inserted into copy at the position
//	of the delimiter, so ch points to a terminated string of 
//	non delimiters.  (The strspn magic skips any leading spaces.)
	strcpy(message, ch + strspn(ch, " "));
#ifdef _extremely_verbose_
	printf("\tfound %s...\n", ch);
#endif

//	int fEscapeNextDelim = ch[strlen(ch)-1] == '\\';
		
//	if there are more tokens, the next one will be a number,
//	white space delimited, or else the RecentHandle character.
	while (NULL != (ch = strtok(NULL, IndexDelimStr)))
		{
		// cch is used to find the next thing in the token after
		// the delimiter. After we substitute the handle into the
		// message, we will still have to copy in the remainder of the token.

		int cch = 0; // how many characters to skip over, sort of.
		float insertFloat;
LGotMyOwnTokenAlready:
		*szT = '\0';

//		if (fEscapeNextDelim)
//			{
//			// strip previous backslash from end of accumulator
//			message[strlen(message)-1] = '\0';
//			}
//		else
		if (*ch == HandleDelim)
			{
#ifdef VSS_WINDOWS
			++cch; // skip over HandleDelim
			// don't bother skipping over any whitespace after it (hope this works)
#else
			sscanf(ch, "%*c %n", &cch);
#endif
			insertFloat = recentHandle;
			sprintf(szT, "%f %s", insertFloat, ch+cch);
			// %f is the float, %s is everything in the token after the float,
			// i.e. everything up to the next "*" (or to the end of the string).
			}
		else if (*ch == '$')
			{
			// it's just the end of a "*4 to *$" string
			}
		else //	indexed data
			{
			int index;
#ifdef VSS_WINDOWS
			// workaround for compiler bug (scanf %n)
			{
			char* chNext;
			index = (int)strtol(ch, &chNext, 0);
			cch = chNext - ch;
			}
			if (cch == 0)
#else
			if (1 != sscanf(ch, "%d %n", &index, &cch))
#endif
				{
				fprintf(stderr,
					"vss error: MessageGroup found non-number \"%s\" after \"%s\" in message \"%s\".\n",
					ch, IndexDelimStr, pmsg);
				return NULL;
				}
				
			//	find the indexed datum
			if (index >= dataSize)
				{
				// This allows you to use *4 outside a messagegroup, sort of.
				//printf("\t\t\t\tusing secret ninja [%d]\n", index);;
				insertFloat = VrgzMG()[index];
				}
			else
				{
				insertFloat = data[index];
				}

			// Special check for "*4 to *9" or "*4 to **".
			int indexLast;
//printf("special check <%s>\n", ch+cch);
			if (0 == strcmp(ch+cch, "to "))
				{
				ch = strtok(NULL, IndexDelimStr); // next token
//printf("now ch==<%s>\n", ch);
				if (0 == strncmp(ch, "$", 1))
					{
					cch = 1;
					indexLast = dataSize - 1;
//printf("*$ = %d\n", indexLast);
					goto LGotIndexLast;
					}
#ifdef VSS_WINDOWS
				// workaround for compiler bug (scanf %n)
				{
				char* chNext;
				indexLast = (int)strtol(ch, &chNext, 0);
				cch = chNext - ch;
				}
				if (cch == 0)
#else
				if (1 != sscanf(ch, "%d %n", &indexLast, &cch))
#endif
					{
					// Special check failed.  Append the previous token
					// anyways, and continue.
					sprintf(szT, "%f %s", insertFloat, ch+cch);
//printf("Special check failed.  append <%s>\n", szT);;;;
					strcat(message, szT);
					goto LGotMyOwnTokenAlready;
					}
//printf("*%d\n", indexLast);

LGotIndexLast:
				if (indexLast < index)
					{
					fprintf(stderr, "vss error: *a to *b, b must be > a\n");
					indexLast = index;
					}

				float* dataReal = indexLast >= dataSize ? VrgzMG() : data;
				*szT = '\0';
//printf("from %d to %d\n", index, indexLast);
				for (int i=index; i<=indexLast; i++)
					{
					// fancy strcat
					sprintf(szT + strlen(szT), "%f ", dataReal[i]);
//printf("(%g) ", dataReal[i]);
					}
				sprintf(szT + strlen(szT), "%s", ch+cch);
//printf("finally, szT= <%s>\n", szT);;;;
				}
			else
				sprintf(szT, "%f %s", insertFloat, ch+cch);
			}

//printf("XXXX <%s>\n", szT);;;;

		strcat(message, szT); // append szT to accumulator "message"

//		fEscapeNextDelim = ch[strlen(ch)-1] == '\\';
		}
	// Will be delete[]d by the caller, receiveData().
	char* msgRet = new char[strlen(message) + 2];
	strcpy(msgRet, message);
	return msgRet;
}

//	The ScheduleData message requires special parsing, because its
//	arguments are an unspecified number of arrays of data to be received
//	(with receiveData()) at some later time. The times are in the first
//	array argument, and the remaining arrays are the actual data arrays.
//	There must be as many data arrays as time offsets.
//
//	Derived classes may override the members for handling schedules,
//	but parseSchedule is the gross part, and should not need to be 
//	overridden. Try overriding the other members (startReceiveSchedule,
//	receiveScheduledData, endReceiveSchedule) first.
int
MessageGroup::parseSchedule(char* arrays)
{
// printf("\n\tparseSchedule received %s\n", arrays);

	const char	BeginArray = '[';
	const char	EndArray = ']';
	const int	MaxDataSize = 64;
	
//	The first array will contain the time offsets.
#define MaxNumDelayedArrays 32
	float timeOffsets[MaxNumDelayedArrays];
	int	numOffsets = SscanfFloats(MaxNumDelayedArrays, timeOffsets, arrays);
	
	if (numOffsets < 0)	// bogus
	{
		printf("vss error: MessageGroup received bogus scheduled data.\n");
		return Uncatch();
	}
#ifdef _extremely_verbose_
	printf("\tparseSchedule received %d time offsets\n", numOffsets);
#endif

	startReceiveSchedule(numOffsets);
	
//	Look for numOffsets arrays of delayed data.
	int count;
	for( count = 0; count < numOffsets; count++ )
	{
		// point to past the end of the array we just handled
		arrays = strchr(arrays, EndArray) + 1;
		
		//	make sure that the next thing is an array
		char ch;
		if (1 != sscanf(arrays, " %c", &ch) || ch != BeginArray)
		{
			printf("vss error: MessageGroup received garbled scheduled data at %s.\n", arrays);
			endReceiveSchedule(count);
			return Uncatch();
		}
			
		//	read the array
		float data[MaxDataSize]; //;; used to be new[]'ed, hemorrhaging memory.
		int dataSize = SscanfFloats(MaxDataSize, data, arrays);
		if (dataSize < 0)	// bogus
		{
			printf("vss error: MessageGroup received bogus scheduled data.\n");
			endReceiveSchedule(count);
			return Uncatch();
		}
		
		//	receive the new scheduled data
#ifdef _extremely_verbose_
		printf("\tparseSchedule received array of size %d\n", dataSize);
#endif
		receiveScheduledData( timeOffsets[count], data, dataSize );
	}
	
#ifdef _extremely_verbose_
	printf("\tparseSchedule received %d data arrays\n", count);
#endif
	endReceiveSchedule(count);
	return Catch();
}

//	Add an array of data with a time offset to the dataList, to be 
//	handled later. Derived classes may override this member to perform
//	data filtering or editing.
void
MessageGroup::receiveScheduledData(float time, float * data, int size)
{
	const auto dd = new DelayedData(time + currentTime(), data, size);
#ifdef DEBUG
	printf("adding data at time %f, size %d\n", dd->time, dd->size);
#endif
	dataList.push_back(dd);
}

//	Add a new message to our list.
void 
MessageGroup::addMessage(char* message)
{
	ParamMsg pm(message);
	messageList.push_back(pm);
}

#ifdef VSS_MATH_HACK
void MessageGroup::addMathPrefix(const char* sz)
{
	strncpy(szMathPrefix, sz, sizeof(szMathPrefix)-3);
	strcat(szMathPrefix, ";");
}
#endif

int 
MessageGroup::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("AddMessage"))
	{
		ifM( msg, addMessage(msg) );
		return Uncatch();
	}

#ifdef VSS_MATH_HACK
	if (CommandIs("MathHack"))
	{
		ifD( f, fMathHack=f );
		return Uncatch();
	}

	if (CommandIs("MathPrefix"))
	{
		ifM( sz, addMathPrefix(sz) );
		return Uncatch();
	}
#endif

	if (CommandIs("SendData"))
	{
		ifFloatArray( data, count, receiveData( data, count ) );

		fprintf(stderr, "vss error: MessageGroup failed to execute.\n");
		return Uncatch();
	}
	
	if (CommandIs("ScheduleData"))
	{
		ifM( stringOfArrays, parseSchedule(stringOfArrays) );
		return Uncatch();
	}

	return VActor::receiveMessage(Message);
}

//	Send and delete all delayed data arrays whose time has come.
void 
MessageGroup::act()
{
	VActor::act();
	const float now = currentTime();
	for (auto it = dataList.begin(); it != dataList.end(); ++it)
	{
		if (now >= (*it)->time)
		{
			receiveData( (*it)->data, (*it)->size );
			dataList.erase( it-- );
		}
	}
}
