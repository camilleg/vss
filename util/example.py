#!/usr/bin/env python3

"""
A hello-world example of pure Python playing a sound on VSS running on the same host.
Error checking is omitted for clarity.
"""

import socket
from time import sleep

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(2.5) # From cli/cliMsg.c.
addr = ('127.0.0.1', 7999)

def send(s):
    sock.sendto(s.encode(), addr)

def get():
    try:
        r, _server_unused = sock.recvfrom(512)
        r = r[0:len(r)-1] # Strip the trailing null, from e.g. "AckNoteMsg 7.00"
    except socket.timeout:
        print('Timed out waiting for VSS.')
        exit(1)
    return r

def getnum(s):
    send(s)
    return int(float(get().split()[1].decode()))

def done():
    sock.close()

def main():
    send('SetPrintCommands 2')
    send('EnableOfile 1 /tmp/example.raw')

    # Play a 1000 Hz tone that sweeps to 500 Hz and fades to silence.

    fm = getnum('Create FmActor')
    sound = getnum(f'BeginSound {fm}')

    send(f'SetAmp {sound} 0.4')
    send(f'SetCarFreq {sound} 1000')
    sleep(0.3)

    send(f'SetCarFreq {sound} 500 0.2')
    send(f'SetAmp {sound} 0 0.4')
    sleep(0.5)

    send(f'Delete {sound}')
    send(f'Delete {fm}')
    sleep(0.1)

    send('EnableOfile 0 /tmp/example.raw')
    sleep(0.1)

    # send('KillServer')
    done()

if __name__ == '__main__':
    main()
