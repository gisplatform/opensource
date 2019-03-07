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
 * \file gp_rpc-data.c
 *
 * \author Andrei Fadeev
 * \date 20.02.2014
 * \brief Исходный файл класса работы с RPC данными.
 *
*/

#include "gp-rpc-data.h"
#include "gp-rpc-common.h"

#include <string.h>


enum { PROP_O, PROP_BUFFER_SIZE, PROP_HEADER_SIZE, PROP_IBUFFER, PROP_OBUFFER };

#define DATA_ALIGN_SIZE  sizeof( guint32 ) // Минимальный размер переменной.

typedef struct DataBuffer {

  gpointer     data;                       // Указатель на данные в буфере приемо-передачи.
  guint32      buffer_size;                // Размер буфера.
  guint32      data_size;                  // Размер данных.

} DataBuffer;

typedef struct DataParam {

  guint32      id;                         // Идентификатор переменной.
  guint32      size;                       // Размер переменной.
  guint32      next;                       // Смещение от начала текущего заголовка до следующего, если == 0 - это последняя переменная.
  gchar        data[ DATA_ALIGN_SIZE ];    // Данные, реальный размер содержится в size, но не менее DATA_ALIGN_SIZE байт.

} DataParam;

typedef struct GpRpcDataPriv {

  guint32      buffer_size;                // Размер всего буфера.
  guint32      header_size;                // Размер заголовка в буфера перед данными.
  gpointer     ibuffer;                    // Буфер входящих данных.
  gpointer     obuffer;                    // Буфер исходящих данных.

  DataBuffer   input;
  DataBuffer   output;

} GpRpcDataPriv;


#define GP_RPC_DATA_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_RPC_DATA, GpRpcDataPriv ) )


static void gp_rpc_data_set_property (GpRpcData *gp_rpc_data, guint prop_id, const GValue *value, GParamSpec *pspec);
static GObject*gp_rpc_data_constructor (GType g_type, guint n_construct_properties,
                                        GObjectConstructParam *construct_properties);


G_DEFINE_TYPE( GpRpcData, gp_rpc_data, G_TYPE_OBJECT )

static void gp_rpc_data_init (GpRpcData *gp_rpc_data) { ; }


static void gp_rpc_data_class_init (GpRpcDataClass *klass)
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  this_class->set_property = gp_rpc_data_set_property;
  this_class->constructor = gp_rpc_data_constructor;

  g_object_class_install_property( this_class, PROP_BUFFER_SIZE,
                                   g_param_spec_uint( "buffer-size", "Buffer size", "Buffer size", 0, G_MAXUINT, 0, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_HEADER_SIZE,
                                   g_param_spec_uint( "header-size", "Header size", "Header size", 0, G_MAXUINT, 0, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_IBUFFER,
                                   g_param_spec_pointer( "input-buffer", "Input buffer", "Input buffer", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_OBUFFER,
                                   g_param_spec_pointer( "output-buffer", "Output buffer", "Output buffer", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_type_class_add_private( klass, sizeof(GpRpcDataPriv) );

}


static void gp_rpc_data_set_property (GpRpcData *gp_rpc_data, guint prop_id, const GValue *value, GParamSpec *pspec)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  switch ( prop_id )
  {

    case PROP_BUFFER_SIZE:
      priv->buffer_size = g_value_get_uint( value );
      break;

    case PROP_HEADER_SIZE:
      priv->header_size = g_value_get_uint( value );
      break;

    case PROP_IBUFFER:
      priv->ibuffer = g_value_get_pointer( value );
      break;

    case PROP_OBUFFER:
      priv->obuffer = g_value_get_pointer( value );
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(gp_rpc_data, prop_id, pspec );
      break;

  }

}


static GObject*gp_rpc_data_constructor (GType g_type, guint n_construct_properties,
                                        GObjectConstructParam *construct_properties)
{

  GObject *gp_rpc_data = G_OBJECT_CLASS( gp_rpc_data_parent_class )->constructor( g_type, n_construct_properties, construct_properties );
  if( gp_rpc_data == NULL ) return NULL;

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  if( priv->ibuffer == NULL || priv->obuffer == NULL || priv->header_size >= priv->buffer_size )
  { g_object_unref(gp_rpc_data); return NULL; }

  priv->input.data = priv->ibuffer + priv->header_size;
  priv->input.data_size = 0;
  priv->input.buffer_size = priv->buffer_size - priv->header_size;

  priv->output.data = priv->obuffer + priv->header_size;
  priv->output.data_size = 0;
  priv->output.buffer_size = priv->buffer_size - priv->header_size;

  return gp_rpc_data;

}


static DataParam *gp_rpc_data_find_param (DataBuffer *buffer, guint32 id)
{

  DataParam *param = buffer->data;
  guint32 left_size = buffer->data_size;   // Размер данных в буфере.
  guint32 offset = 0;
  guint32 cur_param_size;

  if( buffer->data_size == 0 ) return NULL;

  while( TRUE )
  {

    guint32 param_id;
    guint32 param_size;
    guint32 param_next;

    // Проверяем, что в буфере присутствуют данные как минимум размера структуры RpcParam.
    if( left_size < sizeof( DataParam ) - DATA_ALIGN_SIZE )
    { g_warning( "gp_rpc_data: buffer error" ); return NULL; }

    param_id = GUINT32_FROM_BE( param->id );
    param_size = GUINT32_FROM_BE( param->size );
    param_next = GUINT32_FROM_BE( param->next );

    // Вычисляем размер занимаемый текущим проверяемым параметром и сравниваем
    // его с размером данных в буфере.
    cur_param_size = param_size + sizeof( DataParam ) - DATA_ALIGN_SIZE;
    if( cur_param_size > left_size )
    { g_warning( "gp_rpc_data: buffer error" ); return NULL; }

    // Если идентификаторы совпали, возвращаем указатель на искомый параметр.
    // Последний параметр в списке можно определить по нулевому смещению до
    // следующего параметра. В этом случае завершаем поиск.
    if( param_id == id ) return param;
    if( param_next == 0 ) break;

    // "Уменьшаем" размер данных в буфере на размер текущего параметра,
    // переходим к следующему параметру и повторяем проверки.
    left_size -= param_next;
    offset += param_next;
    param = buffer->data + offset;

  }

  // Если искомый параметр не был найден возвращается указатель на последний
  // параметр в буфере.
  return param;

}


static gpointer gp_rpc_data_set_param (DataBuffer *buffer, guint32 id, gconstpointer object, guint32 size)
{

  guint32 param_id;
  guint32 param_size;
  guint32 param_next;

  DataParam *param = gp_rpc_data_find_param (buffer, id);

  if( param != NULL  )
  {
    param_id = GUINT32_FROM_BE( param->id );
    param_size = GUINT32_FROM_BE( param->size );
    param_next = GUINT32_FROM_BE( param->next );
  }

  // Если rpc_set вызван для последнего зарегистрированного параметра, а так-же
  // если object == NULL, то изменим размер этого параметра. Дополнительно проверяется
  // наличие достаточного свободного места в буфере при увеличении размера параметра.
  if( param != NULL && param_id == id && param_next == 0 && object == NULL )
  if( ( size < param_size ) || ( ( buffer->buffer_size - buffer->data_size ) >= ( size - param_size ) ) )
  {
    buffer->data_size = buffer->data_size - param_size + size;
    param->size = GUINT32_TO_BE( size );
    return param->data;
  }

  // Параметр с таким идентификатором уже есть.
  if( param != NULL && param_id == id )
  {
    if( param_size == size ) // Если размер совпадает, установим значение.
    { memcpy( param->data, object, size ); return param->data; }
    else // Иначе вернем ошибку.
    { g_warning( "gp_rpc_data: parameter %d was already registered", id ); return NULL; }
  }

  // Проверка оставшегося места в буфере.
  if( ( buffer->buffer_size - buffer->data_size ) < ( size + sizeof( DataParam ) ) )
  { g_warning( "gp_rpc_data: parameter %d too large", id ); return NULL; }

  // Смещение до следующего параметра с выравниванием.
  if( param != NULL )
  {
    guint32 data_pad = ( DATA_ALIGN_SIZE - ( param_size % DATA_ALIGN_SIZE ) );
    data_pad = ( data_pad == DATA_ALIGN_SIZE ) ? 0 : data_pad;
    param->next = GUINT32_TO_BE( param_size + sizeof( DataParam ) - DATA_ALIGN_SIZE + data_pad );
    memset( buffer->data + buffer->data_size, 0, data_pad );
    buffer->data_size += data_pad;
  }

  // Запоминаем параметр в буфере.
  param = buffer->data + buffer->data_size;
  buffer->data_size += size + sizeof( DataParam ) - DATA_ALIGN_SIZE;
  param->id = GUINT32_TO_BE( id );
  param->size = GUINT32_TO_BE( size );
  param->next = 0;

  if( object != NULL )
    memcpy( param->data, object, size );

  return param->data;

}


static gpointer gp_rpc_data_get_param (DataBuffer *buffer, guint32 id, guint32 *size)
{

  DataParam *param = gp_rpc_data_find_param (buffer, id);

  // Буфер пуст или параметр не найден.
  if( param == NULL || GUINT32_FROM_BE( param->id ) != id ) return NULL;

  if( size ) *size = GUINT32_FROM_BE( param->size );
  return param->data;

}


GpRpcData *gp_rpc_data_new (guint32 buffer_size, guint32 header_size, gpointer ibuffer, gpointer obuffer)
{

  return g_object_new( G_TYPE_RPC_DATA, "buffer-size", buffer_size, "header-size", header_size, "input-buffer", ibuffer, "output-buffer", obuffer, NULL );

}


guint32 gp_rpc_data_get_header_size (GpRpcData *gp_rpc_data)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  return priv->header_size;

}


gboolean gp_rpc_data_set_header_size (GpRpcData *gp_rpc_data, guint32 header_size)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  if( header_size >= priv->buffer_size ) return FALSE;

  priv->header_size = header_size;

  priv->input.data = priv->ibuffer + priv->header_size;
  priv->input.data_size = 0;
  priv->input.buffer_size = priv->buffer_size - priv->header_size;

  priv->output.data = priv->obuffer + priv->header_size;
  priv->output.data_size = 0;
  priv->output.buffer_size = priv->buffer_size - priv->header_size;

  return TRUE;

}


gpointer gp_rpc_data_get_header (GpRpcData *gp_rpc_data, GpRpcDataDirection direction)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  if( direction == GP_RPC_DATA_INPUT) return priv->ibuffer;
  else if( direction == GP_RPC_DATA_OUTPUT) return priv->obuffer;

  return NULL;

}


gboolean gp_rpc_data_set_header (GpRpcData *gp_rpc_data, GpRpcDataDirection direction, gpointer header,
                                 guint32 header_size)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  if( header_size != priv->header_size ) return FALSE;

  if( direction == GP_RPC_DATA_INPUT) memcpy( priv->ibuffer, header, priv->header_size );
  else if( direction == GP_RPC_DATA_OUTPUT) memcpy( priv->obuffer, header, priv->header_size );
  else return FALSE;

  return TRUE;

}


guint32 gp_rpc_data_get_data_size (GpRpcData *gp_rpc_data, GpRpcDataDirection direction)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  if( direction == GP_RPC_DATA_INPUT) return priv->input.data_size;
  else if( direction == GP_RPC_DATA_OUTPUT) return priv->output.data_size;
  return 0;

}


gboolean gp_rpc_data_set_data_size (GpRpcData *gp_rpc_data, GpRpcDataDirection direction, guint32 data_size)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);
  DataBuffer   *data_buffer;

  if( direction == GP_RPC_DATA_INPUT) data_buffer = &priv->input;
  else if( direction == GP_RPC_DATA_OUTPUT) data_buffer = &priv->output;
  else return FALSE;

  if( data_buffer->buffer_size < data_size ) return FALSE;

  if( data_buffer->data_size > data_size)
    memset( data_buffer->data + data_size, 0, data_buffer->data_size - data_size );

  data_buffer->data_size = data_size;

  return TRUE;

}


gpointer gp_rpc_data_get_data (GpRpcData *gp_rpc_data, GpRpcDataDirection direction)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  if( direction == GP_RPC_DATA_INPUT) return priv->input.data;
  else if( direction == GP_RPC_DATA_OUTPUT) return priv->output.data;

  return NULL;

}


gboolean  gp_rpc_data_set_data (GpRpcData *gp_rpc_data, GpRpcDataDirection direction, gpointer data, guint32 data_size)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);
  DataBuffer   *data_buffer;

  if( direction == GP_RPC_DATA_INPUT) data_buffer = &priv->input;
  else if( direction == GP_RPC_DATA_OUTPUT) data_buffer = &priv->output;
  else return FALSE;

  if( data_buffer->buffer_size < data_size ) return FALSE;

  memcpy( data_buffer->data, data, data_size );

  if( data_buffer->data_size > data_size)
    memset( data_buffer->data + data_size, 0, data_buffer->data_size - data_size );

  data_buffer->data_size = data_size;

  return TRUE;

}


gboolean gp_rpc_data_validate (GpRpcData *gp_rpc_data, GpRpcDataDirection direction)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  DataBuffer *buffer;
  DataParam *param;
  guint32 left_size;
  guint32 offset = 0;
  guint32 cur_param_size;

  if( direction == GP_RPC_DATA_INPUT) buffer = &priv->input;
  else if( direction == GP_RPC_DATA_OUTPUT) buffer = &priv->output;
  else return FALSE;

  param = buffer->data;
  left_size = buffer->data_size;

  if( buffer->data_size == 0 ) return TRUE;

  while( TRUE )
  {

    guint32 param_size;
    guint32 param_next;

    // Проверяем, что в буфере присутствуют данные как минимум размера структуры RpcParam.
    if( left_size < sizeof( DataParam ) )
    { g_warning( "gp_rpc_data: buffer error" ); return FALSE; }

    param_size = GUINT32_FROM_BE( param->size );
    param_next = GUINT32_FROM_BE( param->next );

    // Вычисляем размер занимаемый текущим проверяемым параметром и сравниваем
    // его с размером данных в буфере.
    cur_param_size = param_size + sizeof( DataParam ) - DATA_ALIGN_SIZE;
    if( cur_param_size > left_size )
    { g_warning( "gp_rpc_data: buffer error" ); return FALSE; }

    // Последний параметр в списке можно определить по нулевому смещению до
    // следующего параметра. В этом случае завершаем поиск.
    if( param_next == 0 ) break;

    // "Уменьшаем" размер данных в буфере на размер текущего параметра,
    // переходим к следующему параметру и повторяем проверки.
    left_size -= param_next;
    offset += param_next;
    param = buffer->data + offset;

  }

  // Если искомый параметр не был найден возвращается указатель на последний
  // параметр в буфере.
  return TRUE;

}


gboolean gp_rpc_data_is_set (GpRpcData *gp_rpc_data, guint32 id)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  DataParam *param = gp_rpc_data_find_param (&priv->output, id);

  if( param == NULL || GUINT32_FROM_BE( param->id ) != id ) return FALSE;

  return TRUE;

}


gpointer gp_rpc_data_set (GpRpcData *gp_rpc_data, guint32 id, gconstpointer object, guint32 size)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  return gp_rpc_data_set_param (&priv->output, id, object, size);

}


gpointer gp_rpc_data_get (GpRpcData *gp_rpc_data, guint32 id, guint32 *size)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  return gp_rpc_data_get_param (&priv->input, id, size);

}


gboolean gp_rpc_data_set_int32 (GpRpcData *gp_rpc_data, guint32 id, gint32 value)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  gint32 set_value = GINT32_TO_BE( value );

  return gp_rpc_data_set_param (&priv->output, id, &set_value, sizeof (gint32)) == NULL ? FALSE : TRUE;

}


gint32 gp_rpc_data_get_int32 (GpRpcData *gp_rpc_data, guint32 id)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  gint32 *get_value;
  guint32 value_size;

  get_value = gp_rpc_data_get_param (&priv->input, id, &value_size);
  if( get_value == NULL || value_size != sizeof( gint32 ) ) return 0;

  return GINT32_FROM_BE( *get_value );

}


gboolean gp_rpc_data_set_uint32 (GpRpcData *gp_rpc_data, guint32 id, guint32 value)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  guint32 set_value = GUINT32_TO_BE( value );

  return gp_rpc_data_set_param (&priv->output, id, &set_value, sizeof (guint32)) == NULL ? FALSE : TRUE;

}


guint32 gp_rpc_data_get_uint32 (GpRpcData *gp_rpc_data, guint32 id)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  guint32 *get_value;
  guint32 value_size;

  get_value = gp_rpc_data_get_param (&priv->input, id, &value_size);
  if( get_value == NULL || value_size != sizeof( guint32 ) ) return 0;

  return GUINT32_FROM_BE( *get_value );

}


gboolean gp_rpc_data_set_int64 (GpRpcData *gp_rpc_data, guint32 id, gint64 value)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  gint64 set_value = GINT64_TO_BE( value );

  return gp_rpc_data_set_param (&priv->output, id, &set_value, sizeof (gint64)) == NULL ? FALSE : TRUE;

}


gint64 gp_rpc_data_get_int64 (GpRpcData *gp_rpc_data, guint32 id)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  gint64 *get_value;
  guint32 value_size;

  get_value = gp_rpc_data_get_param (&priv->input, id, &value_size);
  if( get_value == NULL || value_size != sizeof( gint64 ) ) return 0;

  return GINT64_FROM_BE( *get_value );

}


gboolean gp_rpc_data_set_uint64 (GpRpcData *gp_rpc_data, guint32 id, guint64 value)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  guint64 set_value = GUINT64_TO_BE( value );

  return gp_rpc_data_set_param (&priv->output, id, &set_value, sizeof (guint64)) == NULL ? FALSE : TRUE;

}


guint64 gp_rpc_data_get_uint64 (GpRpcData *gp_rpc_data, guint32 id)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  guint64 *get_value;
  guint32 value_size;

  get_value = gp_rpc_data_get_param (&priv->input, id, &value_size);
  if( get_value == NULL || value_size != sizeof( guint64 ) ) return 0;

  return GUINT64_FROM_BE( *get_value );

}


gboolean gp_rpc_data_set_float (GpRpcData *gp_rpc_data, guint32 id, gfloat value)
{

  gpointer pvalue = &value;
  guint32  uvalue = *(guint32*)pvalue;

  return gp_rpc_data_set_uint32 (gp_rpc_data, id, uvalue);

}


gfloat gp_rpc_data_get_float (GpRpcData *gp_rpc_data, guint32 id)
{

  guint32  uvalue = gp_rpc_data_get_uint32 (gp_rpc_data, id);
  gpointer pvalue = &uvalue;

  return *(gfloat*)pvalue;

}


gboolean gp_rpc_data_set_double (GpRpcData *gp_rpc_data, guint32 id, gdouble value)
{

  gpointer pvalue = &value;
  guint64  uvalue = *(guint64*)pvalue;

  return gp_rpc_data_set_uint64 (gp_rpc_data, id, uvalue);

}


gdouble gp_rpc_data_get_double (GpRpcData *gp_rpc_data, guint32 id)
{

  guint64  uvalue = gp_rpc_data_get_uint64 (gp_rpc_data, id);
  gpointer pvalue = &uvalue;

  return *(gdouble*)pvalue;

}


gboolean gp_rpc_data_set_string (GpRpcData *gp_rpc_data, guint32 id, const gchar *string)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  return gp_rpc_data_set_param (&priv->output, id, string, strlen (string) + 1) == NULL ? FALSE : TRUE;

}


gchar *gp_rpc_data_get_string (GpRpcData *gp_rpc_data, guint32 id)
{

  GpRpcDataPriv *priv = GP_RPC_DATA_GET_PRIVATE(gp_rpc_data);

  guint32 size;
  gchar *string;

  string = gp_rpc_data_get_param (&priv->input, id, &size);
  if( !string || string[ size - 1 ] != 0 ) return NULL;

  return string;

}


gchar *gp_rpc_data_dup_string (GpRpcData *gp_rpc_data, guint32 id)
{

  return g_strdup(gp_rpc_data_get_string (gp_rpc_data, id) );

}
