##
## Provide libgps support
##

# Check for LIBGPS
pkg_check_modules( LIBGPS REQUIRED libgps)
if( NOT LIBGPS_FOUND )
   return()
endif( NOT LIBGPS_FOUND )


#Add compiler flags
add_definitions( ${LIBGPS_CFLAGS} )
add_definitions( ${LIBGPS_CFLAGS_OTHER} )

link_directories( ${LIBGPS_LIBRARY_DIRS} )

