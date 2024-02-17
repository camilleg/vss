#!/usr/bin/env ruby

require 'socket'
$sock = UDPSocket.new
$sock.connect 'localhost', 7999

def send(s) $sock.send(s, 0) end

def get() $sock.recvfrom(500)[0] end  # e.g., "AckNoteMsg 7.00"

def gethandle(s) send(s); get.split[1].to_i end

def done() $sock.close end

send 'SetPrintCommands 2'
send 'EnableOfile 1 /tmp/audio-out.raw'
fm = gethandle 'Create FmActor'

UseMessageGroups = false
if UseMessageGroups

  # AUDupdateSimpleFloats()
  def update(hMG, floats, fGetReply=false)
    msg = "SendData #{hMG} [ " + floats.join(' ') + "]";
    fGetReply ? gethandle(msg) : send(msg)
  end

  newX = gethandle 'Create MessageGroup'
  send "AddMessage #{newX} BeginSound #{fm} SetAmp 0.12 SetCarFreq *0"

  changeFreqX = gethandle 'Create MessageGroup'
  send "AddMessage #{changeFreqX} SetCarFreq *0 *1"

  deleteX = gethandle 'Create MessageGroup'
  later = gethandle 'Create LaterActor'
  send "AddMessage #{deleteX} SetAmp *0 0 .15"
  send "AddMessage #{deleteX} AddMessage #{later} .16 Delete *0"

  # Create 10 sounds.
  sounds = (0..10).map {|i|
    sleep 0.04
    update newX, [300 + 100 * i], true
  }
  sleep 0.25

  # Modify them.
  sounds.each_with_index {|h,i|
    sleep 0.06
    update changeFreqX, [h, 80+40*i]
  }
  sleep 0.5

  # Fade out and delete them.
  sounds.each {|h|
    update deleteX, [h]
  }
  sleep 0.2

else

  # Create 10 sounds.
  sounds = (0..10).map {|i|
    sleep 0.04
    gethandle "BeginSound #{fm} SetAmp 0.12 SetCarFreq #{300 + 100 * i}"
  }
  sleep 0.25

  # Modify them.
  sounds.each_with_index {|h,i|
    sleep 0.06
    send "SetCarFreq #{h} #{80+40*i}"
  }
  sleep 0.5

  # Fade out and delete them.
  sounds.each {|h| send "SetAmp #{h} 0 .15" }
  sleep 0.16
  sounds.each {|h| send "Delete #{h}" }
  sleep 0.2

end

# Save the output to /tmp/audio-out.raw.aiff.
send 'EnableOfile 0'
sleep 0.05
done
