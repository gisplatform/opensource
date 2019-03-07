/*
 * GPRPC - rpc (remote procedure call) library, this library is part of GRTL.
 *
 * Copyright 2009, 2010, 2014 Andrei Fadeev
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
 * \file gp-rpc.h
 *
 * \author Andrei Fadeev
 * \date 20.03.2009
 * \brief Заголовочный файл интерфейса удалённых вызовов процедур.
 *
 * \defgroup GpRpc GpRpc - Клиент библиотеки удалённых вызовов процедур.
 *
 * Библиотека реализует механизм передачи rpc запросов от клиента к серверу через
 * определённый канал связи, такой как TCP, UDP или разделяемую область памяти.
 * Библиотека реализована с использованием механизма gobject и представляет собой
 * интерфейс к различным реализациям GPRPC.
 *
 * Клиент GPRPC создается функцией #gp_rpc_create, которая в случае успешного подключения
 * к серверу возвращает указатель на новый объект. Этот объект может использоваться для
 * передачи запросов, исполнения и получения результатов.
 *
 * Взаимодействие клиента сервером происходит следующим образом:
 *
 * - клиент блокирует канал передачи данных функцией #gp_rpc_lock;
 * - клиент регистрирует аргументы для передачи в сервер;
 * - клиент выполняет запрос функцией #gp_rpc_exec;
 * - клиент считывает результаты выполнения;
 * - клиент разблокирует канал передачи данных функцией #gp_rpc_unlock.
 *
 * Регистрация и считывание данных для обмена между сервером и клиентом производится
 * при помощи объекта \link GpRpcData \endlink. Указатель на этот объект возвращает
 * функция #gp_rpc_lock в случае успеха.
 *
 * Клиент вызывает функции сервера используя функцию #gp_rpc_exec, в которую передает
 * идентификаторы функции и объекта с которыми он хочет взаимодействовать на сервере.
 *
 * Функция #gp_rpc_exec возвращает TRUE, если RPC запрос успешно выполнен, но это относится
 * только к механизму RPC. Успешность выполнения самой функции на сервере необходимо
 * передавать отдельно через данные объекта \link GpRpcData \endlink.
 *
 * Для получения локального адреса или адреса сервера можно использовать функции
 * #gp_rpc_get_self_uri и #gp_rpc_get_server_uri. Функции возвращают строку содержащую
 * адрес в формате специфичном для данного типа GPRPC объекта.
 *
 * В случае критической ошибки при обмене данными клиент переходит в отключенное состояние.
 * Узнать текущее состояние клиента можно функцией #gp_rpc_connected.
 *
 * В качестве примера некритичной ошибки стоит привести попытку вызова несуществующей функции.
 * В таком случае gp_rpc_exec выбрасывает исключение GP_RPC_ERROR_NOT_FOUND и,
 * в случае если клиенту не критично отсуствие данной функции, клиент может продолжить работу.
 *
 * В процессе своего взаимодействия клиент и сервер могут использовать различные механизмы
 * аутентификации. Для этого предназначен интерфейс \link GpRpcAuth \endlink. В случае
 * необходимости можно создать объект с интерфейсом \link GpRpcAuth \endlink и передать его
 * GPRPC клиенту для использования.
 *
 * Клиент GPRPC можно создать используя стандартный механизм g_object_new.
 * Доступны следующие типы GPRPC клиентов:
 *
 * - G_TYPE_URPC - обмен через udp/ip;
 * - G_TYPE_TRPC - обмен через tcp/ip;
 * - G_TYPE_SRPC - обмен через разделяемую область памяти.
 *
 * В этом случае возможна передача следующих параметров конструктору:
 *
 * - uri - строка с адресом сервера;
 * - data_size - размер буфера приема-передачи в байтах (для механизмов TCP и SHM);
 * - exec_timeout - максимальное время выполнения запроса в секундах (для механизмов UDP и TCP);
 * - restart - число попыток выполнения запроса (для механизма UDP);
 * - auth - объект для аутентификации типа \link GpRpcAuth \endlink.
 *
 * Параметр uri является обязательным. Для остальных параметров значения по умолчанию следующие:
 *
 * - data_size = GP_RPC_DEFAULT_DATA_SIZE = 65000;
 * - exec_timeout = GP_RPC_DEFAULT_EXEC_TIMEOUT = 5.0;
 * - restart = GP_RPC_DEFAULT_RESTART = 5;
 * - auth = NULL - не используется.
 *
 * Отключение от сервера и удаление клиента производится функцией g_object_unref.
 *
*/

#ifndef _gp_rpc_h
#define _gp_rpc_h

#include <glib-object.h>

#include <gp-rpc-common.h>
#include <gp-rpc-auth.h>
#include <gp-rpc-data.h>

G_BEGIN_DECLS


#define G_TYPE_RPC                  gp_rpc_get_type()
#define GP_RPC( obj )                 ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), G_TYPE_RPC, GpRpc ) )
#define GP_RPC_CLASS( vtable )        ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), G_TYPE_RPC, GpRpcInterface ) )
#define IS_GP_RPC( obj )              ( G_TYPE_CHECK_INSTANCE_TYPE ( ( obj ), G_TYPE_RPC ) )
#define GP_RPC_GET_CLASS( inst )      ( G_TYPE_INSTANCE_GET_INTERFACE (( inst ), G_TYPE_RPC, GpRpcInterface ) )

GType gp_rpc_get_type (void);


typedef struct GpRpc GpRpc;

typedef struct GpRpcInterface {

  GTypeInterface        parent;

  GpRpcData * (*lock)( GpRpc *gp_rpc);
  gboolean  (*exec)( GpRpc *gp_rpc, guint32 proc_id, guint32 obj_id, GError **error );
  void    (*unlock)( GpRpc *gp_rpc);

  gboolean (*connected)( GpRpc *gp_rpc);

  gchar* (*get_self_uri)( GpRpc *gp_rpc);
  gchar* (*get_server_uri)( GpRpc *gp_rpc);

} GpRpcInterface;


/**
 * gp_rpc_lock:
 * @gp_rpc: указатель на интерфейс GpRpc.
 *
 * Блокировка канала передачи.
 *
 * Ждет пока освободится транспортный уровень для передачи данных,
 * обнуляет буфер передаваемых данных и блокирует канал передачи
 * для текущего вызывающего потока.
 *
 * Returns: (transfer none) (allow-none): Указатель на объект #GpRpcData в случае успешного завершения, иначе NULL.
*/
GpRpcData *gp_rpc_lock (GpRpc *gp_rpc);


/**
 * gp_rpc_exec:
 * @gp_rpc: указатель на интерфейс GpRpc;
 * @proc_id: идентификатор вызываемой процедуры;
 * @obj_id: идентификатор вызываемого объекта.
 * @error: GError или NULL.
 *
 * Вызов удалённой процедуры.
 *
 * Производит передачу параметров процедуры, её вызов и передачу
 * результата работы. Успешное завершение функции говорит только
 * о том, что удалённая процедура была вызвана и результат работы
 * получен обратно. Возвращаемое значение удалённой процедуры должно
 * передаваться среди результатов.
 *
 * Returns: TRUE в случае успешного завершения, иначе FALSE и бросается исключение.
*/
gboolean gp_rpc_exec(GpRpc *gp_rpc, guint32 proc_id, guint32 obj_id, GError **error);


/**
 * gp_rpc_unlock:
 * @gp_rpc: указатель на интерфейс GpRpc.
 *
 * Освобождение канала передачи.
 *
 * Освобождает канал передачи делая его доступным другим потокам.
*/
void gp_rpc_unlock (GpRpc *gp_rpc);


/**
 * gp_rpc_connected:
 * @gp_rpc: указатель на интерфейс GpRpc.
 *
 * Проверка наличия соединения
 *
 * Проверяет находится RPC объект в подключенном состоянии или нет.
 *
 * Returns: TRUE если объект подключен, иначе FALSE.
*/
gboolean gp_rpc_connected (GpRpc *gp_rpc);


/**
 * gp_rpc_get_self_uri:
 * @gp_rpc: указатель на интерфейс GpRpc.
 *
 * Получение локального адреса объекта.
 *
 * Возвращает строку содержащую локальный адрес к которому подключен RPC объект.
 * Адрес возвращается в виде как для параметра uri.
 * Получить адрес можно только для объекта в состоянии "Подключен".
 * Пользователь должен освободить полученную строку вызовом g_free.
 *
 * Returns: Строка с адресом, иначе NULL.
*/
gchar *gp_rpc_get_self_uri (GpRpc *gp_rpc);


/**
 * gp_rpc_get_server_uri:
 * @gp_rpc: указатель на интерфейс GpRpc.
 *
 * Получение адреса сервера.
 *
 * Возвращает строку содержащую адрес сервера к которому подключен RPC объект.
 * Адрес возвращается в виде как он был задан в параметре uri.
 * Получить адрес можно только для объекта в состоянии "Подключен".
 * Пользователь должен освободить полученную строку вызовом g_free.
 *
 * Returns: Строка с адресом, иначе NULL.
 *
*/
gchar *gp_rpc_get_server_uri (GpRpc *gp_rpc);


/**
 * gp_rpc_create:
 * @uri: адрес сервера;
 * @data_size: размер буфера приема-передачи в байтах;
 * @exec_timeout: максимальное время выполнения запроса в секундах;
 * @restart: число попыток выполнения запроса (для механизма UDP);
 * @auth: (allow-none): объект для аутентификации типа #GpRpcAuth или NULL;
 * @error: GError или NULL.
 *
 * Создание RPC клиента.
 *
 * Создает RPC клиент и подключает его к серверу заданому адресом uri. Адрес задается в виде строки:
 * "<type>://name:port", где:
 * - &lt;type&gt; - тип RPC ( udp, tcp, shm );
 * - name - имя или ip адрес системы;
 * - port - номер udp или tcp порта.
 *
 * Для IP версии 6 ip адрес должен быть задан в прямых скобках [], например [::1/128].
 * Для shm номер порта может быть любым или отсутствовать.
 *
 * Returns: (transfer full): Указатель на #GpRpc объект в случае успеха, иначе NULL.
 *
*/
GpRpc *gp_rpc_create (const gchar *uri, guint data_size, gdouble exec_timeout, guint restart, GpRpcAuth *auth, GError **error);

G_END_DECLS

#endif // _gp_rpc_h
