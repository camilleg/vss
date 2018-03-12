#	vssObj.py
#	 
#	object and function definitions for Python 
#	embedded in vss.
#
import string

# define communication with vss and real-time access:
try:
    # import the statically-linked vssSrv module,
    # linking message and response functions in vss,
    # and setup the vssObj classes:
    import vssSrv 
    postVssMessage = vssSrv.Send
    def currentTime():
        return vssSrv.Time()
except ImportError:
    # no vssSrv module, use a stub
    def postVssMessage( msg ):
        print '\tpost: ' + msg
        return 0
    try:
        import time
        def currentTime():
            return time.time()
    except ImportError:
        print 'neither vssSrv nor time modules available, currentTime() undefined'

##############################################################################
#    vss handles
#
# bad actor handle
hNil = -1 

# Exception class - bad actor handle
class BadHandle:
    def __init__(self, h = hNil):
        self.handle = h
    def __repr__(self):
        return 'VssObj got a bad actor handle: ' + str(self.handle)               

##############################################################################
#    vss messages
#
#	Message class 
#	Initialize with the message (a string) and
#	the receiver (a VssObj, or something with a
#	handle method).	Call with any args.
class Msg:
    def __init__(self, msg, rcvr):
    	self.msg = msg
    	self.rcvr = rcvr
    def __call__(self, *args):
        slist = [ self.msg ]
        slist.append( str( self.rcvr.handle() ) )
        slist.append( string.join( map(str, args), ' ') )
     	return postVssMessage( string.join(slist, ' ') )
   
##############################################################################
#    Base class of objects in vss.
#
class VssObj:
    #	initialize a VssObj with its handle,
    #	must not be hNil
    def __init__(self, h):
        self.__handle = self._strToHandle(h)
    # 	restrict handle access
    def handle(self):
	return self.__handle
    # 	make sure this works    
    def Delete(self):
	if self.handle() > hNil:
	    self.msg('Delete')
	self.__handle = hNil
    #	send a message to myself
    def msg(self, msg, *args):
        m = Msg( msg, self )
        # have to convert the tuple of args
        return m( self._makeargs( args ) )
    #	define a attribute corresponding to
    #	a vss message     
    def __getattr__(self, msg):
        # don't make messages out of things that
        # can't be vss messages	
        if self._validVssMessage(msg):
            #print '\t(defining new message: ' + msg + ')'
            setattr(self, msg, Msg( msg, self ))
            return getattr(self, msg)
        else:
            s = '\t(' + msg + ' is not a valid vss message)'
            raise AttributeError
    #	make it nice to look at
    def __repr__(self):
        return 'Vss object with handle = ' + str(self.handle())
    def __str__(self):
        return self.__repr__()
    #	helpers:          
    def _makeargs(self, args):
        return string.join( map(str, args), ' ')
    # conversion of return string
    # to handle:
    def _strToHandle(self, str):
        try:
            f = float( str );
            if f < 0:
                return hNil
            else:
                return f
        except TypeError:
            raise BadHandle( str )
    # test for valid vss message
    # (how should I do this?)
    # (for now, all vss messages are capitalized)
    def _validVssMessage(self, msg):
        return msg[0] in string.uppercase
    
##############################################################################
#    Actor class
#
class Actor( VssObj ):
    def __init__(self, type):
        self.__type = type
        s = 'Create ' + type
        VssObj.__init__(self, postVssMessage(s))
    def type(self):
        return self.__type
    def __repr__(self):
        return 'Vss Actor of type ' + self.type() + ' with handle ' + \
        	str(self.handle())
        	
##############################################################################
#    Sound class
#
class Sound( VssObj ):
    def __init__(self, parent, *args):
        self.__parent = parent
        h = parent.msg( 'BeginSound', self._makeargs(args) )
        VssObj.__init__( self, h )
    def parent( self ):
        return self.__parent
    def __repr__( self ):
        return 'Vss Sound with handle ' + str(self.handle()) + \
        	'\n\tparent: ' + str(self.parent())
        	
##############################################################################
#    other handy items
#
def load( dso ):
	msg = 'LoadDSO ' + dso
	postVssMessage(msg)

def dump( obj = None ):
    if obj:
        postVssMessage('Dump ' + str( obj.handle() ) )
    else:
    	postVssMessage('DumpAll')
    	
##############################################################################
#    scheduling
#
_interp = ''
_scheduler = ''
_namespace = ''
_eventList = {}

#    admin, need this for scheduling messages,
#    called from PyInterp
def setNamespace( name ):
    global _namespace
    _namespace = name
    
def _init_scheduling():
    if _interp and _scheduler:
        return
    load( 'later.so')
    global _interp, _scheduler, _namespace;
    _scheduler = Actor( 'LaterActor' )
    _interp = Actor('PyActor')
    _interp.Namespace(_namespace)

# add an event to the list of events:
def ScheduleEvent( delay, func, *args ):
    if not _scheduler:
        _init_scheduling();
    eventTime = currentTime() + delay
    event = (func, args)
    if _eventList.has_key(eventTime):
        # add this event to the list of events at eventTime
	_eventList[eventTime].append( event )
    else:
        # register a new event list with _scheduler
        s = 'Execute %f TriggerEvents()' % _interp.handle()
        _scheduler.AddMessage( delay, s )
    	# create the event list with this event:
    	_eventList[eventTime] = [event]
		
# trigger events having times in the schedule
# occuring at or before the current time.
# (must be accessible at the top module level)
def TriggerEvents():
    time = currentTime()
    for t in _eventList.keys():
	if t <= time:
	    # print 'triggering events at', t
	    events = _eventList[t]
	    for e in events:
		func,args = e
		apply(func,args)
	    del _eventList[t]
