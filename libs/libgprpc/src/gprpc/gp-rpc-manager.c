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
 * \file gp_rpc-manager.c
 *
 * \author Andrei Fadeev
 * \date 26.02.2014
 * \brief Исходный файл класса работы с RPC процедурами и параметрами.
 *
*/

#include "gp-rpc-manager.h"

#include <string.h>


typedef struct GpRpcManagerObject {

  gpointer     object;                     // Указатель на хранимый объект.
  gboolean     mt_safe;                    // Поддерживает или нет объект многопоточное использование.
  GRWLock      lock;                       // Блокировка объекта.

} GpRpcManagerObject;

typedef struct GpRpcManagerPriv {

  GRWLock      lock;                       // Блокировка списков.

  GHashTable  *procs;                      // Массив зарегистрированных функций.
  GHashTable  *objs;                       // Массив зарегистрированных объектов.

} GpRpcManagerPriv;


#define GP_RPC_MANAGER_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_RPC_MANAGER, GpRpcManagerPriv ) )


static void gp_rpc_manager_finalize (GpRpcManager *gp_rpc_manager);


G_DEFINE_TYPE( GpRpcManager, gp_rpc_manager, G_TYPE_OBJECT )


static void gp_rpc_manager_class_init (GpRpcManagerClass *klass)
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  this_class->finalize = gp_rpc_manager_finalize;

  g_type_class_add_private( klass, sizeof(GpRpcManagerPriv) );

}


static void gp_rpc_manager_init (GpRpcManager *gp_rpc_manager)
{

  GpRpcManagerPriv *priv = GP_RPC_MANAGER_GET_PRIVATE(gp_rpc_manager);

  priv->procs = g_hash_table_new_full( g_direct_hash, g_direct_equal, NULL, g_free );
  priv->objs = g_hash_table_new_full( g_direct_hash, g_direct_equal, NULL, g_free );

  g_rw_lock_init( &priv->lock );

}


static void gp_rpc_manager_finalize (GpRpcManager *gp_rpc_manager)
{

  GpRpcManagerPriv *priv = GP_RPC_MANAGER_GET_PRIVATE(gp_rpc_manager);

  g_hash_table_unref( priv->procs );
  g_hash_table_unref( priv->objs );

  G_OBJECT_CLASS( gp_rpc_manager_parent_class )->finalize(gp_rpc_manager);

}


static gboolean gp_rpc_manager_reg_object (GHashTable *objs, guint32 obj_id, gpointer object, gboolean mt_safe)
{

  GpRpcManagerObject *obj_info;

  if( g_hash_table_lookup( objs, GUINT_TO_POINTER( obj_id ) ) ) return FALSE;

  obj_info = g_malloc( sizeof(GpRpcManagerObject) );
  obj_info->object = object;
  obj_info->mt_safe = mt_safe;
  g_rw_lock_init( &obj_info->lock );

  g_hash_table_insert( objs, GUINT_TO_POINTER( obj_id ), obj_info );

  return TRUE;

}


static gboolean gp_rpc_manager_unreg_object (GHashTable *objs, guint32 obj_id)
{

  GpRpcManagerObject *obj_info;

  obj_info = g_hash_table_lookup( objs, GUINT_TO_POINTER( obj_id ) );
  if( obj_info != NULL )
  {
    g_rw_lock_writer_lock( &obj_info->lock );
    g_hash_table_remove( objs, GUINT_TO_POINTER( obj_id ) );
  }

  return obj_info ? TRUE : FALSE;

}


static gpointer gp_rpc_manager_get_object (GHashTable *objs, guint32 obj_id)
{

  GpRpcManagerObject *obj_info;

  obj_info = g_hash_table_lookup( objs, GUINT_TO_POINTER( obj_id ) );

  if( obj_info == NULL ) return NULL;

  obj_info->mt_safe ? g_rw_lock_reader_lock( &obj_info->lock ) : g_rw_lock_writer_lock( &obj_info->lock );

  return obj_info->object;

}


static gboolean gp_rpc_manager_release_object (GHashTable *objs, guint32 obj_id)
{

  GpRpcManagerObject *obj_info;

  obj_info = g_hash_table_lookup( objs, GUINT_TO_POINTER( obj_id ) );

  if( obj_info == NULL ) return FALSE;

  obj_info->mt_safe ? g_rw_lock_reader_unlock( &obj_info->lock ) : g_rw_lock_writer_unlock( &obj_info->lock );

  return TRUE;

}


GpRpcManager *gp_rpc_manager_new (void)
{

  return g_object_new( G_TYPE_RPC_MANAGER, NULL );

}



gboolean gp_rpc_manager_reg_proc (GpRpcManager *gp_rpc_manager, guint32 proc_id, GpRpcServerCallback callback,
                                  gboolean mt_safe)
{

  GpRpcManagerPriv *priv = GP_RPC_MANAGER_GET_PRIVATE(gp_rpc_manager);
  gboolean status;

  if( proc_id == 0 ) return FALSE;

  g_rw_lock_writer_lock( &priv->lock );
  status = gp_rpc_manager_reg_object (priv->procs, proc_id, callback, mt_safe);
  g_rw_lock_writer_unlock( &priv->lock );

  return status;

}


gboolean gp_rpc_manager_unreg_proc (GpRpcManager *gp_rpc_manager, guint32 proc_id)
{

  GpRpcManagerPriv *priv = GP_RPC_MANAGER_GET_PRIVATE(gp_rpc_manager);
  gboolean status;

  g_rw_lock_writer_lock( &priv->lock );
  status = gp_rpc_manager_unreg_object (priv->procs, proc_id);
  g_rw_lock_writer_unlock( &priv->lock );

  return status;

}


gboolean gp_rpc_manager_has_proc (GpRpcManager *gp_rpc_manager, guint32 proc_id)
{

  GpRpcManagerPriv *priv = GP_RPC_MANAGER_GET_PRIVATE(gp_rpc_manager);
  gpointer status;

  g_rw_lock_reader_lock( &priv->lock );
  status = g_hash_table_lookup( priv->procs, GUINT_TO_POINTER( proc_id ) );
  g_rw_lock_reader_unlock( &priv->lock );

  return status ? TRUE : FALSE;

}


GpRpcServerCallback gp_rpc_manager_get_proc (GpRpcManager *gp_rpc_manager, guint32 proc_id)
{

  GpRpcManagerPriv *priv = GP_RPC_MANAGER_GET_PRIVATE(gp_rpc_manager);
  gpointer status;
  g_rw_lock_reader_lock( &priv->lock );
  status = gp_rpc_manager_get_object (priv->procs, proc_id);
  g_rw_lock_reader_unlock( &priv->lock );

  return status;

}


gboolean gp_rpc_manager_release_proc (GpRpcManager *gp_rpc_manager, guint32 proc_id)
{

  GpRpcManagerPriv *priv = GP_RPC_MANAGER_GET_PRIVATE(gp_rpc_manager);
  gboolean status;

  g_rw_lock_reader_lock( &priv->lock );
  status = gp_rpc_manager_release_object (priv->procs, proc_id);
  g_rw_lock_reader_unlock( &priv->lock );

  return status;


}


gboolean gp_rpc_manager_reg_obj (GpRpcManager *gp_rpc_manager, guint32 obj_id, gpointer callback, gboolean mt_safe)
{

  GpRpcManagerPriv *priv = GP_RPC_MANAGER_GET_PRIVATE(gp_rpc_manager);
  gboolean status;

  if( obj_id == 0 ) return FALSE;

  g_rw_lock_writer_lock( &priv->lock );
  status = gp_rpc_manager_reg_object (priv->objs, obj_id, callback, mt_safe);
  g_rw_lock_writer_unlock( &priv->lock );

  return status;

}


gboolean gp_rpc_manager_unreg_obj (GpRpcManager *gp_rpc_manager, guint32 obj_id)
{

  GpRpcManagerPriv *priv = GP_RPC_MANAGER_GET_PRIVATE(gp_rpc_manager);
  gboolean status;

  g_rw_lock_writer_lock( &priv->lock );
  status = gp_rpc_manager_unreg_object (priv->objs, obj_id);
  g_rw_lock_writer_unlock( &priv->lock );

  return status;

}


gboolean gp_rpc_manager_has_obj (GpRpcManager *gp_rpc_manager, guint32 obj_id)
{

  GpRpcManagerPriv *priv = GP_RPC_MANAGER_GET_PRIVATE(gp_rpc_manager);
  gpointer status;

  g_rw_lock_reader_lock( &priv->lock );
  status = g_hash_table_lookup( priv->objs, GUINT_TO_POINTER( obj_id ) );
  g_rw_lock_reader_unlock( &priv->lock );

  return status ? TRUE : FALSE;

}


gpointer gp_rpc_manager_get_obj (GpRpcManager *gp_rpc_manager, guint32 obj_id)
{

  GpRpcManagerPriv *priv = GP_RPC_MANAGER_GET_PRIVATE(gp_rpc_manager);
  gpointer status;

  g_rw_lock_reader_lock( &priv->lock );
  status = gp_rpc_manager_get_object (priv->objs, obj_id);
  g_rw_lock_reader_unlock( &priv->lock );

  return status;

}


gboolean gp_rpc_manager_release_obj (GpRpcManager *gp_rpc_manager, guint32 obj_id)
{

  GpRpcManagerPriv *priv = GP_RPC_MANAGER_GET_PRIVATE(gp_rpc_manager);
  gboolean status;

  g_rw_lock_reader_lock( &priv->lock );
  status = gp_rpc_manager_release_object (priv->objs, obj_id);
  g_rw_lock_reader_unlock( &priv->lock );

  return status;


}
