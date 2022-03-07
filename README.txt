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

• the Essentia 2.1_beta5 library (released under Affero GPL, compatible with GPLv3). If you need to compile the Xcode project, the static library must be located at /usr/local/lib/libessentia.a
The packaged version of Essentia has been modified to 
1) prevent the collision with functions named "error" (in debugger.h). The naming has been modified to "essentia_error". 
2) allow non-integer hopSize for the frameCutter algorithm
3) fix some little bugs (e.g. parameters of the SPS model were not passed to the internal sinusoidal model).

To build the library, you should use the lightweight configuration, without dependencies. 
If you're on a Mac Intel, enter the library folder and then:

    ./waf configure --build-static --fft='KISS' --lightweight=""
    ./waf
    ./waf install

If you're on an Apple Silicon machine, in principle by running the line above you should get a FAT build of the library containing the executable code for both native arm64 and the Rosetta emulation. In practice, this is not what happens, as for some reason the x86_64 version in the FAT container has many undefined symbols. The only way we found to makes things work is as follows:

1) first of all make a "clean" copy of the whole library folder, and keep it somewhere
2) enter the library folder, and then
    ./waf configure --build-static --fft='KISS' --lightweight="" --arch=arm64 --no-msse
    ./waf
    ./waf install
    mv /usr/local/lib/libessentia.a /usr/local/lib/libessentia_arm64.a
3) If you only want to build the native arm64 version, you're done. Keep in mind that in this case you must edit the target architecture for ears.features~ in the Xcode project. If you want to build both architecture instead, delete the library folder and replace it with the copy you kept aside. There might be a more elegant way to clean everything up, but this one works for sure. 
4) Re-enter the library folder, and then
    arch -x64_64 zsh
    ./waf configure --build-static --fft='KISS' --lightweight=""
    ./waf
    ./waf install
    cd /usr/local/lib   
    lipo libessentia_backup.a -extract arm64 -output libessentia_arm64.a

Notice we are using the KissFFT library because it is already a dependency of bach (see below). 


• For the [ears.write~] and [ears.read~] module: the TagLib (https://taglib.org/, released under LGPL)
A version of the library is included in the repository. On a Mac, simply enter the folder source/lib/taglib-1.12/
then 

    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 -DCMAKE_OSX_ARCHITECTURES="i386;x86_64;arm64" -DBUILD_SHARED_LIBS=OFF .
    make
    make install

• For the [ears.write~] and [ears.read~] module: the LibAIFF library (http://aifftools.sourceforge.net/libaiff/, released under MIT License, compatible with GPLv3)
A modified version of the library is included in the repository.
The modifications are the following:
– Comment line 180 "Unprepare(p)" in libaiff.c
– Added
    r->buffer2 = NULL;
    r->buflen2 = 0;
at line 152 of libaiff.c
You do not need to install the library, because the sources are directly compiled in the project.


• For the [ears.write~] and [ears.read~] module: the AudioFile library released under GPLv3)
A modified version of the library is included in the repository.
The modifications extend the functionalities of the library in order to support marker and cues in WAV files.

• the mpg123 library 1.23.4 (released under LGPLv2.1). To do so, run
./configure --enable-static=yes
make install
make
If you need to compile the Xcode project, the static library must be located at /usr/local/lib/libmpg123.a 
If you need to build it for Apple Silicon, you need to
1. Run the commands above
2. Rename the library (e.g. libmpg123.arm64)
3. Open a terminal under Rosetta 2: arch -x86_64 zsh
4. Clean the temporary build locations (or just recreate a new source code folder)
5. Run again the commands above
6. Create a fat bundle out of the two single-platform binaries:
 cd /usr/local/lib 
 lipo libmpg123.a libmpg123.arm64 -create -output libmpg123.a
7. Remove libmpg123.arm64

• The mp3 LAME library 3.1.00, licensed under the LGPL (If you need to compile the Xcode project, the static library must be located at /usr/local/lib/libmp3lame.a)
On an Apple Silicon machine, you need to have two versions of Homebrew installed, respectively for the ARM64 and Intel architectures. The ARM64 is the default one, which you can install by running in the terminal the commands provided listed the Homebrew site home page. Once you've done this, you can run `brew install lame' as usual.
  Once you've done this, you must install Homebrew for the Intel architecture. To do so, run `arch -x86_64 zsh' in a terminal. This will open a zsh session under Rosetta 2. Now you have to install Homebrew anew, but the x64 version will be installed in a different location: /usr/local/Homebrew/bin . If you just call the system-wide brew, the arm64 version will be called, so now you have to run `/usr/local/Homebrew/bin/brew install lame'.
  At this point, you have two architecture-specific versions of the library installed, which you now need to pack in a single fat binary. The arm64 version is in /opt/homebrew/lib, the x86 version is in /usr/local/opt/lame/lib. So we have to run the following line:
    lipo /usr/local/opt/mpfr/lib/libmp3lame.a /opt/homebrew/lib/libmp3lame.a -create -output /usr/local/opt/lame/lib/libmp3lame.a
Now the file that previously contained the x86 version contains the fat binary, while the arm64 files has not been changed. This should allow you to compile everything.

• WavPack (released under BSD license) 

• for the [ears.freeverb~] module: a slightly modified version of the Freeverb library for the freeverb algorithm (in the public domain)

• for the [ears.writetags] and [ears.readtags] modules: a modified version of the id3 library (released under GPLv2)

• for the [ears.rubberband~] module: the Rubberband library (released under GPLv2); a slightly modified version of commit f42a369 is included in the repository; the resampler used is libsamplerate (released under BSD license), which you must install from Homebrew on Apple Silicon, following the same steps provided for the lame library (download separately the arm64 and x86 version, and combine them with lipo).

• for the [ears.ambi*~] modules: the HoaLibrary released under GPLv3, and the Eigen library, released under GPLv3



In turn, bach depends on
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
