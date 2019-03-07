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
 * \file gp-rpc-manager.h
 *
 * \author Andrei Fadeev
 * \date 26.02.2014
 * \brief Заголовочный файл класса работы с RPC процедурами и параметрами.
 *
 * \defgroup GpRpcManager GpRpcManager - Менеджер функций и объектов GPRPC сервера.
 *
 * Менеджер функций и объектов GPRPC сервера предназначен для хранения указателей
 * на функции и объекты вызываемые клиентом GPRPC. Он может использоваться в нескольких
 * объектах GPRPC серверов одновременно, например на разных портах или адресах TCP.
 *
 * Функции и объекты определяются при помощи числовых идентификаторов. Для пользователя
 * доступны идентификаторы с номерами больше чем \link GP_RPC_PROC_USER \endlink для
 * функций и \link GP_RPC_PARAM_USER \endlink для объектов. Например:
 *
 * - \#define USER_SIN_PROC GP_RPC_PROC_USER + 1
 * - \#define USER_COS_PROC GP_RPC_PROC_USER + 2
 *
 * При получении запроса от клиента GPRPC сервер ищет вызываемую функцию и объект при
 * помощи функций #gp_rpc_manager_get_proc и #gp_rpc_manager_get_obj.
 * При этом если функция или объект не поддерживают многопоточную работу происходит
 * блокировка этой функции или объекта до момента завершения обработки запроса.
 * После завершение обработки запроса функция и объект разблокируются при помощи
 * #gp_rpc_manager_release_proc и #gp_rpc_manager_release_obj.
 *
 * Регистрация и удаление функций и объектов в менеджере возможна как до создания
 * GRCP сервера, так и во время его работы. Регистрация производится функциями
 * #gp_rpc_manager_reg_proc и #gp_rpc_manager_reg_obj, удаление #gp_rpc_manager_unreg_proc
 * и #gp_rpc_manager_unreg_obj.
 *
 * Для работы необходимо зарегистрировать callback функции, которые
 * будут вызываться при поступлении запроса с совпадающим идентификатором proc_id.
 * Также необходимо зарегистрировать объект ( или объекты ), указатели на которые
 * будут передаваться callback функциям.
 *
 * Таким образом если зарегистрировать функции func1 с идентификатором proc_id1 и
 * func2 с идентификатором proc_id2, а также объект obj с идентификатором obj_id,
 * то при RPC запросе #gp_rpc_exec ( rpc, proc_id1, obj_id ) на сервере выполнится функция
 * func1, которой будет передан указатель на объект obj. А при RPC запросе
 * #gp_rpc_exec ( rpc, proc_id2, obj_id ) на сервере выполнится функция func2.
 *
 * Создание объекта GpRpcManager производится функцией #gp_rpc_manager_new.
 * Удаление функцией g_object_unref.
 *
*/

#ifndef _gp_rpc_manager_h
#define _gp_rpc_manager_h

#include <glib-object.h>

#include <gp-rpc-data.h>

G_BEGIN_DECLS


// Описание класса.
#define G_TYPE_RPC_MANAGER                 gp_rpc_manager_get_type()
#define GP_RPC_MANAGER( obj )                ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), G_TYPE_RPC_MANAGER, GpRpcManager ) )
#define GP_RPC_MANAGER_CLASS( vtable )       ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), G_TYPE_RPC_MANAGER, GpRpcManagerClass ) )
#define GP_RPC_MANAGER_GET_CLASS( obj )      ( G_TYPE_INSTANCE_GET_INTERFACE (( obj ), G_TYPE_RPC_MANAGER, GpRpcManagerClass ) )

GType gp_rpc_manager_get_type (void);


typedef GObject GpRpcManager;
typedef GObjectClass GpRpcManagerClass;


/*! Тип callback функции вызываемой при получении запроса на выполнение.
 *
 * \param gp_rpc_manager указатель на объект GpRpcManager;
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param object указатель на объект для которого должна быть вызвана RPC функция.
 *
 * \return TRUE если RPC запрос был выполнен, иначе FALSE.
 *
*/
typedef gboolean (*GpRpcServerCallback)( GpRpcManager *gp_rpc_manager, GpRpcData *gp_rpc_data, gpointer object );


/*! Тип callback функции вызываемой при проверке прав доступа и возможности исполнения запроса клиента.
 *
 * \param gp_rpc_manager указатель на объект GpRpcManager;
 * \param proc_id идентификатор вызываемой функции;
 * \param obj_id идентификатор объекта;
 * \param auth_data данные пользователя из \link GpRpcAuth \endlink для которого производится проверка.
 *
 * \return TRUE если выполнение разрешено, иначе FALSE.
 *
*/
typedef gboolean (*GpRpcServerAclCallback)( GpRpcManager *gp_rpc_manager, guint32 proc_id, guint32 obj_id, gpointer auth_data );


/*! Создание менеджера функций и объектов.
 *
 * \return Указатель на объект GpRpcManager.
 *
*/
GpRpcManager *gp_rpc_manager_new (void);


/**
 * gp_rpc_manager_reg_proc:
 * @gp_rpc_manager: указатель на интерфейс GpRpcManager;
 * @proc_id: идентификатор callback функции;
 * @callback: (skip): callback функция;
 * @mt_safe: возможно или нет вызывать функцию из нескольких потоков.
 *
 * Регистрирует callback функцию с идентификатором proc_id. Эта функция
 * будет вызвана когда клиент вызовет #gp_rpc_exec с соответствующим
 * proc_id. Все переданные параметры доступны в callback функции через объект \link GpRpcData \endlink.
 * Результат работы должен быть передан обратно регистрацией переменных через объект \link GpRpcData \endlink.
 *
 * Returns: TRUE в случае успешного завершения, иначе FALSE.
*/
gboolean gp_rpc_manager_reg_proc (GpRpcManager *gp_rpc_manager, guint32 proc_id, GpRpcServerCallback callback,
                                  gboolean mt_safe);


/*! Удаление callback функции
 *
 * Удаляет callback функцию с идентификатором proc_id из внутреннего списка функций.
 *
 * \param gp_rpc_manager - указатель на интерфейс GpRpcManager;
 * \param proc_id - идентификатор callback функции.
 *
 * \return TRUE в случае успешного завершения, иначе FALSE.
 *
*/
gboolean gp_rpc_manager_unreg_proc (GpRpcManager *gp_rpc_manager, guint32 proc_id);


/*! Проверка callback функции
 *
 * Проверяет, зарегистрирована ли уже callback функция
 * с идентификатором proc_id.
 *
 * \param gp_rpc_manager указатель на интерфейс GpRpcManager;
 * \param proc_id идентификатор callback функции.
 *
 * \return TRUE если функция с таким идентификатором уже зарегистрирована, иначе FALSE.
 *
*/
gboolean gp_rpc_manager_has_proc (GpRpcManager *gp_rpc_manager, guint32 proc_id);


/**
 * gp_rpc_manager_get_proc: (skip)
 * @gp_rpc_manager: указатель на интерфейс GpRpcManager;
 * @proc_id: идентификатор callback функции.
 *
 * Получение указателя на функцию.
 *
 * Если функция не поддерживает многопоточную работу это вызов заблокирует функцию
 * до момента вызова #gp_rpc_manager_release_proc.
 *
 * Returns: Указатель на функцию или NULL - если такой функции нет.
 *
*/
GpRpcServerCallback gp_rpc_manager_get_proc (GpRpcManager *gp_rpc_manager, guint32 proc_id);


/*! Разблокировка функции.
 *
 * \param gp_rpc_manager указатель на интерфейс GpRpcManager;
 * \param proc_id идентификатор callback функции.
 *
 * \return TRUE в случае успеха, FALSE если такой функции нет.
 *
*/
gboolean gp_rpc_manager_release_proc (GpRpcManager *gp_rpc_manager, guint32 proc_id);


/*! Регистрация объекта
 *
 * Регистрирует объект с идентификатором obj_id. Адрес объекта будет
 * передан в callback функцию для RPC вызова с соответствующим obj_id.
 *
 * \param gp_rpc_manager указатель на интерфейс GpRpcManager;
 * \param obj_id идентификатор объекта;
 * \param object адрес объекта;
 * \param mt_safe возможно или нет работать с объектом из нескольких потоков.
 *
 * \return TRUE в случае успешного завершения, иначе FALSE.
 *
*/
gboolean gp_rpc_manager_reg_obj (GpRpcManager *gp_rpc_manager, guint32 obj_id, gpointer object, gboolean mt_safe);


/*! Удаление объекта
 *
 * Удаляет объект с идентификатором obj_id из внутреннего списка объектов.
 *
 * \param gp_rpc_manager указатель на интерфейс GpRpcManager;
 * \param obj_id идентификатор объекта.
 *
 * \return TRUE в случае успешного завершения, иначе FALSE.
 *
*/
gboolean gp_rpc_manager_unreg_obj (GpRpcManager *gp_rpc_manager, guint32 obj_id);


/*! Проверка объекта
 *
 * Проверяет, зарегистрирован ли уже объект
 * с идентификатором obj_id.
 *
 * \param gp_rpc_manager указатель на интерфейс GpRpcManager;
 * \param obj_id идентификатор объекта.
 *
 * \return TRUE если объект с таким идентификатором уже зарегистрирован, иначе FALSE.
 *
*/
gboolean gp_rpc_manager_has_obj (GpRpcManager *gp_rpc_manager, guint32 obj_id);


/*! Получение указателя на объект.
 *
 * Если объект не поддерживает многопоточную работу это вызов заблокирует объект
 * до момента вызова #gp_rpc_manager_release_obj.
 *
 * \param gp_rpc_manager указатель на интерфейс GpRpcManager;
 * \param obj_id идентификатор объекта.
 *
 * \return Указатель на функцию или NULL - если такой функции нет.
 *
*/
gpointer gp_rpc_manager_get_obj (GpRpcManager *gp_rpc_manager, guint32 obj_id);


/*! Разблокировка объекта.
 *
 * \param gp_rpc_manager указатель на интерфейс GpRpcManager;
 * \param obj_id идентификатор объекта.
 *
 * \return TRUE в случае успеха, FALSE если такой функции нет.
 *
*/
gboolean gp_rpc_manager_release_obj (GpRpcManager *gp_rpc_manager, guint32 obj_id);


G_END_DECLS

#endif // _gp_rpc_manager_h
