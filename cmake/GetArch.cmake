# ~~~
# Summary:      Set ARCH using cmake probes and various heuristics.
# License:      GPLv3+
# Copyright (c) 2021 Alec Leamas
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.


if (COMMAND GetArch)
  return()
endif ()

#~ # Based on code from nohal
#~ function (GetArch)
  #~ if (NOT "${OCPN_TARGET_TUPLE}" STREQUAL "")
    #~ # Return last element from tuple like "Android-armhf;16;armhf"
    #~ list(GET OCPN_TARGET_TUPLE 2 ARCH)
  #~ elseif (NOT WIN32)
    #~ # default
    #~ set(ARCH "x86_64")
    #~ if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
      #~ if (CMAKE_SIZEOF_VOID_P MATCHES "8")
        #~ set(ARCH "arm64")
      #~ else ()
        #~ set(ARCH "armhf")
      #~ endif ()
    #~ else (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
      #~ set(ARCH ${CMAKE_SYSTEM_PROCESSOR})
    #~ endif ()
    #~ if ("${BUILD_TYPE}" STREQUAL "flatpak")
      #~ if (ARCH STREQUAL "arm64")
        #~ set(ARCH "aarch64")
      #~ endif ()
    #~ elseif (EXISTS /etc/redhat-release)
      #~ if (ARCH STREQUAL "arm64")
        #~ set(ARCH "aarch64")
      #~ endif ()
    #~ elseif (EXISTS /etc/suse-release OR EXISTS /etc/SuSE-release)
      #~ if (ARCH STREQUAL "arm64")
        #~ set(ARCH "aarch64")
      #~ endif ()
    #~ endif ()
  #~ else (NOT WIN32)
    #~ # Should really be i386 since we are on win32. However, it's x86_64 for now,
    #~ # see #2027
    #~ set(ARCH "x86_64")
  #~ endif ()
  #~ set(ARCH ${ARCH} PARENT_SCOPE)
#~ endfunction (GetArch)

    function (GetArch)

## START pmx-2022.02.21 Upate to adapt to the schizophrenic arm64/aarch64 duality
    set(ARCH ${CMAKE_SYSTEM_PROCESSOR})
    
    message(STATUS "${CMLOC}*** DETECTED ARCH ${ARCH}  ***")
    message(STATUS "${CMLOC}*** OCPN_TARGET_TUPLE ${OCPN_TARGET_TUPLE}  ***")

      if (NOT "${OCPN_TARGET_TUPLE}" STREQUAL "")
        # Return last element from tuple like "Android-armhf;16;armhf"
        list(GET OCPN_TARGET_TUPLE 2 ARCH)

      elseif(UNIX AND NOT APPLE)

        message(STATUS "${CMLOC}*** Will install to ${CMAKE_INSTALL_PREFIX}  ***")

        if(EXISTS /etc/debian_version)
            message(STATUS "${CMLOC}*** Debian detected  ***")
            set(PACKAGE_FORMAT "DEB")
            set(PACKAGE_DEPS "libc6, libwxgtk3.0-0, wx3.0-i18n, libglu1-mesa (>= 7.0.0), libgl1-mesa-glx (>= 7.0.0), zlib1g, bzip2, libportaudio2")
            set(PACKAGE_RECS "xcalib,xdg-utils")
            set(LIB_INSTALL_DIR "lib")
            if(ARCH MATCHES "arm64|aarch64")
                add_definitions(-DOCPN_ARM64)
            elseif(ARCH MATCHES "armhf")
                add_definitions(-DOCPN_ARMHF)
            elseif(ARCH MATCHES "x86_64")
                set(ARCH_DEB "amd64")
            endif()
        endif(EXISTS /etc/debian_version)
 ## END pmx-2022.02.21 Upate to adapt to the schizophrenic arm64/aarch64 duality
       
        if(NOT DEFINED PACKAGE_FORMAT)
            if(EXISTS /app)
                message(STATUS "*** Flatpak detected  ***")
                set(PACKAGE_FORMAT "TGZ")
                set(ARCH "aarch64")
                set(LIB_INSTALL_DIR "lib")
            endif(EXISTS /app)
        endif(NOT DEFINED PACKAGE_FORMAT)

        if(NOT DEFINED PACKAGE_FORMAT)
            if(EXISTS /etc/redhat-release)
                message(STATUS "${CMLOC}*** Redhat detected  ***")
                set(PACKAGE_FORMAT "RPM")
                set(PACKAGE_DEPS "opencpn")
                if(ARCH MATCHES "x86_64")
                    set(LIB_INSTALL_DIR "lib64")
                elseif(ARCH MATCHES "i386")
                    set(LIB_INSTALL_DIR "lib")
                endif()
            endif(EXISTS /etc/redhat-release)
        endif(NOT DEFINED PACKAGE_FORMAT)

        if(NOT DEFINED PACKAGE_FORMAT)
            if(EXISTS /etc/os-release
               OR EXISTS /etc/sysconfig/SuSEfirewall2.d
               OR EXISTS /etc/suse-release
               OR EXISTS /etc/SuSE-release)
                message(STATUS "${CMLOC}*** OpenSUSE detected  ***")
                set(PACKAGE_FORMAT "RPM")
                set(PACKAGE_DEPS "opencpn")
                if(ARCH MATCHES "x86_64")
                    set(LIB_INSTALL_DIR "lib64")
                elseif(ARCH MATCHES "i386")
                    set(LIB_INSTALL_DIR "lib")
                endif()
            endif(
                EXISTS /etc/os-release
                OR EXISTS /etc/sysconfig/SuSEfirewall2.d
                OR EXISTS /etc/suse-release
                OR EXISTS /etc/SuSE-release)
        endif(NOT DEFINED PACKAGE_FORMAT)

    endif()

    if(APPLE)
        set(ARCH "x86_64")
    endif(APPLE)

#~ else(NOT WIN32)
    #~ set(ARCH "x86_64")
#~ endif(NOT WIN32)

  message(STATUS "${CMLOC}ARCH: ${ARCH}")
  set(ARCH ${ARCH} PARENT_SCOPE)

endfunction (GetArch)

getarch()
