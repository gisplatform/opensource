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
 * \file gp-rpc-auth.h
 *
 * \author Andrei Fadeev
 * \date 19.02.2014
 * \brief Заголовочный файл интерфейса аутентификации.
 *
 * \defgroup GpRpcAuth GpRpcAuth - Интерфейс механизма аутентификации.
 *
 * В общем случае подключение к серверу GPRPC и выполнение запросов возможно для
 * любого клиента имеющего связь по протоколу IP (для механизмов TCP и UDP) или
 * входящего в одну с сервером группу пользователей (для механизма SHM).
 *
 * На данный момент механизм аутентификации позволяет ограничить доступ к серверу,
 * разрешив его только для клиентов с действительным ключом -
 * \link GpRpcAuthCram \endlink и \link GpRpcAuthCramServer \endlink.
 *
 * Интерфейс аутентификации можно расширять, путем реализации дополнительных
 * механизмов, например, с использованием шифрования - но это оставим на потом:)))
 *
 * Интерфейс содержит две основных функции для аутентификации #gp_rpc_auth_check и
 * #gp_rpc_auth_authenticate. Функция #gp_rpc_auth_check вызывается клиентом и сервером
 * для проверки принятого GPRPC пакета. Она возвращает один из следующих статусов:
 *
 * - GP_RPC_AUTH_NOT_AUTHENTICATED - аутентификация еще не проводилась;
 * - GP_RPC_AUTH_IN_PROGRESS - аутентификация находится в процессе работы;
 * - GP_RPC_AUTH_AUTHENTICATED - аутентификация успешно проведена;
 * - GP_RPC_AUTH_AUTHENTICATED2 - аутентификация успешно проведена для повторно переданного пакета;
 * - GP_RPC_AUTH_NOT_SUPPORTED - тип аутентификации не поддерживается;
 * - GP_RPC_AUTH_FAILED - ошибка аутентификации.
 *
 * При подключении к серверу клиент несколько раз вызвает GPRPC функцию GP_RPC_PROC_AUTHENTICATE
 * до тех пор пока статус аутентификации равен GP_RPC_AUTH_IN_PROGRESS. При успешной аутентификации
 * статус должен быть равен GP_RPC_AUTH_AUTHENTICATED или GP_RPC_AUTH_AUTHENTICATED2. Во всех остальных
 * случаях продолжение работы не возможно.
 *
 * Перед отправкой GPRPC пакета клиент и сервер вызывают функцию #gp_rpc_auth_authenticate для того
 * что бы добавить в этот пакет информацию об аутентификации - она будет проверяться другой стороной
 * при вызове функции #gp_rpc_auth_check. Возможно полное изменение данных (например шифрованием), но
 * только не находящихся в заголовке GPRPC пакета (\link GpRpcHeader \endlink).
 *
 * По полям RPC заголовка: session, sequence и size система аутентификации должна определять
 * свое состояние, а остальное содержимое пакета может меняться по усмотрению системы аутентификации.
 *
 * Функция #gp_rpc_auth_get_user_data может вызываться только на сервере и предназначена для получения
 * информации о пользователе запрашивающем выполнение запроса. Эта информация зависит от конкретного
 * метода аутентификации. Затем она передается в функцию проверки прав доступа #GpRpcServerAclCallback.
 *
 * Функция #gp_rpc_auth_clean_session вызывается сервером при отключении клиента. В ответ система
 * аутентификации должна удалить информацию о данной сессии из своего списка сессий.
 *
*/

#ifndef _gp_rpc_auth_h
#define _gp_rpc_auth_h

#include <glib-object.h>

#include <gp-rpc-data.h>

G_BEGIN_DECLS


#define G_TYPE_RPC_AUTH                  gp_rpc_auth_get_type()
#define GP_RPC_AUTH( obj )                 ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), G_TYPE_RPC_AUTH, GpRpcAuth ) )
#define GP_RPC_AUTH_CLASS( vtable )        ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), G_TYPE_RPC_AUTH, GpRpcAuthInterface ) )
#define GP_RPC_AUTH_GET_CLASS( inst )      ( G_TYPE_INSTANCE_GET_INTERFACE (( inst ), G_TYPE_RPC_AUTH, GpRpcAuthInterface ) )

GType gp_rpc_auth_get_type (void);


/*! \brief Статусы аутентификации. */
typedef enum {

  GP_RPC_AUTH_NOT_AUTHENTICATED,  /*!< Аутентификация еще не проводилась. */
      GP_RPC_AUTH_IN_PROGRESS,        /*!< Аутентификация находится в процессе работы. */
      GP_RPC_AUTH_AUTHENTICATED,      /*!< Аутентификация успешно проведена. */
      GP_RPC_AUTH_NOT_SUPPORTED,      /*!< Тип аутентификации не поддерживается. */
      GP_RPC_AUTH_FAILED              /*!< Ошибка аутентификации. */

} GpRpcAuthStatus;


typedef struct GpRpcAuth GpRpcAuth;

typedef struct GpRpcAuthInterface {

  GTypeInterface        parent;

  GpRpcAuthStatus   (*check)( GpRpcAuth *gp_rpc_auth, GpRpcData *gp_rpc_data);

  gboolean  (*authenticate)( GpRpcAuth *gp_rpc_auth, GpRpcData *gp_rpc_data);

  gpointer (*get_user_data)( GpRpcAuth *gp_rpc_auth, GpRpcData *gp_rpc_data);

  gboolean (*clean_session)( GpRpcAuth *gp_rpc_auth, GpRpcData *gp_rpc_data);

} GpRpcAuthInterface;


/*! Проверка аутентификации.
 *
 * \param gp_rpc_auth указатель на интерфейс GpRpcAuth;
 * \param gp_rpc_data указатель на интерфейс \link GpRpcData \endlink;
 *
 * \return Статус аутентификации.
 *
*/
GpRpcAuthStatus gp_rpc_auth_check (GpRpcAuth *gp_rpc_auth, GpRpcData *gp_rpc_data);


/*! Добавление информации об аутентификации в GPRPC пакет.
 *
 * \param gp_rpc_auth указатель на интерфейс GpRpcAuth;
 * \param gp_rpc_data указатель на интерфейс \link GpRpcData \endlink;
 *
 * \return TRUE в случае успеха, FALSE в случае ошибки.
 *
*/
gboolean gp_rpc_auth_authenticate (GpRpcAuth *gp_rpc_auth, GpRpcData *gp_rpc_data);


/*! Получение информации о пользователе сессии.
 *
 * \param gp_rpc_auth указатель на интерфейс GpRpcAuth;
 * \param gp_rpc_data указатель на интерфейс \link GpRpcData \endlink;
 *
 * \return TRUE в случае успеха, FALSE в случае ошибки.
 *
*/
gpointer gp_rpc_auth_get_user_data (GpRpcAuth *gp_rpc_auth, GpRpcData *gp_rpc_data);


/*! Удаление сессии при отключении пользователя.
 *
 * \param gp_rpc_auth указатель на интерфейс GpRpcAuth;
 * \param gp_rpc_data указатель на интерфейс \link GpRpcData \endlink;
 *
 * \return TRUE в случае успеха, FALSE в случае ошибки.
 *
*/
gboolean gp_rpc_auth_clean_session (GpRpcAuth *gp_rpc_auth, GpRpcData *gp_rpc_data);


G_END_DECLS

#endif // _gp_rpc_auth_h
