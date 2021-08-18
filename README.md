# VSS virtual sound server

VSS has two parts: a **server** (`vss` or `vss.exe`), and a **client** library (`vssClient.h` and `libsnd.a`) for building apps that talk to the server.

This version, 4.2, is the same as 4.1 (2012), but without code that was copyrighted by others.  
It is compatible with applications built with VSS 3.1 clients.  
The client library is included in version 3.1 (2000).

Prerequisites: `sudo apt install make g++ libasound2-dev`  
To build `vss`: `make depend && make`  
To tidy up, if you need only `./vss`: `make clean`

To run, you may first need to: `sudo echo 'pcm.!default { type plug slave.pcm "null" }' > /etc/asound.conf`

Copyright 2021 University of Illinois Board of Trustees.
