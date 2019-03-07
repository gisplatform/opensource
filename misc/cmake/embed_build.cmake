##
## In this file checked variables EMBED_BUILD_*
##

get_cmake_property( _vars VARIABLES )

foreach( _var ${_vars} )
  string( REGEX MATCH "^EMBED_BUILD" item ${_var} )
  if( item )
    set( EMBED_BUILD "YES" )
  endif( item )
endforeach()
