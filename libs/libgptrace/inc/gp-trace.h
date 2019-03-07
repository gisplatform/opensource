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
 * \file gtrace.h
 *
 * \author Andrei Fadeev, Petr Zankov
 * \date 22.07.2009
 *
 * \brief Заголовочный файл библиотеки вывода отладочной информации.
 *
 * \mainpage GTRACE - Библиотека вывода отладочной информации
 *
 * Библиотека gtrace предназначена для вывода отладочной информации в различные
 * источники, такие как:
 *
 * - console - текстовая консоль (только для Unix);
 * - syslog - система syslog (только для Unix);
 * - windbg - система windbg (только для Windows);
 * - file - файл (имя файла указывается после символа '=', например file=output.log);
 * - callback - функция пользователя.
 *
 * Сообщения создаются стандартными функциями вывода диагностических сообщений библиотеки GLIB:
 * g_log, g_error, g_critical, g_warning, g_message и g_debug. Также можно перенаправить вывод
 * функций g_print и g_printerr представив их как вывод отладочных сообщений с определённым уровнем.
 *
 * Параметры вывода задаются через переменные окружения, параметры командной строки или функцией #g_trace_add_output.
 *
 * В командной строке можно указать следующие параметры изменяющие поведение библиотеки:
 *
 * -  --trace-levels - строка с разделёнными запятой названиями уровней сообщений использующих вывод по умолчанию;
 * -  --trace-default - строка с параметрами вывода сообщений по умолчанию;
 * -  --trace-error - строка с параметрами вывода сообщений для уровня error;
 * -  --trace-critical - строка с параметрами вывода сообщений для уровня critical;
 * -  --trace-warning - строка с параметрами вывода сообщений для уровня warning;
 * -  --trace-message - строка с параметрами вывода сообщений для уровня message;
 * -  --trace-info - строка с параметрами вывода сообщений для уровня info;
 * -  --trace-debug - строка с параметрами вывода сообщений для уровня debug;
 * -  --trace-print-to - если с разделёнными запятой названиями уровней сообщений в которые перенаправляется вывод g_print;
 * -  --trace-printerr-to - если с разделёнными запятой названиями уровней сообщений в которые перенаправляется вывод g_printerr;
 * -  --trace-time - отобразить дату и время печати сообщения;
 * -  --trace-terse - не показывать уровень сообщения.
 *
 * Строка с параметрами содержит разделённые запятой названия систем вывода сообщений с возможными
 * параметрами. Например "console,file=filename".
 *
 * Переменные окружения имеют действие аналогичное параметрам командной строки. Параметры командной
 * строки имеют больший приоритет перед переменными окружения. Могут быть заданы следующие переменные окружения:
 *
 * - G_TRACE_LEVELS - значение аналогично параметру --trace-levels;
 * - G_TRACE_DEFAULT - значение аналогично параметру --trace-default;
 * - G_TRACE_ERROR - значение аналогично параметру --trace-error;
 * - G_TRACE_CRITICAL - значение аналогично параметру --trace-critical;
 * - G_TRACE_WARNING - значение аналогично параметру --trace-warning;
 * - G_TRACE_MESSAGE - значение аналогично параметру --trace-message;
 * - G_TRACE_INFO - значение аналогично параметру --trace-info;
 * - G_TRACE_DEBUG - значение аналогично параметру --trace-debug;
 * - G_TRACE_PRINT_TO - значение аналогично параметру --trace-print-to;
 * - G_TRACE_PRINTERR_TO - значение аналогично параметру --trace-printerr-to.
 *
 * По умолчанию для Linux устанавливается вывод через консоль, а для Windows через windbg.
 *
 * По умолчанию выводятся только сообщения с уровнем error, critical, warning и message.
 *
 * Основные функции библиотеки:
 *
 * - #g_trace_init - инициализация библиотеки;
 * - #g_trace_add_output - установка параметров вывода сообщений;
 *
 */

#ifndef _gtrace_h
#define _gtrace_h

#include <glib.h>


G_BEGIN_DECLS


/**
 * GpTraceCallback:
 * @log_level: Тип сообщения.
 * @date_time: Строка даты и времени.
 * @source: Строка источника сообщения.
 * @level: Тип сообщения в виде строки.
 * @message: Сообщение.
 *
 * Тип callback-функции вызываемой при обработке сообщений.
*/
typedef void (*GpTraceCallback)( GLogLevelFlags log_level, const gchar *date_time, const gchar *source, const gchar *level, const gchar *message );


/**
 * gp_trace_init:
 * @argc: (inout) (allow-none): Указатель на число аргументов командной строки.
 * @argv: (inout) (array length=argc) (allow-none) (transfer none): Указатель на массив аргументов командной строки.
 *
 * Инициализация библиотеки.
 *
 * Инициализация должна выполняться в самом начале программы,
 * до первого вызова функций вывода сообщений.
*/
void gp_trace_init (int *argc, char ***argv);


/**
 * gp_trace_add_output:
 * @log_level: Флаги уровней вывода для которых устанавливаются параметры, 0 - для параметров по умолчанию.
 * @output_def: Строка определения параметров вывода.
 *
 * Установка параметров вывода сообщений.
 *
 * Параметры вывода сообщений добавляются к текущим параметрам.
 * Для того чтобы полностью переопределить параметры вывода в строке определения
 * первым нужно указать параметр - 'none'. Например - 'none,console'.
*/
void gp_trace_add_output (GLogLevelFlags log_level, const gchar *output_def);


/**
 * gp_trace_set_callback:
 * @callback: (scope async): Пользовательская функция вывода сообщений.
 *
 * Установка пользовательской функции вывода сообщений.
 *
 * Если пользовательская функция вывода сообщений определена, она вызывается всегда.
*/
void gp_trace_set_callback (GpTraceCallback callback);


G_END_DECLS

#endif // _gtrace_h
