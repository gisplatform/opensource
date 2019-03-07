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
 * \file gp-rpc-data.h
 *
 * \author Andrei Fadeev
 * \date 20.02.2014
 * \brief Заголовочный файл класса работы с RPC данными.
 *
 * \defgroup GpRpcData GpRpcData - Буфер приема-передачи RPC данных.
 *
 * Буфер используется для хранения данных в процессе RPC обмена. Он состоит из
 * двух буферов: приема и передачи. Данные могут быть зарегистрированы в буфере
 * и считаны из него. При регистрации данные автоматически размещаются в
 * буфере передачи, а считываются из буфера приема.
 *
 * Каждый буфер может хранить дополнительный заголовок, в котором система
 * GPRPC размещает свои служебные данные.
 *
 * При регистрации данных функцией #gp_rpc_data_set возвращается указатель на область
 * памяти. Пользователь может записывать и считывать данные из этой области в
 * границах заданного размера. Эти данные будут переданы серверу или клиенту в неизменном виде.
 *
 * Указатель на принятые данные можно получить функцией #gp_rpc_data_get.
 *
 * Так как клиент и сервер могут работать на разных архитектурах, включая архитектуры
 * с отличающимся порядком следования байт необходимо учитывать это при разработке. Для
 * упрощения обмена стандартными типами данных реализованы функции gp_rpc_data_set_&lt;type&gt; и
 * gp_rpc_data_get_&lt;type&gt;, где &lt;type&gt; один следуюших типов данных:
 *
 *  - int32 - 32-х битный знаковый целый;
 *  - uint32 - 32-х битный беззнаковый целый;
 *  - int64 - 64-х битный знаковый целый;
 *  - uint64 - 64-х битный беззнаковый целый;
 *  - float - число с плавающей запятой одинарной точности;
 *  - double - число с плавающей запятой двойной точности;
 *  - string - строка с нулем на конце.
 *
 * Для строковых данных дополнительно доступна функция #gp_rpc_data_dup_string которая
 * возвращает указатель на копию строки из буфера.
 *
 * При использовании этих функций будет автоматически происходить преобразование данных
 * в зависимости от архитектуры.
 *
 * Создание объекта GpRpcData производится функцией #gp_rpc_data_new. Память под буферы
 * должна быть выделена заранее.
 *
 * Удаление объекта производится функцией g_object_unref.
 *
*/

#ifndef _gp_rpc_data_h
#define _gp_rpc_data_h

#include <glib-object.h>

G_BEGIN_DECLS


typedef enum {
  GP_RPC_DATA_INPUT, GP_RPC_DATA_OUTPUT
} GpRpcDataDirection;


// Описание класса.
#define G_TYPE_RPC_DATA                 gp_rpc_data_get_type()
#define GP_RPC_DATA( obj )                ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), G_TYPE_RPC_DATA, GpRpcData ) )
#define GP_RPC_DATA_CLASS( vtable )       ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), G_TYPE_RPC_DATA, GpRpcDataClass ) )
#define GP_RPC_DATA_GET_CLASS( obj )      ( G_TYPE_INSTANCE_GET_INTERFACE (( obj ), G_TYPE_RPC_DATA, GpRpcDataClass ) )

GType gp_rpc_data_get_type (void);


typedef GObject GpRpcData;
typedef GObjectClass GpRpcDataClass;


/*! Создание объекта для работы с RPC данными.
 *
 * При работе с GPRPC этот объект автоматически создается клиентом или сервером.
 *
 * \param buffer_size размер каждого из буферов приема и передачи в байтах;
 * \param header_size размер заголовка в начале каждого буфера в байтах;
 * \param ibuffer указатель на буфер принимаемых данных;
 * \param obuffer указатель на буфер отправляемых данных.
 *
 * \return Указатель на объект GpRpcData или NULL в случае ошибки.
 *
*/
GpRpcData *gp_rpc_data_new (guint32 buffer_size, guint32 header_size, gpointer ibuffer, gpointer obuffer);


/*! Возвращает размер заголовка в начале буфера.
 *
 * \param gp_rpc_data указатель на объект GpRpcData.
 *
 * \return Размер заголовка в байтах.
 *
*/
guint32 gp_rpc_data_get_header_size (GpRpcData *gp_rpc_data);


/*! Изменяет размер заголовка в начале буфера.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param header_size новый размер заголовка в байтах.
 *
 * \return TRUE если размер заголовка изменен, FALSE в случае ошибки.
*/
gboolean gp_rpc_data_set_header_size (GpRpcData *gp_rpc_data, guint32 header_size);


/*! Получает указатель на заголовок в начале буфера.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param direction тип буфера принимаемых - GPRPC_DATA_INPUT или отправляемых - GPRPC_DATA_OUTPUT данных.
 *
 * \return Указатель на заголовок в начале буфера.
 *
*/
gpointer gp_rpc_data_get_header (GpRpcData *gp_rpc_data, GpRpcDataDirection direction);


/*! Записывает данные в заголовок в начале буфера.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param direction тип буфера принимаемых - GPRPC_DATA_INPUT или отправляемых - GPRPC_DATA_OUTPUT данных;
 * \param header указатель на данные для записи в заголовок;
 * \param header_size размер данных для записи в заголовок.
 *
 * \return TRUE если данные были записаны, FALSE в случае ошибки.
 *
*/
gboolean gp_rpc_data_set_header (GpRpcData *gp_rpc_data, GpRpcDataDirection direction, gpointer header,
                                 guint32 header_size);


/*! Получает размер который занимают данные в буфере.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param direction тип буфера принимаемых - GPRPC_DATA_INPUT или отправляемых - GPRPC_DATA_OUTPUT данных.
 *
 * \return Размер данных в буфере в байтах.
 *
*/
guint32 gp_rpc_data_get_data_size (GpRpcData *gp_rpc_data, GpRpcDataDirection direction);


/*! Изменяет размер который занимают данные в буфере.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param direction тип буфера принимаемых - GPRPC_DATA_INPUT или отправляемых - GP_RPC_DATA_OUTPUT данных;
 * \param data_size новый размер данных в буфере.
 *
 * \return TRUE если размер данных изменен, FALSE в случае ошибки.
 *
*/
gboolean gp_rpc_data_set_data_size (GpRpcData *gp_rpc_data, GpRpcDataDirection direction, guint32 data_size);


/*! Получает указатель на данные в буфере.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param direction тип буфера принимаемых - GPRPC_DATA_INPUT или отправляемых - GP_RPC_DATA_OUTPUT данных.
 *
 * \return Указатель на данные в буфере.
 *
*/
gpointer gp_rpc_data_get_data (GpRpcData *gp_rpc_data, GpRpcDataDirection direction);


/*! Записывает данные в буфер.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param direction тип буфера принимаемых - GP_RPC_DATA_INPUT или отправляемых - GP_RPC_DATA_OUTPUT данных;
 * \param data указатель на данные для записи в буфер;
 * \param data_size размер данных для записи в буфер.
 *
 * \return TRUE если данные были записаны, FALSE в случае ошибки.
 *
*/
gboolean gp_rpc_data_set_data (GpRpcData *gp_rpc_data, GpRpcDataDirection direction, gpointer data, guint32 data_size);


/*! Проверяет границы данных в буфере.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param direction тип буфера принимаемых - GP_RPC_DATA_INPUT или отправляемых - GP_RPC_DATA_OUTPUT данных.
 *
 * \return TRUE если смещения и границы данных не выходят за размер буфера, FALSE в случае ошибки.
 *
*/
gboolean gp_rpc_data_validate (GpRpcData *gp_rpc_data, GpRpcDataDirection direction);


/*! Проверяет зарегистрирована переменная в буфере исходящих данных или нет.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной.
 *
 * \return TRUE если переменная с таким идентификатором уже была зарегистрирована, иначе FALSE.
 *
*/
gboolean  gp_rpc_data_is_set (GpRpcData *gp_rpc_data, guint32 id);


/*! Запись значения / регистрация / изменение размера переменной в буфере передачи
 *
 * Записывает значение или регистрирует переменную по ее идентификатору
 * в буфере передачи или изменяет размер уже зарегистрированной переменной.
 *
 * Можно изменить размер только самой последней зарегистрированной переменной и только
 * в сторону уменьшения её размера. При этом object должен равняться NULL.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной;
 * \param object указатель на записываемые данные (может быть NULL);
 * \param size размер данных.
 *
 * \return Адрес переменной в буфере в случае успешного завершения, иначе NULL.
 *
*/
gpointer  gp_rpc_data_set (GpRpcData *gp_rpc_data, guint32 id, gconstpointer object, guint32 size);


/*! Получение указателя на переменную в буфере приема по идентификатору
 *
 * Возвращает указатель на хранимую переменную и ее размер по идентификатору
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной;
 * \param size указатель по которому будет сохранен размер переменной (может быть NULL).
 *
 * \return Указатель на переменную в буфере в случае успешного завершения, иначе NULL.
 *
*/
gpointer  gp_rpc_data_get (GpRpcData *gp_rpc_data, guint32 id, guint32 *size);


/*! Регистрация переменной типа 32-х битный знаковый целый в буфере передачи.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной;
 * \param value значение переменной.
 *
 * \return TRUE в случае успешной регистрации, FALSE в случае ошибки.
 *
*/
gboolean gp_rpc_data_set_int32 (GpRpcData *gp_rpc_data, guint32 id, gint32 value);


/*! Считывание значения переменной типа 32-х битный знаковый целый из буфера приема.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной.
 *
 * \return Значение переменной. Если переменная не зарегистрирована возвращается 0.
 *
*/
gint32 gp_rpc_data_get_int32 (GpRpcData *gp_rpc_data, guint32 id);


/*! Регистрация переменной типа 32-х битный беззнаковый целый в буфере передачи.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной;
 * \param value значение переменной.
 *
 * \return TRUE в случае успешной регистрации, FALSE в случае ошибки.
 *
*/
gboolean gp_rpc_data_set_uint32 (GpRpcData *gp_rpc_data, guint32 id, guint32 value);


/*! Считывание значения переменной типа 32-х битный беззнаковый целый из буфера приема.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной.
 *
 * \return Значение переменной. Если переменная не зарегистрирована возвращается 0.
 *
*/
guint32 gp_rpc_data_get_uint32 (GpRpcData *gp_rpc_data, guint32 id);


/*! Регистрация переменной типа 64-х битный знаковый целый в буфере передачи.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной;
 * \param value значение переменной.
 *
 * \return TRUE в случае успешной регистрации, FALSE в случае ошибки.
 *
*/
gboolean gp_rpc_data_set_int64 (GpRpcData *gp_rpc_data, guint32 id, gint64 value);


/*! Считывание значения переменной типа 64-х битный знаковый целый из буфера приема.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной.
 *
 * \return Значение переменной. Если переменная не зарегистрирована возвращается 0.
 *
*/
gint64 gp_rpc_data_get_int64 (GpRpcData *gp_rpc_data, guint32 id);


/*! Регистрация переменной типа 64-х битный беззнаковый целый в буфере передачи.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной;
 * \param value значение переменной.
 *
 * \return TRUE в случае успешной регистрации, FALSE в случае ошибки.
 *
*/
gboolean gp_rpc_data_set_uint64 (GpRpcData *gp_rpc_data, guint32 id, guint64 value);


/*! Считывание значения переменной типа 64-х битный беззнаковый целый из буфера приема.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной.
 *
 * \return Значение переменной. Если переменная не зарегистрирована возвращается 0.
 *
*/
guint64 gp_rpc_data_get_uint64 (GpRpcData *gp_rpc_data, guint32 id);


/*! Регистрация переменной с плавающей запятой одинарной точности в буфере передачи.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной;
 * \param value значение переменной.
 *
 * \return TRUE в случае успешной регистрации, FALSE в случае ошибки.
 *
*/
gboolean gp_rpc_data_set_float (GpRpcData *gp_rpc_data, guint32 id, gfloat value);


/*! Считывание значения переменной с плавающей запятой одинарной точности из буфера приема.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной.
 *
 * \return Значение переменной. Если переменная не зарегистрирована возвращается 0.
 *
*/
gfloat gp_rpc_data_get_float (GpRpcData *gp_rpc_data, guint32 id);


/*! Регистрация переменной с плавающей запятой двойной точности в буфере передачи.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной;
 * \param value значение переменной.
 *
 * \return TRUE в случае успешной регистрации, FALSE в случае ошибки.
 *
*/
gboolean gp_rpc_data_set_double (GpRpcData *gp_rpc_data, guint32 id, gdouble value);


/*! Считывание значения переменной с плавающей запятой двойной точности из буфера приема.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной.
 *
 * \return Значение переменной. Если переменная не зарегистрирована возвращается 0.
 *
*/
gdouble gp_rpc_data_get_double (GpRpcData *gp_rpc_data, guint32 id);


/*! Регистрация строки с нулем на конце в буфере передачи.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной;
 * \param string строка для регистрации.
 *
 * \return TRUE в случае успешной регистрации, FALSE в случае ошибки.
 *
*/
gboolean gp_rpc_data_set_string (GpRpcData *gp_rpc_data, guint32 id, const gchar *string);


/*! Считывание значения строки из буфера приема.
 *
 *  Функция возвращает указатель на данные в буфере. Этот указатель действителен только
 *  во время блокировки канала передачи. Его содержимое доступно только для чтения.
 *  При необходимости пользователь может получить копию этой строки функцией #gp_rpc_data_dup_string.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной.
 *
 * \return Указатель на строку из буфера или NULL если переменная не зарегистрирована.
 *
*/
gchar *gp_rpc_data_get_string (GpRpcData *gp_rpc_data, guint32 id);


/*! Считывание значения строки из буфера приема.
 *
 *  Функция возвращает указатель на копию строки с данными из буфера.
 *  Пользователь должен самостоятельно освободить память функцией g_free.
 *
 * \param gp_rpc_data указатель на объект GpRpcData;
 * \param id идентификатор переменной.
 *
 * \return Указатель на новую строку или NULL если переменная не зарегистрирована.
 *
*/
gchar *gp_rpc_data_dup_string (GpRpcData *gp_rpc_data, guint32 id);


G_END_DECLS

#endif // _gp_rpc_data_h
