/*
 * GTRACE - message output library, this library is part of GRTL.
 *
 * Copyright 2010 Andrei Fadeev, Petr Zankov
 *
 * This file is part of GRTL.
 *
 * GRTL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * GRTL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with GRTL. If not, see <http://www.gnu.org/licenses/>.
 *
*/

/*!
 * \file gtrace.c
 *
 * \author Andrei Fadeev, Petr Zankov
 * \date 22.07.2009
 *
 * \brief Исходный файл библиотеки вывода отладочной информации.
 *
 */

#include <time.h>
#include <stdio.h>
#include <string.h>

#define G_LOG_DOMAIN "GPTRACE"

#include "gp-trace.h"

#if defined( G_OS_WIN32 )
#include <windows.h>
#endif

#if defined( G_OS_UNIX )
#include <syslog.h>
#endif


typedef enum {

  GP_TRACE_OUTPUT_NONE = 0,
  GP_TRACE_OUTPUT_DEFAULT = 1 << 0,
  GP_TRACE_OUTPUT_CONSOLE = 1 << 1,
  GP_TRACE_OUTPUT_FILE = 1 << 2,
  GP_TRACE_OUTPUT_SYSLOG = 1 << 3,
  GP_TRACE_OUTPUT_WINDBG = 1 << 4,

} GpTraceOutputType;


typedef enum {

  GP_TRACE_LEVEL_INDEX_ERROR = 0,
  GP_TRACE_LEVEL_INDEX_CRITICAL,
  GP_TRACE_LEVEL_INDEX_WARNING,
  GP_TRACE_LEVEL_INDEX_MESSAGE,
  GP_TRACE_LEVEL_INDEX_INFO,
  GP_TRACE_LEVEL_INDEX_DEBUG,
  GP_TRACE_LEVEL_INDEX_NUM,

} GpTraceLevelLogIndex;


typedef struct {

  GpTraceOutputType output;

  gchar *level_name;

  gchar *output_file_name;
  FILE *output_file_fd;

} GpTraceOutput;


static gboolean gp_trace_configured = FALSE;
static gboolean gp_trace_time = FALSE;
static gboolean gp_trace_terse = FALSE;
static GLogLevelFlags gp_trace_print_to = 0;
static GLogLevelFlags gp_trace_printerr_to = 0;

static gchar *gp_trace_prog_name = NULL;

static GpTraceOutput gp_trace_output_default = {GP_TRACE_OUTPUT_NONE, NULL, NULL };

static GpTraceOutput gp_trace_output[GP_TRACE_LEVEL_INDEX_NUM] =
  {
    {GP_TRACE_OUTPUT_NONE, "error", NULL, NULL },
    {GP_TRACE_OUTPUT_NONE, "critical", NULL, NULL },
    {GP_TRACE_OUTPUT_NONE, "warning", NULL, NULL },
    {GP_TRACE_OUTPUT_NONE, "message", NULL, NULL },
    {GP_TRACE_OUTPUT_NONE, "info", NULL, NULL },
    {GP_TRACE_OUTPUT_NONE, "debug", NULL, NULL },
  };

static GpTraceCallback gp_trace_callback_func = NULL;


// Функция вывода сообщений в консоль в UNIX.
// В Windows кодировка консоли отличается от системной, пока не поддерживаем вывод.
static void gp_trace_output_console (const gchar *date_time, const gchar *source, const gchar *level,
                                     const gchar *message)
{

#if defined( G_OS_UNIX )

  if(gp_trace_terse) printf( "%s\n", message );
  else if(gp_trace_time) printf( "%s %s %s:\t%s\n", date_time, source, level, message );
  else printf( "%s %s: %s\n", source, level, message );

  fflush( stdout );

#endif

}


/*! Функция вывода сообщений в файл.*/
static void gp_trace_output_file (FILE *output_fd, const gchar *date_time, const gchar *source, const gchar *level,
                                  const gchar *message)
{

  if(gp_trace_terse) fprintf( output_fd, "%s\n", message );
  else if(gp_trace_time) fprintf( output_fd, "%s %s %s:\t%s\n", date_time, source, level, message );
  else fprintf( output_fd, "%s %s: %s\n", source, level, message );

  fflush( output_fd );

}


/*! Функция вывода сообщений в систему syslog.*/
static void gp_trace_output_syslog (const gchar *date_time, const gchar *source, const gchar *level,
                                    const gchar *message)
{

#if defined( G_OS_UNIX )

  if(gp_trace_terse) syslog( LOG_INFO, message );
  else syslog( LOG_INFO, "%s %s: %s", source, level, message );

#endif

}


/*! Функция вывода сообщений в систему dbg windows.*/
static void gp_trace_output_windbg (const gchar *date_time, const gchar *source, const gchar *level,
                                    const gchar *message)
{

#if defined( G_OS_WIN32 )

  gchar native_message[2048];
  gchar *locale_message;

  if( gp_trace_terse ) snprintf( native_message, sizeof( native_message ), "%s\n", message );
  else if( gp_trace_time ) snprintf( native_message, sizeof( native_message ), "%s %s %s:\t%s\n", date_time, source, level, message );
  else snprintf( native_message, sizeof( native_message ), "%s %s: %s\n", source, level, message );

  locale_message = g_locale_from_utf8( native_message, -1, NULL, NULL, NULL );
  OutputDebugString( locale_message );
  g_free( locale_message );
  fflush( stdout );

#endif

}


// Основной обработчик вывода сообщений.
static void gp_trace_log (const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer data)
{

  GpTraceLevelLogIndex level_log_index;
  GpTraceOutput *output;

  gchar date_time[128];
  gchar source[256];
  gchar *level;

  if( !gp_trace_configured) return;

  // Определяем тип выводимого сообщения.
  if( log_level & G_LOG_LEVEL_ERROR ) level_log_index = GP_TRACE_LEVEL_INDEX_ERROR;
  else if( log_level & G_LOG_LEVEL_CRITICAL ) level_log_index = GP_TRACE_LEVEL_INDEX_CRITICAL;
  else if( log_level & G_LOG_LEVEL_WARNING ) level_log_index = GP_TRACE_LEVEL_INDEX_WARNING;
  else if( log_level & G_LOG_LEVEL_MESSAGE ) level_log_index = GP_TRACE_LEVEL_INDEX_MESSAGE;
  else if( log_level & G_LOG_LEVEL_INFO ) level_log_index = GP_TRACE_LEVEL_INDEX_INFO;
  else if( log_level & G_LOG_LEVEL_DEBUG ) level_log_index = GP_TRACE_LEVEL_INDEX_DEBUG;
  else return;

  // Определяем параметры вывода сообщения.
  if( gp_trace_output[ level_log_index ].output == GP_TRACE_OUTPUT_DEFAULT)
    output = &gp_trace_output_default;
  else
    output = &gp_trace_output[ level_log_index ];

  level = gp_trace_output[ level_log_index ].level_name;

  // Формируем строку с датой и временем.
  GTimeVal time_tv;
  struct tm *time_tm;
  time_t time_t;
  gint time_len;

  g_get_current_time( &time_tv );
  time_t = time_tv.tv_sec;
  time_tm = localtime( &time_t );

  date_time[0] = '\0';
  time_len = strftime( date_time, sizeof( date_time ), "%Y-%m-%d %H:%M:%S", time_tm );
  snprintf( date_time + time_len, sizeof( date_time ) - time_len, ".%03ld", time_tv.tv_usec / 1000 );

  if( gp_trace_prog_name != NULL && log_domain != NULL )
    snprintf( source, sizeof( source ), "%s.%s", gp_trace_prog_name, log_domain );
  else if( gp_trace_prog_name != NULL )
    snprintf( source, sizeof( source ), "%s", gp_trace_prog_name);
  else if( log_domain != NULL )
    snprintf( source, sizeof( source ), "%s", log_domain );
  else
    snprintf( source, sizeof( source ), "Unknown" );

  // Выводим сообщения.
  if( output->output & GP_TRACE_OUTPUT_CONSOLE)
    gp_trace_output_console (date_time, source, level, message);

  if( output->output & GP_TRACE_OUTPUT_FILE)
    {

    // Если файл еще не открыт, открываем его.
    if( output->output_file_fd == NULL )
      output->output_file_fd = fopen( output->output_file_name, "a" );

    // Если произошла ошибка, отключаем вывод в файл.
    if( output->output_file_fd == NULL )
      {
      g_warning( "can't open log file %s\n", output->output_file_name );
      output->output &= ~GP_TRACE_OUTPUT_FILE;
      }
    else
      gp_trace_output_file (output->output_file_fd, date_time, source, level, message);

    }

  if( output->output & GP_TRACE_OUTPUT_SYSLOG)
    gp_trace_output_syslog (date_time, log_domain, level, message);

  if( output->output & GP_TRACE_OUTPUT_WINDBG)
    gp_trace_output_windbg (date_time, source, level, message);

  if( gp_trace_callback_func != NULL )
    gp_trace_callback_func ( log_level, date_time, source, level, message );

}


static void gp_trace_print (const gchar *message)
{

  guint i;
  gchar **strings = g_strsplit_set( message, "\r\n", -1 );

  for( i = 0; i < g_strv_length( strings ); i++ )
    {
    if( strings[i][0] == 0 ) continue;
    if( gp_trace_print_to & G_LOG_LEVEL_ERROR ) gp_trace_log ("print", G_LOG_LEVEL_ERROR, strings[i], NULL);
    if( gp_trace_print_to & G_LOG_LEVEL_CRITICAL ) gp_trace_log ("print", G_LOG_LEVEL_CRITICAL, strings[i], NULL);
    if( gp_trace_print_to & G_LOG_LEVEL_WARNING ) gp_trace_log ("print", G_LOG_LEVEL_WARNING, strings[i], NULL);
    if( gp_trace_print_to & G_LOG_LEVEL_MESSAGE ) gp_trace_log ("print", G_LOG_LEVEL_MESSAGE, strings[i], NULL);
    if( gp_trace_print_to & G_LOG_LEVEL_INFO ) gp_trace_log ("print", G_LOG_LEVEL_INFO, strings[i], NULL);
    if( gp_trace_print_to & G_LOG_LEVEL_DEBUG ) gp_trace_log ("print", G_LOG_LEVEL_DEBUG, strings[i], NULL);
    }

  g_strfreev( strings );

}


static void gp_trace_printerr (const gchar *message)
{

  guint i;
  gchar **strings = g_strsplit_set( message, "\r\n", -1 );

  for( i = 0; i < g_strv_length( strings ); i++ )
    {
    if( strings[i][0] == 0 ) continue;
    if( gp_trace_printerr_to & G_LOG_LEVEL_ERROR ) gp_trace_log ("printerr", G_LOG_LEVEL_ERROR, strings[i], NULL);
    if( gp_trace_printerr_to & G_LOG_LEVEL_CRITICAL ) gp_trace_log ("printerr", G_LOG_LEVEL_CRITICAL, strings[i], NULL);
    if( gp_trace_printerr_to & G_LOG_LEVEL_WARNING ) gp_trace_log ("printerr", G_LOG_LEVEL_WARNING, strings[i], NULL);
    if( gp_trace_printerr_to & G_LOG_LEVEL_MESSAGE ) gp_trace_log ("printerr", G_LOG_LEVEL_MESSAGE, strings[i], NULL);
    if( gp_trace_printerr_to & G_LOG_LEVEL_INFO ) gp_trace_log ("printerr", G_LOG_LEVEL_INFO, strings[i], NULL);
    if( gp_trace_printerr_to & G_LOG_LEVEL_DEBUG ) gp_trace_log ("printerr", G_LOG_LEVEL_DEBUG, strings[i], NULL);
    }

  g_strfreev( strings );

}


// Функция разбора строки с параметрами вывода сообщений.
static void gp_trace_parse_output (const gchar *output_def, GpTraceOutput *output)
{

  gchar **values;
  guint i;

  if( output_def == NULL ) return;

  values = g_strsplit( output_def, ",", -1 );
  for( i = 0; i < g_strv_length( values ); i++ )
    {

    // Вывод отключен.
    if( g_strcmp0( values[i], "none" ) == 0 ) output->output = GP_TRACE_OUTPUT_NONE;

    // Вывод по умолчанию.
    if( g_strcmp0( values[i], "default" ) == 0 ) output->output = GP_TRACE_OUTPUT_DEFAULT;

    // Вывод в консоль.
    if( g_strcmp0( values[i], "console" ) == 0 ) output->output |= GP_TRACE_OUTPUT_CONSOLE;

    // Вывод в файл.
    if( g_str_has_prefix( values[i], "file=" ) )
      {
      if( output->output_file_name != NULL ) g_free( output->output_file_name );
      if( output->output_file_fd != NULL ) fclose( output->output_file_fd );
      output->output_file_name = g_strdup( values[i] + 5 );
      output->output |= GP_TRACE_OUTPUT_FILE;
      output->output_file_fd = NULL;
      }

    // Вывод в syslog.
    if( g_strcmp0( values[i], "syslog" ) == 0 ) output->output |= GP_TRACE_OUTPUT_SYSLOG;

    // Вывод через OutputDebugString в Windows.
    if( g_strcmp0( values[i], "windbg" ) == 0 ) output->output |= GP_TRACE_OUTPUT_WINDBG;

    }
  g_strfreev( values );

}


// Функция инициализации библиотеки.
void gp_trace_init (int *argc, char ***argv)
{

  gchar *trace_levels = g_strdup( "error,critical,warning,message" );
  gchar *trace_default = NULL;
  gchar *trace_error = NULL;
  gchar *trace_critical = NULL;
  gchar *trace_warning = NULL;
  gchar *trace_message = NULL;
  gchar *trace_info = NULL;
  gchar *trace_debug = NULL;
  gchar *trace_print_to = NULL;
  gchar *trace_printerr_to = NULL;

  const gchar *env_value;
  gchar **values;
  guint i, j, k;

  GError          *error = NULL;
  GOptionContext  *context;
  GOptionEntry     entries[] =
  {
    { "trace-levels", 0, 0, G_OPTION_ARG_STRING, &trace_levels, NULL, NULL },
    { "trace-default", 0, 0, G_OPTION_ARG_STRING, &trace_default, NULL, NULL },
    { "trace-error", 0, 0, G_OPTION_ARG_STRING, &trace_error, NULL, NULL },
    { "trace-critcal", 0, 0, G_OPTION_ARG_STRING, &trace_critical, NULL, NULL },
    { "trace-warning", 0, 0, G_OPTION_ARG_STRING, &trace_warning, NULL, NULL },
    { "trace-message", 0, 0, G_OPTION_ARG_STRING, &trace_message, NULL, NULL },
    { "trace-info", 0, 0, G_OPTION_ARG_STRING, &trace_info, NULL, NULL },
    { "trace-debug", 0, 0, G_OPTION_ARG_STRING, &trace_debug, NULL, NULL },
    { "trace-print-to", 0, 0, G_OPTION_ARG_STRING, &trace_print_to, NULL, NULL },
    { "trace-printerr-to", 0, 0, G_OPTION_ARG_STRING, &trace_printerr_to, NULL, NULL },
    { "trace-time", 0, 0, G_OPTION_ARG_NONE, &gp_trace_time, NULL, NULL },
    { "trace-terse", 0, 0, G_OPTION_ARG_NONE, &gp_trace_terse, NULL, NULL },
    { NULL }
  };

  if(gp_trace_configured) return;

  if( argv )
    gp_trace_prog_name = g_path_get_basename( *argv[0] );
  else
    gp_trace_prog_name = g_strdup( "Unknown" );

  #if defined( G_OS_UNIX )
  trace_default = g_strdup( "console" );
  #endif

  #if defined( G_OS_WIN32 )
  trace_default = g_strdup( "windbg" );
  #endif

  // Параметры заданые через переменные окружения.
  env_value = g_getenv( "G_TRACE_LEVELS" );
  if( env_value != NULL ) { g_free( trace_levels ); trace_levels = g_strdup( env_value ); }

  env_value = g_getenv( "G_TRACE_DEFAULT" );
  if( env_value != NULL ) { g_free( trace_default ); trace_default = g_strdup( env_value ); }

  trace_error = g_strdup( g_getenv( "G_TRACE_ERROR" ) );
  trace_critical = g_strdup( g_getenv( "G_TRACE_CRITICAL" ) );
  trace_warning = g_strdup( g_getenv( "G_TRACE_WARNING" ) );
  trace_message = g_strdup( g_getenv( "G_TRACE_MESSAGE" ) );
  trace_info = g_strdup( g_getenv( "G_TRACE_INFO" ) );
  trace_debug = g_strdup( g_getenv( "G_TRACE_DEBUG" ) );

  trace_print_to = g_strdup( g_getenv( "G_TRACE_PRINT_TO" ) );
  trace_printerr_to = g_strdup( g_getenv( "G_TRACE_PRINTERR_TO" ) );

  // Разбираем аргументы командной строки.
  context = g_option_context_new( "" );
  g_option_context_set_help_enabled( context, FALSE );
  g_option_context_add_main_entries( context, entries, NULL );
  g_option_context_set_ignore_unknown_options( context, TRUE );
  g_option_context_parse( context, argc, argv, &error );
  g_option_context_free( context );

  // Вывод по умолчанию.
  gp_trace_parse_output (trace_default, &gp_trace_output_default);

  // Какие уровни используют вывод по умолчанию.
  values = g_strsplit( trace_levels, ",", -1 );
  for( i = 0; i < g_strv_length( values ); i++ )
    {

    if( strlen( values[i] ) < 3 ) continue;

    // Все уровни используют вывод по умолчанию.
    if( g_strcmp0( values[i], "all" ) == 0 )
      for( j = 0; j < GP_TRACE_LEVEL_INDEX_NUM; j++ )
        gp_trace_output[j].output = GP_TRACE_OUTPUT_DEFAULT;

    // Вывод на всех уровнях отключен.
    if( g_strcmp0( values[i], "none" ) == 0 )
      for( j = 0; j < GP_TRACE_LEVEL_INDEX_NUM; j++ )
        gp_trace_output[j].output = GP_TRACE_OUTPUT_NONE;

    // Проверяем каждый уровень.
    for( j = 0; j < GP_TRACE_LEVEL_INDEX_NUM; j++ )
      {

      // Отключили вывод на данном уровне.
      if( values[i][0] == '-' && g_str_has_prefix( values[i] + 1, gp_trace_output[j].level_name ) )
        gp_trace_output[j].output = GP_TRACE_OUTPUT_NONE;

      if( g_str_has_prefix( values[i], gp_trace_output[j].level_name ) )
        {

        // Включен вывод на данном уровне.
        if( strlen( values[i] ) == strlen( gp_trace_output[j].level_name ) )
          gp_trace_output[j].output = GP_TRACE_OUTPUT_DEFAULT;

        // Включен вывод до этого уровня и ниже.
        if( values[i][ strlen( gp_trace_output[j].level_name ) ] == '-' )
          for( k = j; k < GP_TRACE_LEVEL_INDEX_NUM; k++ )
            gp_trace_output[k].output = GP_TRACE_OUTPUT_DEFAULT;

        // Включен вывод начиная с этого уровня и выше.
        if( values[i][ strlen( gp_trace_output[j].level_name ) ] == '+' )
          for( k = 0; k <=j ; k++ )
            gp_trace_output[k].output = GP_TRACE_OUTPUT_DEFAULT;

        }

      }

    }
  g_strfreev( values );

  // Перенаправление вывода g_print.
  if( trace_print_to != NULL )
    {
    values = g_strsplit( trace_print_to, ",", -1 );
    for( i = 0; i < g_strv_length( values ); i++ )
      {

      if( g_strcmp0( values[i], "error" ) == 0 ) gp_trace_print_to |= G_LOG_LEVEL_ERROR;
      if( g_strcmp0( values[i], "critical" ) == 0 ) gp_trace_print_to |= G_LOG_LEVEL_CRITICAL;
      if( g_strcmp0( values[i], "warning" ) == 0 ) gp_trace_print_to |= G_LOG_LEVEL_WARNING;
      if( g_strcmp0( values[i], "message" ) == 0 ) gp_trace_print_to |= G_LOG_LEVEL_MESSAGE;
      if( g_strcmp0( values[i], "info" ) == 0 ) gp_trace_print_to |= G_LOG_LEVEL_INFO;
      if( g_strcmp0( values[i], "debug" ) == 0 ) gp_trace_print_to |= G_LOG_LEVEL_DEBUG;

      }
    g_strfreev( values );
    }

  // Перенаправление вывода g_printerr.
  if( trace_printerr_to != NULL )
    {
    values = g_strsplit( trace_printerr_to, ",", -1 );
    for( i = 0; i < g_strv_length( values ); i++ )
      {

      if( g_strcmp0( values[i], "error" ) == 0 ) gp_trace_printerr_to |= G_LOG_LEVEL_ERROR;
      if( g_strcmp0( values[i], "critical" ) == 0 ) gp_trace_printerr_to |= G_LOG_LEVEL_CRITICAL;
      if( g_strcmp0( values[i], "warning" ) == 0 ) gp_trace_printerr_to |= G_LOG_LEVEL_WARNING;
      if( g_strcmp0( values[i], "message" ) == 0 ) gp_trace_printerr_to |= G_LOG_LEVEL_MESSAGE;
      if( g_strcmp0( values[i], "info" ) == 0 ) gp_trace_printerr_to |= G_LOG_LEVEL_INFO;
      if( g_strcmp0( values[i], "debug" ) == 0 ) gp_trace_printerr_to |= G_LOG_LEVEL_DEBUG;

      }
    g_strfreev( values );
    }

  // Вывод для каждого из уровней.
  gp_trace_parse_output (trace_error, &gp_trace_output[GP_TRACE_LEVEL_INDEX_ERROR]);
  gp_trace_parse_output (trace_critical, &gp_trace_output[GP_TRACE_LEVEL_INDEX_CRITICAL]);
  gp_trace_parse_output (trace_warning, &gp_trace_output[GP_TRACE_LEVEL_INDEX_WARNING]);
  gp_trace_parse_output (trace_message, &gp_trace_output[GP_TRACE_LEVEL_INDEX_MESSAGE]);
  gp_trace_parse_output (trace_info, &gp_trace_output[GP_TRACE_LEVEL_INDEX_INFO]);
  gp_trace_parse_output (trace_debug, &gp_trace_output[GP_TRACE_LEVEL_INDEX_DEBUG]);

  g_log_set_default_handler (gp_trace_log, NULL);
  if(gp_trace_print_to) g_set_print_handler (gp_trace_print);
  if(gp_trace_printerr_to) g_set_printerr_handler (gp_trace_printerr);

  gp_trace_configured = TRUE;

  g_free( trace_levels );
  g_free( trace_default );
  g_free( trace_error );
  g_free( trace_critical );
  g_free( trace_warning );
  g_free( trace_message );
  g_free( trace_info );
  g_free( trace_debug );

}


void gp_trace_add_output (GLogLevelFlags log_level, const gchar *output_def)
{

  if( log_level == 0 ) gp_trace_parse_output (output_def, &gp_trace_output_default);
  if( log_level & G_LOG_LEVEL_ERROR ) gp_trace_parse_output (output_def, &gp_trace_output[GP_TRACE_LEVEL_INDEX_ERROR]);
  if( log_level & G_LOG_LEVEL_CRITICAL )
    gp_trace_parse_output (output_def, &gp_trace_output[GP_TRACE_LEVEL_INDEX_CRITICAL]);
  if( log_level & G_LOG_LEVEL_WARNING )
    gp_trace_parse_output (output_def, &gp_trace_output[GP_TRACE_LEVEL_INDEX_WARNING]);
  if( log_level & G_LOG_LEVEL_MESSAGE )
    gp_trace_parse_output (output_def, &gp_trace_output[GP_TRACE_LEVEL_INDEX_MESSAGE]);
  if( log_level & G_LOG_LEVEL_INFO ) gp_trace_parse_output (output_def, &gp_trace_output[GP_TRACE_LEVEL_INDEX_INFO]);
  if( log_level & G_LOG_LEVEL_DEBUG ) gp_trace_parse_output (output_def, &gp_trace_output[GP_TRACE_LEVEL_INDEX_DEBUG]);

}


void gp_trace_set_callback (GpTraceCallback callback)
{

  gp_trace_callback_func = callback;

}
