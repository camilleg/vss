#!/usr/bin/env ruby

# A hello-world example of pure Ruby playing a sound on VSS running on the same host.
# Error checking is omitted for clarity.

require 'socket'
$sock = UDPSocket.new
$sock.connect 'localhost', 7999

def send(s) $sock.send(s, 0) end

def get() $sock.recvfrom(500)[0] end  # e.g., "AckNoteMsg 7.00"

def getnum(s) send(s); get.split[1].to_i end

def done() $sock.close end

send 'SetPrintCommands 2'
send 'EnableOfile 1 /tmp/example.raw'

# Play a 1000 Hz tone that sweeps to 500 Hz and fades to silence.

fm = getnum 'Create FmActor'
sound = getnum "BeginSound #{fm}"

send "SetAmp #{sound} 0.4"
send "SetCarFreq #{sound} 1000"
sleep 0.3

send "SetCarFreq #{sound} 500 0.2"
send "SetAmp #{sound} 0 0.4"
sleep 0.5

send "Delete #{sound}"
send "Delete #{fm}"
sleep 0.1
send 'EnableOfile 0 /tmp/example.raw'
sleep 0.1

# send 'KillServer'

done # Not strictly needed.
