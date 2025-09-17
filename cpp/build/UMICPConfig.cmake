
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was UMICPConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

# UMICP CMake Configuration File

set(UMICP_VERSION "1.0.0")

# Find dependencies
find_dependency(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(JSON_C REQUIRED json-c)
    pkg_check_modules(OPENSSL REQUIRED openssl)
    pkg_check_modules(ZLIB REQUIRED zlib)
endif()

# Include targets
include("${CMAKE_CURRENT_LIST_DIR}/UMICPTargets.cmake")

check_required_components(UMICP)
