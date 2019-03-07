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
 * \file gp-rpc-auth-cram.h
 *
 * \author Andrei Fadeev
 * \date 20.02.2014
 * \brief Заголовочный файл класса взаимной аутентификации на сервере по механизму запрос-ответ.
 *
 * \defgroup GpRpcAuthCram GpRpcAuthCram - Клиент аутентификации по механизму запрос-ответ.
 *
 * Данная система позволяет проверить, что клиент и сервер имеют одинаковый ключ. Ключ представляет
 * собой случайный набор данных длиной от 8 до 1024 байт. Данной проверке подвергается каждый GPRPC
 * пакет от клиента и сервера. В начале своей работы клиент запрашивает у сервера строку доступа -
 * server_chalenge. Затем производится генерация строки доступа для сервера - client_chalenge.
 * Клиент вычисляет хеш-код аутентификации сообщений - HMAC для следующих данных:
 *
 * - ключ доступа;
 * - пользовательские данные в запросе;
 * - строка доступа client_chalenge;
 * - строка доступа server_chalenge.
 *
 * Вычисленный хеш-код - client_response отправляется серверу вместе со строкой доступа client_chalenge.
 *
 * Имея строку запроса client_chalenge сервер в свою очередь вычисляет хеш-код и сравнивает его
 * с ответом клиента client_response. В случае совпадения считается, что клиент правильно знает ключ
 * и подключение разрешается.
 *
 * При отправке ответа сервер вычисляет хеш-код для следующих данных:
 *
 * - ключ доступа;
 * - пользовательские данные в запросе;
 * - строка доступа server_chalenge;
 * - строка доступа client_chalenge.
 *
 * Вычисленный хеш-код - server_response сервер отправляет клиенту вместе с НОВОЙ строкой запроса
 * server_chalenge. Предыдущая строка запроса сохраняется на случай потери пакета с ответом сервера
 * и повторным запросом клиента с тем же идентификатором пакета данных \link GpRpcHeader \endlink.
 *
 * Клиент проверяет ответ сервера, запоминает новую строку запроса server_chalenge и генерирует
 * НОВУЮ строку запроса client_chalenge.
 *
 * Таким образом проверяется, что и клиент и сервер владеют одним и тем же ключом доступа.
 *
 * Для создания клиента CRAM аутентификации используется функция #gp_rpc_auth_cram_new.
 *
 * Удаление клиента производится функцией g_object_unref.
 *
*/

#ifndef _gp_rpc_auth_cram_h
#define _gp_rpc_auth_cram_h

#include <glib-object.h>

#include <gp-rpc-auth.h>

G_BEGIN_DECLS


// Описание класса.
#define G_TYPE_RPC_AUTH_CRAM                 gp_rpc_auth_cram_get_type()
#define GP_RPC_AUTH_CRAM( obj )                ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), G_TYPE_RPC_AUTH_CRAM, GpRpcAuthCram ) )
#define GP_RPC_AUTH_CRAM_CLASS( vtable )       ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), G_TYPE_RPC_AUTH_CRAM, GpRpcAuthCramClass ) )
#define GP_RPC_AUTH_CRAM_GET_CLASS( obj )      ( G_TYPE_INSTANCE_GET_INTERFACE (( obj ), G_TYPE_RPC_AUTH_CRAM, GpRpcAuthCramClass ) )

GType gp_rpc_auth_cram_get_type (void);


typedef GObject GpRpcAuthCram;
typedef GObjectClass GpRpcAuthCramClass;


/*! Создание клиента CRAM аутентификации.
 *
 * \param key64 ключ доступа в виде строки base64.
 *
 * \return Указатель на интерфейс \link GpRpcAuth \endlink или NULL в случае ошибки.
 *
*/
GpRpcAuthCram *gp_rpc_auth_cram_new (const gchar *key64);


G_END_DECLS

#endif // _gp_rpc_auth_cram_h
