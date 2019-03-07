##
## Provide libpulse-simple support
##

# Check for PULSE_SIMPLE
pkg_check_modules(PULSE_SIMPLE REQUIRED libpulse-simple)
if(NOT PULSE_SIMPLE_FOUND)
   return()
endif(NOT PULSE_SIMPLE_FOUND)

#Add compiler flags
add_definitions(${PULSE_SIMPLE_CFLAGS})
add_definitions(${PULSE_SIMPLE_CFLAGS_OTHER})
link_directories(${PULSE_SIMPLE_LIBRARY_DIRS})

