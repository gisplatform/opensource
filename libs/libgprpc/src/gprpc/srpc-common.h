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
 * \file srpc-common.h
 *
 * \author Andrei Fadeev
 * \date 19.03.2014
 * \brief Заголовочный файл общих функций SRPC.
 *
*/

#ifndef _srpc_common_h
#define _srpc_common_h

#include <glib.h>

#ifdef G_OS_UNIX

#include <semaphore.h>
typedef sem_t* SEM_TYPE;
typedef gint   SHM_TYPE;

#elif defined G_OS_WIN32

#include <windows.h>
typedef HANDLE SEM_TYPE;
typedef HANDLE SHM_TYPE;

#else

#error "Unsupported platform"

#endif

#include "gp-rpc-common.h"
#include "gp-rpc-data.h"


typedef struct SRpcTransport {

  GpRpcData *gp_rpc_data;

  SEM_TYPE        start;
  SEM_TYPE        stop;
  SEM_TYPE        used;

  gchar          *start_name;
  gchar          *stop_name;
  gchar          *used_name;

} SRpcTransport;


typedef struct SRpcControl {

  gchar          *control_shm_name;
  SHM_TYPE        control_shm_id;
  gpointer        control_shm_ptr;
  gsize           control_shm_size;

  gchar          *transport_shm_name;
  SHM_TYPE        transport_shm_id;
  gpointer        transport_shm_ptr;
  gsize           transport_shm_size;

  SEM_TYPE        access;
  gchar          *access_name;

  guint32         threads_num;

  SRpcTransport **transports;

  gboolean        created;

} SRpcControl;


typedef struct SRpcControlSHM {

  guint32         pid;
  guint32         data_size;
  guint32         threads_num;

} SRpcControlSHM;


typedef enum {

  SRPC_SEM_OK,
  SRPC_SEM_TIMEOUT,
  SRPC_SEM_FAIL

} SRpcSemStatus;



SRpcControl *srpc_create_control( gchar *uri, guint32 threads_num, guint32 data_size );
SRpcControl *srpc_open_control( gchar *uri );
void srpc_close_control( SRpcControl *control );


SRpcSemStatus srpc_sem_wait( SEM_TYPE semaphore, gdouble time );
gboolean srpc_sem_try( SEM_TYPE semaphore );
void srpc_sem_post( SEM_TYPE semaphore );


#endif // _srpc_common_h
