##
## Provide sqlite3 support
##

# Check for SQLITE3
pkg_check_modules(SQLITE3 REQUIRED sqlite3)
if(NOT SQLITE3_FOUND)
   return()
endif(NOT SQLITE3_FOUND)

#Add compiler flags
add_definitions(${SQLITE3_CFLAGS})
add_definitions(${SQLITE3_CFLAGS_OTHER})
link_directories(${SQLITE3_LIBRARY_DIRS})

