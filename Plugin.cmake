# ~~~
# Summary:      Local, non-generic plugin setup
# Copyright (c) 2020-2021 Mike Rossiter
# License:      GPLv3+
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.



# -------- Options ----------

set(OCPN_TEST_REPO
    "opencpn/aismet_hydro-alpha"
    CACHE STRING "Default repository for untagged builds"
)
set(OCPN_BETA_REPO
    "opencpn/aismet_hydro-beta"
    CACHE STRING 
    "Default repository for tagged builds matching 'beta'"
)
set(OCPN_RELEASE_REPO
    "opencpn/aismet_hydro-prod"
    CACHE STRING 
    "Default repository for tagged builds not matching 'beta'"
)
option(aisMET_HYDRO_USE_SVG "Use SVG graphics" ON)

#
#
# -------  Plugin setup --------
#
set(PKG_NAME aisMET_HYDRO_pi)
set(PKG_VERSION  0.1.0)
set(PKG_PRERELEASE "")  # Empty, or a tag like 'beta'

set(DISPLAY_NAME aisMET_HYDRO)    # Dialogs, installer artifacts, ...
set(PLUGIN_API_NAME aisMET_HYDRO) # As of GetCommonName() in plugin API
set(PKG_SUMMARY "Met and hydro AIS binary messages")
set(PKG_DESCRIPTION [=[
Read AIS binary messages for met and hydro data.
]=])

set(PKG_AUTHOR "Mike Rossiter")
set(PKG_IS_OPEN_SOURCE "yes")
set(PKG_HOMEPAGE https://github.com/Rasbats/aisMET_HYDRO_pi)
set(PKG_INFO_URL https://opencpn.org/OpenCPN/plugins/aisMET_HYDRO.html)

set(SRC
	src/ais.h
	src/ais8.cpp
	src/ais_bitset2.cpp
	src/AIS_Bitstring.cpp
	src/AIS_Bitstring.h
	src/ais.cpp
    src/AisMaker.cpp
    src/AisMaker.h
    src/aisMET_HYDROgui.cpp
    src/aisMET_HYDROgui.h
    src/aisMET_HYDROgui_impl.cpp
    src/aisMET_HYDROgui_impl.h
    src/aisMET_HYDRO_pi.cpp
    src/aisMET_HYDRO_pi.h
    src/icons.cpp
    src/icons.h
	src/signal.cpp
	src/signal.h
	src/widget.cpp
	src/widget.h
	src/bbox.cpp
    src/bbox.h	
	src/AISdisplay.cpp
	src/AISdisplay.h
	src/ASMmessages.cpp
	src/ASMmessages.h
)

set(PKG_API_LIB api-16)  #  A directory in libs/ e. g., api-17 or api-16

macro(late_init)
  # Perform initialization after the PACKAGE_NAME library, compilers
  # and ocpn::api is available.
  if (aisMET_HYDRO_USE_SVG)
    target_compile_definitions(${PACKAGE_NAME} PUBLIC aisMET_HYDRO_USE_SVG)
  endif ()
endmacro ()

macro(add_plugin_libraries)
  # Add libraries required by this plugin
  add_subdirectory("libs/tinyxml")
  target_link_libraries(${PACKAGE_NAME} ocpn::tinyxml)

  add_subdirectory("libs/wxJSON")
  target_link_libraries(${PACKAGE_NAME} ocpn::wxjson)

  add_subdirectory("libs/plugingl")
  target_link_libraries(${PACKAGE_NAME} ocpn::plugingl)

  add_subdirectory("libs/jsoncpp")
  target_link_libraries(${PACKAGE_NAME} ocpn::jsoncpp)

endmacro ()
