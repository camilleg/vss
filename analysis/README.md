The 1997 [Pure Data](https://puredata.info/) source code `d*.c` and `m*.c` uses many identifiers that begin with an
underscore and have file scope.  This violates the 1999 C Standard, section 7.1.3
(and the C++ standards based thereon), but instead of correcting that,
VSS should replace that code with a more recent version, such as [2017's](http://msp.ucsd.edu/Software/pd-0.51-4.src.tar.gz).
