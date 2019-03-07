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
 * \file gp-rpc-auth-cram-server.h
 *
 * \author Andrei Fadeev
 * \date 24.02.2014
 * \brief Заголовочный файл класса взаимной аутентификации клиентов по механизму запрос-ответ.
 *
 * \defgroup GpRpcAuthCramServer GpRpcAuthCramServer - Сервер аутентификации по механизму запрос-ответ.
 *
 * Описание системы аутентификации приведено в разделе - \link GpRpcAuthCram \endlink.
 *
 * Для создания сервера CRAM аутентификации используется функция #gp_rpc_auth_cram_server_new.
 *
 * Добавление ключа осуществляется функцией #gp_rpc_auth_cram_server_add_key. Возможно зарегистрировать
 * несколько ключей для клиентов.
 *
 * Сервер CRAM аутентификации может использоваться в нескольких серверах GPRPC одновременно.
 *
 * Удаление сервера производится функцией g_object_unref.
 *
*/

#ifndef _gp_rpc_auth_cram_server_h
#define _gp_rpc_auth_cram_server_h

#include <glib-object.h>

#include <gp-rpc-auth.h>

G_BEGIN_DECLS


#define GP_RPC_AUTH_CRAM_MAX_SESSIONS        1024     // Максимально возможное число сессий.
#define GP_RPC_AUTH_CRAM_TIMEOUT             3600     // Таймаут отключения клиента, с.
#define GP_RPC_AUTH_CRAM_AUTH_TIMEOUT           5     // Таймаут отключения клиента при первичной аутентификации, с.


// Описание класса.
#define G_TYPE_RPC_AUTH_CRAM_SERVER                 gp_rpc_auth_cram_server_get_type()
#define GP_RPC_AUTH_CRAM_SERVER( obj )                ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), G_TYPE_RPC_AUTH_CRAM_SERVER, GpRpcAuthCramServer ) )
#define GP_RPC_AUTH_CRAM_SERVER_CLASS( vtable )       ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), G_TYPE_RPC_AUTH_CRAM_SERVER, GpRpcAuthCramClassServer ) )
#define GP_RPC_AUTH_CRAM_SERVER_GET_CLASS( obj )      ( G_TYPE_INSTANCE_GET_INTERFACE (( obj ), G_TYPE_RPC_AUTH_CRAM_SERVER, GpRpcAuthCramClassServer ) )

GType gp_rpc_auth_cram_server_get_type (void);


typedef GObject GpRpcAuthCramServer;
typedef GObjectClass GpRpcAuthCramServerClass;


/*! Создание сервера CRAM аутентификации.
 *
 * \param hash_type тип хеш функции;
 * \param chalenge_size для строк запроса;
 * \param max_sessions максимально возможное число одновременно подключенных клиентов;
 * \param auth_timeout время в течение которого клиент должен завершить первичную аутентификацию;
 * \param timeout время через которое клиент будет отключен при не активности.
 *
 * \return Указатель на интерфейс \link GpRpcAuth \endlink или NULL в случае ошибки.
 *
*/
GpRpcAuthCramServer *gp_rpc_auth_cram_server_new (GChecksumType hash_type, guint32 chalenge_size, guint32 max_sessions,
                                                  guint32 auth_timeout, guint32 timeout);


/*! Добавление ключа доступа.
 *
 * Данные о пользователе будут возвращаться функцией #gp_rpc_auth_get_user_data и передаваться в #GpRpcServerAclCallback.
 *
 * \param gp_rpc_auth_cram указатель на объект GpRpcAuthCramServer;
 * \param key64ключ доступа в виде строки base64;
 * \param data данные о пользователе.
 *
 * \return TRUE в случае успеха, FALSE в случае ошибки.
 *
*/
gboolean gp_rpc_auth_cram_server_add_key (GpRpcAuthCramServer *gp_rpc_auth_cram, const gchar *key64, gpointer data);


G_END_DECLS

#endif // _gp_rpc_auth_cram_server_h
