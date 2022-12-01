# VSS, Virtual Sound Server

VSS has:
-   a server, `vss` or `vss.exe`
-   a library, `vssClient.h` and `libsnd.a`, to build clients that talk to the server.

Version 4.2 is the same as 4.1 from 2012, except that it excludes external code that was unlicenseable.
It is compatible with VSS 3.1 clients.  
The client library is included in version 3.1 (2000).

Prerequisites: `sudo apt install g++ libasound2-dev make`  
To build: `make`  
To run: `./vss`  
To tidy up: `make clean`

© 2022 University of Illinois Board of Trustees, except for
-   `analysis/[dm]_*.c` from Miller Puckette's [Pure Data](https://puredata.info/), which is © 1997 the Regents of the U. of California;
-   `basic/*`'s Pascal to C translator, which is © 1989 Dave Gillespie;
-   part of `./map/*.c++` from Ken Clarkson's [hull.shar](http://www.netlib.org/voronoi/), which is © 1995 AT&T;
-   part of `osc/*.c++` which is © 1997 the Regents of the U. of California.

The source code is licensed under the [MIT License](https://mit-license.org/).
