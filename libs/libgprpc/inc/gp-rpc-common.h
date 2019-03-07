/*
 * GPRPC - rpc (remote procedure call) library, this library is part of GRTL.
 *
 * Copyright 2014 Andrei Fadeev
 *
 * This file is part of GRTL.
 *
 * GRTL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GRTL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GRTL. If not, see <http://www.gnu.org/licenses/>.
 *
*/

/*!
 * \file gp-rpc-common.h
 *
 * \author Andrei Fadeev
 * \date 12.02.2014
 * \brief Заголовочный файл общих функций GPRPC.
 *
 */

#ifndef _gp_rpc_common_h
#define _gp_rpc_common_h

#include <glib.h>

#include <gp-rpc-auth.h>
#include <gp-rpc-data.h>
#include <gp-rpc-manager.h>


/*! \brief Значения по умолчанию. */
#define GP_RPC_DEFAULT_MAX_SESSIONS    1024  /*!< Максимальное число одновременных сессий. */
#define GP_RPC_DEFAULT_CLIENT_TIMEOUT   600  /*!< Время не активности до автоматического отключение клиента. */
#define GP_RPC_DEFAULT_THREADS_NUM        1  /*!< Число рабочих потоков. */
#define GP_RPC_DEFAULT_EXEC_TIMEOUT     5.0  /*!< Таймаут выполнения запроса в секундах. */
#define GP_RPC_DEFAULT_DATA_TIMEOUT    10.0  /*!< Таймаут чтения запроса в секундах. */
#define GP_RPC_DEFAULT_DATA_SIZE      65000  /*!< Максимальный размер данных в запросе - ответе. */

#define GP_RPC_DEFAULT_RESTART            5  /*!< Число попыток выполнения запроса (для механизма UDP). */

#define GP_RPC_HEADER_SIZE       sizeof( GpRpcHeader )
#define GP_RPC_DEFAULT_BUFFER_SIZE ( GP_RPC_DEFAULT_DATA_SIZE + GP_RPC_HEADER_SIZE )


/*! \brief Все поля RPC заголовка представлены в сетевом (big endian) порядке следования байт. */
#define GP_RPC_MAGIC             0x47525043  /*!< Идентификатор RPC пакета - строка 'GPRPC'. */
#define GP_RPC_VERSION           0x00030000  /*!< Версия протокола GPRPC - старшие 16 бит - MAJOR, младшие 16 бит - MINOR. */


/*! \brief Пользовательские идентификаторы. */
#define GP_RPC_PARAM_USER        0x20000000  /*!< Идентификатор начала пользовательских параметров. */
#define GP_RPC_PROC_USER         0x20000000  /*!< Идентификатор начала пользовательских функций. */


/*! \brief Системные идентификаторы функций. */
#define GP_RPC_PROC_GET_CAP      0x00010000  /*!< Идентификатор функции запроса параметров RPC сервера. */
#define GP_RPC_PROC_AUTHENTICATE 0x00020000  /*!< Идентификатор функции аутентификации. */
#define GP_RPC_PROC_LOGOUT       0x00030000  /*!< Идентификатор функции отключения. */


/*! \brief Системные идентификаторы параметров. */
#define GP_RPC_PARAM_NEED_AUTH   0x00010000  /*!< Идентификатор необходимости аутентификации - guint32. */
#define GP_RPC_PARAM_AUTH_TYPE   0x00020000  /*!< Идентификатор типа аутентификации - guint32. */
#define GP_RPC_PARAM_STATUS      0x00030000  /*!< Идентификатор статуса - guint32. */
#define GP_RPC_PARAM_PROC        0x00040000  /*!< Идентификатор вызываемой функции - guint32. */
#define GP_RPC_PARAM_OBJ         0x00050000  /*!< Идентификатор вызываемого объекта - guint32. */


/*! \brief Статус выполнения. */
#define GP_RPC_STATUS_FAIL               0x00010000  /*!< Ошибка. */
#define GP_RPC_STATUS_OK                 0x00020000  /*!< Выполнено. */
#define GP_RPC_STATUS_VERSION_MISMATCH   0x00030000  /*!< Ошибка версии клиента/сервера. */
#define GP_RPC_STATUS_AUTH_NOT_SUPPORTED 0x00040000  /*!< Тип аутентификации не поддерживается. */
#define GP_RPC_STATUS_NOT_AUTHENTICATED  0x00050000  /*!< Требуется аутентификация. */
#define GP_RPC_STATUS_ACCESS_DENIED      0x00060000  /*!< Доступ запрещен. */
#define GP_RPC_STATUS_NO_PROC            0x00070000  /*!< Отсутствует вызываемая функция. */
#define GP_RPC_STATUS_NO_OBJ             0x00080000  /*!< Отсутствует вызываемый объект. */


typedef enum {
  GP_RPC_EXEC_OK, GP_RPC_EXEC_FAIL, GP_RPC_EXEC_CLOSE
} GpRpcExecStatus;


/*! \brief Заголовок GPRPC пакета. */
typedef struct GpRpcHeader {

  guint32      magic;                      /*!< Идентификатор RPC пакета. */
  guint32      version;                    /*!< Версия протокола RPC. */
  guint32      session;                    /*!< Идентификатор сессии. */
  guint32      sequence;                   /*!< Идентификатор пакета с данными. */
  guint32      size;                       /*!< Общий размер передаваемых данных. */
  guint32      client_id;                  /*!< Идентификатор клиента. */

} GpRpcHeader;


gchar   *gp_rpc_get_transport_type (const gchar *uri);
gchar   *gp_rpc_get_address (const gchar *uri);
guint16  gp_rpc_get_port (const gchar *uri);

/**
* gp_rpc_server_user_exec:
* @gp_rpc_acl: (skip): Функция, вызываемая при проверке прав доступа и возможности исполнения запроса клиента.
*/
GpRpcExecStatus gp_rpc_server_user_exec(GpRpcData *gp_rpc_data,
  GpRpcAuth *gp_rpc_auth, GpRpcServerAclCallback gp_rpc_acl, GpRpcManager *gp_rpc_manager);

gboolean gp_rpc_client_check_header (GpRpcData *gp_rpc_data, guint32 sequence, GError **error);

/**
* GpRpcError:
* @GP_RPC_ERROR_FAILED: Обобщенный тип исключений.
* @GP_RPC_ERROR_BAD_URI: Неправильный URI.
* @GP_RPC_ERROR_TIMEOUT: Исключение, вызванное истечением времени ожидания.
* @GP_RPC_ERROR_NOT_FOUND: Не найден запрашиваемый объект или метод.
* @GP_RPC_ERROR_AUTH_FAILED: Ошибка аутентификации.
*
* Виды исключений GpRpc.
*/
typedef enum
{
  GP_RPC_ERROR_FAILED,
  GP_RPC_ERROR_BAD_URI,
  GP_RPC_ERROR_TIMEOUT,
  GP_RPC_ERROR_NOT_FOUND,
  GP_RPC_ERROR_AUTH_FAILED,
}
GpRpcError;

/**
* GP_RPC_ERROR:
*
* Макрос для получения #GError quark для ошибок библиотеки libgprpc.
*/
#define GP_RPC_ERROR (gp_rpc_error_quark())
GQuark gp_rpc_error_quark(void) G_GNUC_CONST;


#endif // _gp_rpc_common_h
