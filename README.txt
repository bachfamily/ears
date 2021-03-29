==================================================
ears
==================================================

**ears** is a library for `Max <http://cycling74.com/>`_ containing 
objects for elementary buffer manipulation.

**ears** is based on the `bach <http://www.bachproject.net/>`_ public API.

The official website is www.bachproject.net

The official repository is https://github.com/bachfamily/ears. You can find the source code there.


Dependencies
=================

• bach (https://github.com/bachfamily/bach)

• the mpg123 library 1.23.4 (released under LGPLv2.1). 
If you need to compile the Xcode project, the static library must be located at /usr/local/lib/libmpg123.a 

• The mp3 LAME library 3.1.00, licensed under the LGPL (If you need to compile the Xcode project, the static library must be located at /usr/local/lib/libmp3lame.a)

• WavPack (released under BSD license) 

• for the [ears.freeverb~] module: a slightly modified version of the Freeverb library for the freeverb algorithm (in the public domain)

• for the [ears.writetags] and [ears.readtags] modules: a modified version of the id3 library (released under GPLv2)

• for the [ears.rubberband~] module: the Rubberband library (released under GPLv2); a slightly modified version of commit f42a369 is included in the repository; the resampler used is libsamplerate (released under BSD license)

• for the [ears.ambi*~] modules: the HoaLibrary released under GPLv3, and the Eigen library, released under GPLv3

• for the [ears.essentia~] module: the Essentia 2.1_beta5 library (released under Affero GPL, compatible with GPLv3). If you need to compile the Xcode project, the static library must be located at /usr/local/lib/libessentia.a
The packageed version of Essentia has been modified to 
1) prevent the collision with functions named "error" (in debugger.h). The naming has been modified to "essentia_error". 
2) allow non-integer hopSize for the frameCutter algorithm
To build the library, you should use the lightweighted configuration, without dependencies. Enter the library folder and then:
./waf configure --build-static --fft='KISS' --lightweight=""
Notice we are using the KissFFT library because it is already a dependency of bach (see below). Then :
./waf
and finally:
./waf install

In turns, bach depends on
- A modified version of Simon Tatham's listsort (https://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.c), released under the terms of the MIT License.
- A modified version of The Mini-XML library version 2.7 (https://www.msweet.org/mxml/index.html), released under the terms of the GNU Lesser General Public License version 2 (LGPL-v2.0)
- The Kiss FFT library (https://github.com/mborgerding/kissfft), released under the terms of the BSD License.
- A modified version of the IRCAM SDIF library (http://sdif.sourceforge.net/extern/alpha-main.html), released under the terms of the GNU Lesser General Public License version 2 (LGPL-v2.0)


Related projects
=================

* `bach: automated composer's helper <http://www.bachproject.net>`__
* `cage <http://www.bachproject.net/cage>`__
* `dada: automated composer's helper <http://www.bachproject.net/dada>`__
* `Doctor Max <https://github.com/danieleghisi/DoctorMax>`__



Copyrights
==========

* Copyright (c) 2017-2021 Daniele Ghisi
