#include "gp-trace.h"
#include <stdio.h>

void trace_callback( GLogLevelFlags log_level, const gchar *date_time, const gchar *source, const gchar *level, const gchar *message )
{

  printf( "callback: %s %s %s:\t%s\n", date_time, source, level, message );
  fflush( stdout );

}

int main( int argc, char **argv )
{

  gp_trace_init (&argc, &argv);
  gp_trace_set_callback (trace_callback);
  gp_trace_add_output (0, "callback");

  g_print( "g_print message\n" );
  g_printerr( "g_printerr message\n" );

  g_log( G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG, "all-level-message" );
  g_debug( "debug test" );
  g_log( G_LOG_DOMAIN, G_LOG_LEVEL_INFO , "info test" );
  g_message( "message test" );
  g_warning( "warning test" );
  g_critical( "critical test" );
  g_error( "error test" );

  return 0;

}
