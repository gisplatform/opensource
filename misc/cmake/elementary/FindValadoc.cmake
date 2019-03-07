##
# Реализовано аналогично FindVala.cmake
##

set( ENV{PATH} "/opt/gtk/bin:$ENV{PATH}" )
# Search for the valadoc executable in the usual system paths.
find_program(VALADOC_EXECUTABLE
  NAMES valadoc)

# Handle the QUIETLY and REQUIRED arguments, which may be given to the find call.
# Furthermore set VALADOC_FOUND to TRUE if Valadoc has been found (aka.
# VALADOC_EXECUTABLE is set)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Valadoc DEFAULT_MSG VALADOC_EXECUTABLE)

mark_as_advanced(VALADOC_EXECUTABLE)

# Determine the valadoc version
if(VALADOC_FOUND)
  execute_process(COMMAND ${VALADOC_EXECUTABLE} "--version" 
                  OUTPUT_VARIABLE "VALADOC_VERSION")
  string(REPLACE "Valadoc" "" "VALADOC_VERSION" ${VALADOC_VERSION})
  string(STRIP ${VALADOC_VERSION} "VALADOC_VERSION")
else(VALADOC_FOUND)
  message(WARNING "Failed to find Valadoc, can't build documentation!")
endif(VALADOC_FOUND)

