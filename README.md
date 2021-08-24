# VSS, Virtual Sound Server

VSS has:
- a server (`vss` or `vss.exe`)
- a library (`vssClient.h` and `libsnd.a`) for building clients that talk to the server.

Version 4.2 is the same as 4.1 from 2012, except that it excludes code that was copyrighted by others.  
It is compatible with VSS 3.1 clients.  
The client library is included in version 3.1 (2000).

Prerequisites: `sudo apt install make g++ libasound2-dev`  
To build: `make`  
To run: `./vss`  
To tidy up: `make clean`

Copyright 2021 University of Illinois Board of Trustees.
