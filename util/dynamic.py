#!/usr/bin/env python3

"""
Like ./dynamic.rb.
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

def gethandle(s):
    send(s)
    return int(float(get().split()[1].decode()))

def done():
    sock.close()

def main():
    send('SetPrintCommands 2')
    send('EnableOfile 1 /tmp/audio-out.raw')

    fm = gethandle('Create FmActor')
    UseMessageGroups = False
    if UseMessageGroups:

        # AUDupdateSimpleFloats()
        def update(hMG, floats, fGetReply=False):
            msg = f'SendData {hMG} [ ' + ' '.join(map(str, floats)) + ']'
            return gethandle(msg) if fGetReply else send(msg)

        newX = gethandle('Create MessageGroup')
        send(f'AddMessage {newX} BeginSound {fm} SetAmp 0.12 SetCarFreq *0')

        changeFreqX = gethandle('Create MessageGroup')
        send(f'AddMessage {changeFreqX} SetCarFreq *0 *1')

        deleteX = gethandle('Create MessageGroup')
        later = gethandle('Create LaterActor')
        send(f'AddMessage {deleteX} SetAmp *0 0 .15')
        send(f'AddMessage {deleteX} AddMessage {later} .16 Delete *0')

        # Create 10 sounds.
        sounds = []
        for i in range(10):
            sleep(0.04)
            sounds.append(update(newX, [300 + 100 * i], True))
        sleep(0.25)

        # Modify them.
        for i in range(10):
            sleep(0.06)
            update(changeFreqX, [sounds[i], 80+40*i])
        sleep(0.5)

        # Fade out and delete them.
        for i in range(10):
            update(deleteX, [sounds[i]])
        sleep(0.2)

    else:

        # Create 10 sounds.
        sounds = []
        for i in range(10):
            sleep(0.04)
            sounds.append(gethandle(f'BeginSound {fm} SetAmp 0.12 SetCarFreq {300 + 100 * i}'))
        sleep(0.25)

        # Modify them.
        for i in range(10):
            sleep(0.06)
            send(f'SetCarFreq {sounds[i]} {80+40*i}')
        sleep(0.5)

        # Fade out and delete them.
        for h in sounds:
            send(f'SetAmp {h} 0 .15')
        sleep(0.16)
        for h in sounds:
            send(f'Delete {h}')
        sleep(0.2)

    # Save the output to /tmp/audio-out.raw.aiff.
    send('EnableOfile 0')
    sleep(0.05)

    done()

if __name__ == '__main__':
    main()
