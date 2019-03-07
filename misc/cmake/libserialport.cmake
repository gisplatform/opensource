##
## Provide libserialport support
##

# Check for SERIALPORT
pkg_check_modules(SERIALPORT REQUIRED libserialport)
if(NOT SERIALPORT_FOUND)
   return()
endif(NOT SERIALPORT_FOUND)

#Add compiler flags
add_definitions(${SERIALPORT_CFLAGS})
add_definitions(${SERIALPORT_CFLAGS_OTHER})
link_directories(${SERIALPORT_LIBRARY_DIRS})

