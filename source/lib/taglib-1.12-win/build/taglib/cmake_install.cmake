# Install script for directory: C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/taglib")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/taglib/lib/tag.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
      message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
      message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    file(INSTALL DESTINATION "C:/Program Files (x86)/taglib/lib" TYPE STATIC_LIBRARY FILES "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/build/taglib/Debug/tag.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/taglib/lib/tag.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
      message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
      message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    file(INSTALL DESTINATION "C:/Program Files (x86)/taglib/lib" TYPE STATIC_LIBRARY FILES "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/build/taglib/Release/tag.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/taglib/lib/tag.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
      message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
      message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    file(INSTALL DESTINATION "C:/Program Files (x86)/taglib/lib" TYPE STATIC_LIBRARY FILES "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/build/taglib/MinSizeRel/tag.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/taglib/lib/tag.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
      message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
      message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    file(INSTALL DESTINATION "C:/Program Files (x86)/taglib/lib" TYPE STATIC_LIBRARY FILES "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/build/taglib/RelWithDebInfo/tag.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/taglib/include/taglib/tag.h;C:/Program Files (x86)/taglib/include/taglib/fileref.h;C:/Program Files (x86)/taglib/include/taglib/audioproperties.h;C:/Program Files (x86)/taglib/include/taglib/taglib_export.h;C:/Program Files (x86)/taglib/include/taglib/taglib_config.h;C:/Program Files (x86)/taglib/include/taglib/taglib.h;C:/Program Files (x86)/taglib/include/taglib/tstring.h;C:/Program Files (x86)/taglib/include/taglib/tlist.h;C:/Program Files (x86)/taglib/include/taglib/tlist.tcc;C:/Program Files (x86)/taglib/include/taglib/tstringlist.h;C:/Program Files (x86)/taglib/include/taglib/tbytevector.h;C:/Program Files (x86)/taglib/include/taglib/tbytevectorlist.h;C:/Program Files (x86)/taglib/include/taglib/tbytevectorstream.h;C:/Program Files (x86)/taglib/include/taglib/tiostream.h;C:/Program Files (x86)/taglib/include/taglib/tfile.h;C:/Program Files (x86)/taglib/include/taglib/tfilestream.h;C:/Program Files (x86)/taglib/include/taglib/tmap.h;C:/Program Files (x86)/taglib/include/taglib/tmap.tcc;C:/Program Files (x86)/taglib/include/taglib/tpropertymap.h;C:/Program Files (x86)/taglib/include/taglib/trefcounter.h;C:/Program Files (x86)/taglib/include/taglib/tdebuglistener.h;C:/Program Files (x86)/taglib/include/taglib/mpegfile.h;C:/Program Files (x86)/taglib/include/taglib/mpegproperties.h;C:/Program Files (x86)/taglib/include/taglib/mpegheader.h;C:/Program Files (x86)/taglib/include/taglib/xingheader.h;C:/Program Files (x86)/taglib/include/taglib/id3v1tag.h;C:/Program Files (x86)/taglib/include/taglib/id3v1genres.h;C:/Program Files (x86)/taglib/include/taglib/id3v2.h;C:/Program Files (x86)/taglib/include/taglib/id3v2extendedheader.h;C:/Program Files (x86)/taglib/include/taglib/id3v2frame.h;C:/Program Files (x86)/taglib/include/taglib/id3v2header.h;C:/Program Files (x86)/taglib/include/taglib/id3v2synchdata.h;C:/Program Files (x86)/taglib/include/taglib/id3v2footer.h;C:/Program Files (x86)/taglib/include/taglib/id3v2framefactory.h;C:/Program Files (x86)/taglib/include/taglib/id3v2tag.h;C:/Program Files (x86)/taglib/include/taglib/attachedpictureframe.h;C:/Program Files (x86)/taglib/include/taglib/commentsframe.h;C:/Program Files (x86)/taglib/include/taglib/eventtimingcodesframe.h;C:/Program Files (x86)/taglib/include/taglib/generalencapsulatedobjectframe.h;C:/Program Files (x86)/taglib/include/taglib/ownershipframe.h;C:/Program Files (x86)/taglib/include/taglib/popularimeterframe.h;C:/Program Files (x86)/taglib/include/taglib/privateframe.h;C:/Program Files (x86)/taglib/include/taglib/relativevolumeframe.h;C:/Program Files (x86)/taglib/include/taglib/synchronizedlyricsframe.h;C:/Program Files (x86)/taglib/include/taglib/textidentificationframe.h;C:/Program Files (x86)/taglib/include/taglib/uniquefileidentifierframe.h;C:/Program Files (x86)/taglib/include/taglib/unknownframe.h;C:/Program Files (x86)/taglib/include/taglib/unsynchronizedlyricsframe.h;C:/Program Files (x86)/taglib/include/taglib/urllinkframe.h;C:/Program Files (x86)/taglib/include/taglib/chapterframe.h;C:/Program Files (x86)/taglib/include/taglib/tableofcontentsframe.h;C:/Program Files (x86)/taglib/include/taglib/podcastframe.h;C:/Program Files (x86)/taglib/include/taglib/oggfile.h;C:/Program Files (x86)/taglib/include/taglib/oggpage.h;C:/Program Files (x86)/taglib/include/taglib/oggpageheader.h;C:/Program Files (x86)/taglib/include/taglib/xiphcomment.h;C:/Program Files (x86)/taglib/include/taglib/vorbisfile.h;C:/Program Files (x86)/taglib/include/taglib/vorbisproperties.h;C:/Program Files (x86)/taglib/include/taglib/oggflacfile.h;C:/Program Files (x86)/taglib/include/taglib/speexfile.h;C:/Program Files (x86)/taglib/include/taglib/speexproperties.h;C:/Program Files (x86)/taglib/include/taglib/opusfile.h;C:/Program Files (x86)/taglib/include/taglib/opusproperties.h;C:/Program Files (x86)/taglib/include/taglib/flacfile.h;C:/Program Files (x86)/taglib/include/taglib/flacpicture.h;C:/Program Files (x86)/taglib/include/taglib/flacproperties.h;C:/Program Files (x86)/taglib/include/taglib/flacmetadatablock.h;C:/Program Files (x86)/taglib/include/taglib/apefile.h;C:/Program Files (x86)/taglib/include/taglib/apeproperties.h;C:/Program Files (x86)/taglib/include/taglib/apetag.h;C:/Program Files (x86)/taglib/include/taglib/apefooter.h;C:/Program Files (x86)/taglib/include/taglib/apeitem.h;C:/Program Files (x86)/taglib/include/taglib/mpcfile.h;C:/Program Files (x86)/taglib/include/taglib/mpcproperties.h;C:/Program Files (x86)/taglib/include/taglib/wavpackfile.h;C:/Program Files (x86)/taglib/include/taglib/wavpackproperties.h;C:/Program Files (x86)/taglib/include/taglib/trueaudiofile.h;C:/Program Files (x86)/taglib/include/taglib/trueaudioproperties.h;C:/Program Files (x86)/taglib/include/taglib/rifffile.h;C:/Program Files (x86)/taglib/include/taglib/aifffile.h;C:/Program Files (x86)/taglib/include/taglib/aiffproperties.h;C:/Program Files (x86)/taglib/include/taglib/wavfile.h;C:/Program Files (x86)/taglib/include/taglib/wavproperties.h;C:/Program Files (x86)/taglib/include/taglib/infotag.h;C:/Program Files (x86)/taglib/include/taglib/asffile.h;C:/Program Files (x86)/taglib/include/taglib/asfproperties.h;C:/Program Files (x86)/taglib/include/taglib/asftag.h;C:/Program Files (x86)/taglib/include/taglib/asfattribute.h;C:/Program Files (x86)/taglib/include/taglib/asfpicture.h;C:/Program Files (x86)/taglib/include/taglib/mp4file.h;C:/Program Files (x86)/taglib/include/taglib/mp4atom.h;C:/Program Files (x86)/taglib/include/taglib/mp4tag.h;C:/Program Files (x86)/taglib/include/taglib/mp4item.h;C:/Program Files (x86)/taglib/include/taglib/mp4properties.h;C:/Program Files (x86)/taglib/include/taglib/mp4coverart.h;C:/Program Files (x86)/taglib/include/taglib/modfilebase.h;C:/Program Files (x86)/taglib/include/taglib/modfile.h;C:/Program Files (x86)/taglib/include/taglib/modtag.h;C:/Program Files (x86)/taglib/include/taglib/modproperties.h;C:/Program Files (x86)/taglib/include/taglib/itfile.h;C:/Program Files (x86)/taglib/include/taglib/itproperties.h;C:/Program Files (x86)/taglib/include/taglib/s3mfile.h;C:/Program Files (x86)/taglib/include/taglib/s3mproperties.h;C:/Program Files (x86)/taglib/include/taglib/xmfile.h;C:/Program Files (x86)/taglib/include/taglib/xmproperties.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "C:/Program Files (x86)/taglib/include/taglib" TYPE FILE FILES
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/tag.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/fileref.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/audioproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/taglib_export.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/build/taglib/../taglib_config.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/taglib.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/tstring.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/tlist.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/tlist.tcc"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/tstringlist.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/tbytevector.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/tbytevectorlist.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/tbytevectorstream.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/tiostream.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/tfile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/tfilestream.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/tmap.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/tmap.tcc"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/tpropertymap.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/trefcounter.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/toolkit/tdebuglistener.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/mpegfile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/mpegproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/mpegheader.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/xingheader.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v1/id3v1tag.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v1/id3v1genres.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/id3v2.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/id3v2extendedheader.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/id3v2frame.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/id3v2header.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/id3v2synchdata.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/id3v2footer.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/id3v2framefactory.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/id3v2tag.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/attachedpictureframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/commentsframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/eventtimingcodesframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/generalencapsulatedobjectframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/ownershipframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/popularimeterframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/privateframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/relativevolumeframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/synchronizedlyricsframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/textidentificationframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/uniquefileidentifierframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/unknownframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/unsynchronizedlyricsframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/urllinkframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/chapterframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/tableofcontentsframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpeg/id3v2/frames/podcastframe.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ogg/oggfile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ogg/oggpage.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ogg/oggpageheader.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ogg/xiphcomment.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ogg/vorbis/vorbisfile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ogg/vorbis/vorbisproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ogg/flac/oggflacfile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ogg/speex/speexfile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ogg/speex/speexproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ogg/opus/opusfile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ogg/opus/opusproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/flac/flacfile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/flac/flacpicture.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/flac/flacproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/flac/flacmetadatablock.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ape/apefile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ape/apeproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ape/apetag.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ape/apefooter.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/ape/apeitem.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpc/mpcfile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mpc/mpcproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/wavpack/wavpackfile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/wavpack/wavpackproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/trueaudio/trueaudiofile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/trueaudio/trueaudioproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/riff/rifffile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/riff/aiff/aifffile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/riff/aiff/aiffproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/riff/wav/wavfile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/riff/wav/wavproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/riff/wav/infotag.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/asf/asffile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/asf/asfproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/asf/asftag.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/asf/asfattribute.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/asf/asfpicture.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mp4/mp4file.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mp4/mp4atom.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mp4/mp4tag.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mp4/mp4item.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mp4/mp4properties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mp4/mp4coverart.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mod/modfilebase.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mod/modfile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mod/modtag.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/mod/modproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/it/itfile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/it/itproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/s3m/s3mfile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/s3m/s3mproperties.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/xm/xmfile.h"
    "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/taglib/xm/xmproperties.h"
    )
endif()

