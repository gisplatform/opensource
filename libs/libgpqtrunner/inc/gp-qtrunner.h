#ifndef __GP_QT_RUNNER_H__
#define __GP_QT_RUNNER_H__

#include <glib.h>

G_BEGIN_DECLS

/**
 * gp_qt_runner_run:
 * @argc: (inout) (allow-none): указатель на число аргументов командной строки
 * @argv: (inout) (array length=argc) (allow-none): указатель на массив аргументов командной строки
 *
 * Запускает Qt Main Loop.
 *
 * Функция читает и может модифицировать переменные окружения, т.ч. функция не thread-safe.
 */
void gp_qt_runner_run(int *argc, char ***argv);

/**
 * gp_qt_runner_stop:
 *
 * Останавливает Qt Main Loop.
 */
void gp_qt_runner_stop();

G_END_DECLS

#endif /* __GP_QT_RUNNER_H__ */




