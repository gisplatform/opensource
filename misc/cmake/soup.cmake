##
## Provide libsoup-2.4 support
##

# Check for LIBSOUP
pkg_check_modules(LIBSOUP REQUIRED libsoup-2.4)
if(NOT LIBSOUP_FOUND)
   return()
endif(NOT LIBSOUP_FOUND)

#Add compiler flags
add_definitions(${LIBSOUP_CFLAGS})
add_definitions(${LIBSOUP_CFLAGS_OTHER})
link_directories(${LIBSOUP_LIBRARY_DIRS})

