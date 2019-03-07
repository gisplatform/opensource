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

/*
 * \file gp-rpc-server.h
 *
 * \author Andrei Fadeev
 * \date 12.02.2014
 * \brief Заголовочный файл серверного интерфейса удалённых вызовов процедур.
 *
 * \defgroup GpRpcServer GpRpcServer - Сервер библиотеки удалённых вызовов процедур.
 *
 * Библиотека реализует механизм передачи rpc запросов от клиента к серверу через
 * определённый канал связи, такой как TCP, UDP или разделяемую область памяти.
 * Библиотека реализована с использованием механизма gobject и представляет собой
 * интерфейс к различным реализациям GPRPC.
 *
 * Сервер GPRPC создается функцией #gp_rpc_server_create, которая в случае успеха
 * возвращает указатель на новый объект. Этот объект может использоваться для
 * управления работой сервера - получения локального адреса и завершения работы.
 *
 * В процессе своего взаимодействия клиент и сервер могут использовать различные механизмы
 * аутентификации. Для этого предназначен интерфейс \link GpRpcAuth \endlink. В случае
 * необходимости можно создать объект с интерфейсом \link GpRpcAuth \endlink и передать его
 * GPRPC серверу для использования.
 *
 * Помимо аутентификации возможно ограничить клиента в вызове функций или использовании
 * данных при помощи механизма контроля доступа, для этого необходимо при создании сервера
 * указать callback функцию - #GpRpcServerAclCallback. Эта функция будет вызываться перед
 * каждым исполнением запроса клиента и если результатом ее работы будет значение FALSE
 * выполнение запоса клиента будет отменено.
 *
 * Для сервера создается отдельный поток (или потоки) в которых будут выполняться
 * все запросы от клиентов. Для работы сервера необходим объект \link GpRpcManager \endlink
 * содержащий указатели на callback функции  - #GpRpcServerCallback и данные которые
 * могут вызывать клиенты.
 *
 * Сервер GPRPC можно создать используя стандартный механизм g_object_new.
 * Доступны следующие типы GPRPC клиентов:
 *
 * - G_TYPE_URPC_SERVER - обмен через udp/ip;
 * - G_TYPE_TRPC_SERVER - обмен через tcp/ip;
 * - G_TYPE_SRPC_SERVER - обмен через разделяемую область памяти.
 *
 * В этом случае возможна передача следующих параметров конструктору:
 *
 * - uri - строка с адресом сервера;
 * - threads_num - число потоков исполнения (для TCP ограничивает максимальное число подключенных клиентов);
 * - data_size - размер буфера приемо-передачи в байтах (для механизмов TCP и SHM);
 * - data_timeout - максимальное время передачи данных в секундах (для механизма TCP);
 * - auth - объект для аутентификации типа \link GpRpcAuth \endlink;
 * - acl - функция проверки прав доступа #GpRpcServerAclCallback;
 * - manager - объект callback функций и данных \link GpRpcManager \endlink;
 *
 * Параметры uri и manager являются обязательными. Для остальных параметров значения по умолчанию следующие:
 *
 * - threads_num = GP_RPC_DEFAULT_THREADS_NUM = 1;
 * - data_size = GP_RPC_DEFAULT_DATA_SIZE = 65000;
 * - data_timeout = GP_RPC_DEFAULT_DATA_TIMEOUT = 10.0;
 * - auth = NULL - не используется;
 * - acl = NULL - не используется.
 *
 * Удаление сервера производится функцией g_object_unref.
 *
*/

#ifndef _gp_rpc_server_h
#define _gp_rpc_server_h

#include <glib-object.h>

#include <gp-rpc-common.h>
#include <gp-rpc-auth.h>
#include <gp-rpc-manager.h>

G_BEGIN_DECLS


#define G_TYPE_RPC_SERVER                  gp_rpc_server_get_type()
#define GP_RPC_SERVER( obj )                 ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), G_TYPE_RPC_SERVER, GpRpcServer ) )
#define GP_RPC_SERVER_CLASS( vtable )        ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), G_TYPE_RPC_SERVER, GpRpcServerInterface ) )
#define GP_RPC_SERVER_GET_CLASS( inst )      ( G_TYPE_INSTANCE_GET_INTERFACE (( inst ), G_TYPE_RPC_SERVER, GpRpcServerInterface ) )

GType gp_rpc_server_get_type (void);


typedef struct GpRpcServer GpRpcServer;

typedef struct GpRpcServerInterface {

  GTypeInterface        parent;

  gchar* (*get_self_uri)( GpRpcServer *gp_rpc_server);

} GpRpcServerInterface;


/**
 * gp_rpc_server_get_self_uri:
 * @gp_rpc_server: указатель на интерфейс #GpRpcServer.
 *
 * Получение локального адреса сервера.
 *
 * Возвращает строку содержащую локальный адрес сервера к которому он подключен.
 * Адрес возвращается в виде как он был задан в параметре uri.
 * Пользователь должен освободить полученную строку вызовом g_free.
 *
 * Returns: (transfer full) (allow-none): Строка с адресом, иначе NULL.
 *
*/
gchar *gp_rpc_server_get_self_uri (GpRpcServer *gp_rpc_server);


/**
 * gp_rpc_server_create:
 * @uri: адрес сервера;
 * @threads_num: число потоков исполнения на сервере;
 * @data_size: размер буфера приема-передачи в байтах;
 * @data_timeout: максимальное время задержки при передаче данных;
 * @auth: объект для аутентификации типа #GpRpcAuth или NULL;
 * @acl: (skip): функция проверки прав доступа #GpRpcServerAclCallback;
 * @manager: объект callback функций и данных #GpRpcManager;
 * @error: GError или NULL.
 *
 * Создание RPC сервера.
 *
 * Создает RPC сервер заданый адресом uri. Адрес задается в виде строки:
 * "<type>://name:port", где:
 * - &lt;type&gt; - тип RPC ( udp, tcp, shm );
 * - name - имя или ip адрес системы;
 * - port - номер udp или tcp порта.
 *
 * Для IP версии 6 ip адрес должен быть задан в прямых скобках [], например [::1/128].
 * Для shm номер порта может быть любым или отсутствовать.
 *
 * Returns: (transfer full): Указатель на #GpRpcServer объект в случае успеха, иначе NULL.
 *
*/
GpRpcServer *gp_rpc_server_create (const gchar *uri, guint threads_num, guint data_size,
  gdouble data_timeout, GpRpcAuth *auth, GpRpcServerAclCallback acl, GpRpcManager *manager,
  GError **error);


G_END_DECLS

#endif // _gp_rpc_server_h
