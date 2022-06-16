if (NOT EXISTS "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/build/install_manifest.txt")
  message(FATAL_ERROR "Cannot find install manifest: \"C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/build/install_manifest.txt\"")
endif()

file(READ "C:/Users/aa/Documents/Max 8/Packages/ears/source/lib/taglib-1.12-win/build/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")
foreach (file ${files})
  message(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
  if (EXISTS "$ENV{DESTDIR}${file}")
    execute_process(
      COMMAND C:/Program Files/CMake/bin/cmake.exe -E remove "$ENV{DESTDIR}${file}"
      OUTPUT_VARIABLE rm_out
      RESULT_VARIABLE rm_retval
    )
    if(NOT ${rm_retval} EQUAL 0)
      message(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
    endif ()
  else ()
    message(STATUS "File \"$ENV{DESTDIR}${file}\" does not exist.")
  endif ()
endforeach()
