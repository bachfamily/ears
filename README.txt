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

For building ears, we provide project files for Xcode and Visual Studio 2022.
Detailed instructions for building on Mac are provided below.
For Visual Studio, unless specified otherwise, all is taken care of by the Visual Studio project.

• bach (https://github.com/bachfamily/bach)

• the Essentia 2.1_beta5 library (released under Affero GPL, compatible with GPLv3). The essentia library is not needed for the whole project, but for a certain number of its modules (including ears.essentia~, ears.cqt~, ears.tempogram~, ears.peaks~ and all the ears.model.*~).
If you need to compile the Xcode project, the static library must be located at /usr/local/lib/libessentia.a
The packaged version of Essentia has been modified to
1) prevent the collision with functions named "error" (in debugger.h). The naming has been modified to "essentia_error".
2) allow non-integer hopSize for the frameCutter algorithm
3) fix some little bugs (e.g. parameters of the SPS model were not passed to the internal sinusoidal model).

To build the library, you should use the lightweight configuration, without dependencies.
If you're on a Mac Intel (but NOT on a Silicon machine: for this see below!), enter the library folder (source/lib/essentia-2.1_beta5-modif) and then:

    ./waf configure --build-static --fft='KISS' --lightweight=""
    ./waf
    ./waf install

If you're on an Apple Silicon machine, in principle by running the line above you should get a FAT build of the library containing the executable code for both native arm64 and the Rosetta emulation. In practice, this is not what happens, as for some reason the x86_64 version in the FAT container has many undefined symbols. The only way we found to makes things work is as follows:

1) first of all make a "clean" copy of the whole library folder, and keep it somewhere
2) enter the library folder, and then
    MACOSX_DEPLOYMENT_TARGET=10.11 ./waf configure --build-static --fft='KISS' --lightweight="" --arch=arm64 --no-msse
    MACOSX_DEPLOYMENT_TARGET=10.11 ./waf
    MACOSX_DEPLOYMENT_TARGET=10.11 ./waf install
    mv /usr/local/lib/libessentia.a /usr/local/lib/libessentia_arm64.a
I don't see how at this stage the leading MACOSX_DEPLOYMENT_TARGET=10.11 makes any difference but it works in this way, so let's carry on.
3) If you only want to build the native arm64 version, you're done. Keep in mind that in this case you must edit the target architecture for ears.essentia~ in the Xcode project. If you want to build both architectures instead, delete the library folder and replace it with the copy you kept aside. There might be a more elegant way to clean everything up, but this one works for sure.
4) Re-enter the library folder, and then
    arch -x86_64 zsh
    MACOSX_DEPLOYMENT_TARGET=10.11 ./waf configure --build-static --fft='KISS' --lightweight=""
    MACOSX_DEPLOYMENT_TARGET=10.11 ./waf
    MACOSX_DEPLOYMENT_TARGET=10.11 ./waf install
    cd /usr/local/lib
    lipo libessentia.a libessentia_arm64.a -create -output libessentia.a

Notice we are using the KissFFT library because it is already a dependency of bach (see below).


• For the [ears.write~] and [ears.read~] module: the TagLib (https://taglib.org/, released under LGPL)
A version of the library is included in the repository. On a Mac, simply enter the folder source/lib/taglib-1.12/
then

    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" -DBUILD_SHARED_LIBS=OFF .
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
- Fixed a bug in lpcm_dequant() which gave reversed buffers for 24 bit files
You do not need to install the library, because the sources are directly compiled in the project.


• For the [ears.write~] and [ears.read~] module: the AudioFile library (released under GPLv3)
A modified version of the library is included in the repository.
The modifications extend the functionalities of the library in order to support marker and cues in WAV files.
You do not need to compile or install the library separately: sources are directly compiled within the project.

• the mpg123 library 1.23.4 (released under LGPLv2.1).
We have tested with versio 1.29.3 (and, previously with version 1.23.4). It may work with following versions also. The source code is, for convenience, also in the source/lib/ folder (if you want to try a later version you can download it from https://www.mpg123.de), but importantly there is an issue in the "make" portion of the procedure if your path has spaces. So first of all make sure you copy the folder in a path position with no spaces, then enter the folder and run
MACOSX_DEPLOYMENT_TARGET=10.11 ./configure --enable-static=yes
MACOSX_DEPLOYMENT_TARGET=10.11 make
MACOSX_DEPLOYMENT_TARGET=10.11 make install
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
We have tested with version 3.100. A slightly modified version is included in the repository, containing the only modification needed to compile the library, namely: remove the line containing 'lame_init_old' from the file 'include/libmp3lame.sym'. If you take the version of the library in source/lib/, this is already taken care of, otherwise take care of it, make sure to copy the folder in a path with no spaces in it, then enter the folder and run:
MACOSX_DEPLOYMENT_TARGET=10.11 ./configure
MACOSX_DEPLOYMENT_TARGET=10.11 make
MACOSX_DEPLOYMENT_TARGET=10.11 make install
If you need to compile the Xcode project, the static library must be located at /usr/local/lib/libmp3lame.a
If you need to build it for Apple Silicon, you need to
1. Run the commands above
2. Rename the library (e.g. libmp3lame.arm64)
3. Open a terminal under Rosetta 2: arch -x86_64 zsh
4. Clean the temporary build locations (or just recreate a new source code folder)
5. Run again the commands above
6. Create a fat bundle out of the two single-platform binaries:
 cd /usr/local/lib
 lipo libmp3lame.a libmp3lame.arm64 -create -output libmp3lame.a
7. Remove libmp3lame.arm64



• WavPack (released under BSD license)
You do not need to compile or install the library separately: sources are directly compiled within the project.

• for the [ears.freeverb~] module: a slightly modified version of the Freeverb library for the freeverb algorithm (in the public domain)
You do not need to compile or install the library separately: sources are directly compiled within the project.

• for the [ears.write~] and [ears.read~] modules: a modified version of the id3 library (released under GPLv2)
You do not need to compile or install the library separately: sources are directly compiled within the project.

• for the [ears.rubberband~] module: the Rubberband library (released under GPLv2); a slightly modified version of commit f42a369 is included in the repository.
You do not need to compile or install the library separately: sources are directly compiled within the project.
However you may need to install the resampler dependency. The resampler used is libsamplerate (released under BSD license), which you must install from Homebrew on Apple Silicon, following the same steps provided for the lame library (download separately the arm64 and x86 version, and combine them with lipo).
On Apple Silicon:
1) type "brew install libsamplerate". This creates /opt/homebrew/lib/libsamplerate.a
2) enter rosetta with "arch -x86_46 zsh", and use the Rosetta homebrew version you previously installed and type "/usr/local/Homebrew/bin/brew install libsamplerate".
This creates  /usr/local/lib/libsamplerate.a
3) keep a copy of the x86 lib if you want
    cp  /usr/local/lib/libsamplerate.a /usr/local/lib/libsamplerate_x86_64.a
3) then combine them
    lipo /usr/local/lib/libsamplerate.a /opt/homebrew/lib/libsamplerate.a -create -output /usr/local/lib/libsamplerate.a

• for the [ears.hoa.*~] modules: the HoaLibrary released under GPLv3, and the Eigen library, released under GPLv3
You do not need to compile or install the HOA and Eigen libraries separately: sources are directly compiled within the project.
On the other hand, the Boost library is required. On a Mac, it can by installed through Homebrew (no need for separate procedures for Intel and M1, since we only use the header-only part).
On Windows, ears uses the same version (and in the same location) as the dada library.

• for the [ears.soundtouch~] module: the SoundTouch library, released under LGPL v2.1
You do not need to compile or install the library separately: sources are directly compiled within the project.

• for the [ears.vamp~] module: a slightly modified version of the VAMP Plugin and host SDK, which is located inside the repository (source/lib/vamp-plugin-sdk-master).
To compile on a Mac Intel, go to the library folder and run
make -f build/Makefile.osx
To compile on a Mac M1, go to the library folder and run
make -f build/Makefile.osxm1 sdk
Note that, on the M1, building the vamp example plugins is potentially problematic because of their dependencies that should all be built for the arm64 architecture.


-------------------------

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

* Copyright (c) 2017-2022 Daniele Ghisi and Andrea Agostini
