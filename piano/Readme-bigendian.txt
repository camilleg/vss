The files pianodata/* were created on an SGI in UIUC's Computer
Music Project, probably cmpsgi2.music.uiuc.edu, recalls their author
Geoffrey Zheng (personal correspondence, 2021-10-27).

Thus, they were big-endian.  To read them on today's common
little-endian x86, each 32-bit word's bytes were reversed:

  cd pianodata
  mkdir 0
  mv *.??? 0
  cd 0
  for f in *.???; do hexdump -v -e '1/4 "%08x"' -e '"\n"' $f | xxd -r -p > ../$f; done

Two descriptions of this work:

Hua Zheng, James W. Beauchamp
Spectral characteristics and efficient critical-band-associated group synthesis of piano tones
October 1999, The Journal of the Acoustical Society of America 106(4):2141-2142
https://doi.org/10.1121/1.427323

James W. Beauchamp, Hua Zheng, Bowon Lee, Stephen Lakatos
A versatile wavetable-based model for synthesis of piano tones
November 2001, The Journal of the Acoustical Society of America 110(5):2691
https://doi.org/10.1121/1.4777252
