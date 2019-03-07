##
# Реализовано аналогично FindVala.cmake
##

set( ENV{PATH} "/opt/gtk/bin:$ENV{PATH}" )
# Search for the vapigen executable in the usual system paths.
find_program(VAPI_GEN_EXECUTABLE
  NAMES vapigen)

# Handle the QUIETLY and REQUIRED arguments, which may be given to the find call.
# Furthermore set VAPI_GEN_FOUND to TRUE if VapiGen has been found (aka.
# VAPI_GEN_EXECUTABLE is set)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VapiGen DEFAULT_MSG VAPI_GEN_EXECUTABLE)

mark_as_advanced(VAPI_GEN_EXECUTABLE)

# Determine the vapigen version
if(VAPI_GEN_FOUND)
    execute_process(COMMAND ${VAPI_GEN_EXECUTABLE} "--version" 
                    OUTPUT_VARIABLE "VAPI_GEN_VERSION")
    string(REPLACE "Vala API Generator" "" "VAPI_GEN_VERSION" ${VAPI_GEN_VERSION})
    string(STRIP ${VAPI_GEN_VERSION} "VAPI_GEN_VERSION")
endif(VAPI_GEN_FOUND)
