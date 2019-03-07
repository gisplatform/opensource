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
 * \file srpc-common.c
 *
 * \author Andrei Fadeev
 * \date 19.03.2014
 * \brief Исходный файл общих функций SRPC.
 *
*/

#include "srpc-common.h"


#ifdef G_OS_UNIX

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>


SRpcControl *srpc_create_control( gchar *uri, guint32 threads_num, guint32 data_size )
{

  SRpcControl *control = NULL;
  SRpcControlSHM *control_shm = NULL;
  SRpcTransport **transports = NULL;

  gchar *address = gp_rpc_get_address (uri);

  guint i;
  guint buffer_size = data_size + GP_RPC_HEADER_SIZE;
  gpointer ibuffer;
  gpointer obuffer;

  // Память под структуру.
  control = g_malloc( sizeof( SRpcControl ) );

  control->control_shm_name = g_strdup_printf( "%s.control", address );
  control->control_shm_id = -1;
  control->control_shm_ptr = MAP_FAILED;
  control->control_shm_size = 0;

  control->transport_shm_name = g_strdup_printf( "%s.transport", address );
  control->transport_shm_id = -1;
  control->transport_shm_ptr = MAP_FAILED;
  control->transport_shm_size = 0;

  control->access = SEM_FAILED;
  control->access_name = g_strdup_printf( "%s.access", address );

  control->threads_num = threads_num;
  control->created = TRUE;

  // Проверяем наличие уже созданых областей памяти и работоспособность сервера их создавшего.
  control->control_shm_size = sizeof( SRpcControlSHM );
  control->control_shm_id = shm_open( control->control_shm_name, O_RDONLY, 0 );
  if( control->control_shm_id > 0 )
    {
    control->control_shm_ptr = mmap( NULL, control->control_shm_size, PROT_READ, MAP_SHARED, control->control_shm_id, 0 );
    if( control->control_shm_ptr == MAP_FAILED )
      {
      close( control->control_shm_id );
      control->control_shm_id = -1;
      g_warning( "srpc_common: mmap( %s ) failed ( %s )", control->control_shm_name, strerror( errno ) );
      goto srpc_create_control_fail;
      }
    control_shm = control->control_shm_ptr;
    if( kill( control_shm->pid, 0 ) == 0 )
      { g_warning( "server with pid %d already running at %s", control_shm->pid, uri ); goto srpc_create_control_fail; }
    munmap( control->control_shm_ptr, control->control_shm_size );
    close( control->control_shm_id );
    }

  // Создаем управляющий сегмент shared memory сервера.
  shm_unlink( control->control_shm_name );
  control->control_shm_id = shm_open( control->control_shm_name,  O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );
  if( control->control_shm_id < 0 )
    { g_warning( "srpc_common: shm_open( %s ) failed ( %s )", control->control_shm_name, strerror( errno ) ); goto srpc_create_control_fail; }
  if( ftruncate( control->control_shm_id, control->control_shm_size ) < 0 )
    { g_warning( "srpc_common: ftruncate( %s ) failed ( %s )", control->control_shm_name, strerror( errno ) ); goto srpc_create_control_fail; }
  control->control_shm_ptr = mmap( NULL, control->control_shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, control->control_shm_id, 0 );
  if( control->control_shm_ptr == MAP_FAILED )
    { g_warning( "srpc_common: mmap( %s ) failed ( %s )", control->control_shm_name, strerror( errno ) ); goto srpc_create_control_fail; }

  // Создаем семафор доступа к управляющему сегменту.
  sem_unlink( control->access_name );
  control->access = sem_open( control->access_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, threads_num );
  if( control->access == SEM_FAILED )
    { g_warning( "srpc_common: sem_open( %s ) failed ( %s )", control->access_name, strerror( errno ) ); goto srpc_create_control_fail; }

  // Параметры сервера.
  control_shm = control->control_shm_ptr;
  control_shm->pid = getpid();
  control_shm->data_size = data_size;
  control_shm->threads_num = threads_num;

  // Создаем транспортный сегмент shared memory сервера.
  // Сегмент содержит по два буфера размером data_size и место для переменной gint32 для каждого потока.
  // Буферы используются для обмена информацией с клиентом, а переменная для сигнализирования об использовании.
  shm_unlink( control->transport_shm_name );
  control->transport_shm_size = threads_num * 2 * buffer_size;
  control->transport_shm_id = shm_open( control->transport_shm_name,  O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );
  if( control->transport_shm_id < 0 )
    { g_warning( "srpc_common: shm_open( %s ) failed ( %s )", control->transport_shm_name, strerror( errno ) ); goto srpc_create_control_fail; }
  if( ftruncate( control->transport_shm_id, control->transport_shm_size ) < 0 )
    { g_warning( "srpc_common: ftruncate( %s ) failed ( %s )", control->transport_shm_name, strerror( errno ) ); goto srpc_create_control_fail; }
  control->transport_shm_ptr = mmap( NULL, control->transport_shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, control->transport_shm_id, 0 );
  if( control->transport_shm_ptr == MAP_FAILED )
    { g_warning( "srpc_common: mmap( %s ) failed ( %s )", control->transport_shm_name, strerror( errno ) ); goto srpc_create_control_fail; }

  // Буферы приема и передачи, семафоры вызова функций.
  transports = g_malloc( threads_num * sizeof( SRpcTransport* ) );
  control->transports = transports;

  for( i = 0; i < threads_num; i++ )
    {

    transports[i] = g_malloc( sizeof( SRpcTransport ) );
    transports[i]->start_name = g_strdup_printf( "%s.transport.%u.start", address, i );
    transports[i]->stop_name = g_strdup_printf( "%s.transport.%u.stop", address, i );
    transports[i]->used_name = g_strdup_printf( "%s.transport.%u.used", address, i );
    sem_unlink( transports[i]->start_name );
    sem_unlink( transports[i]->stop_name );
    sem_unlink( transports[i]->used_name );

    ibuffer = control->transport_shm_ptr + i * 2 * buffer_size;
    obuffer = ibuffer +  buffer_size;
    transports[i]->gp_rpc_data = gp_rpc_data_new (buffer_size, GP_RPC_HEADER_SIZE, ibuffer, obuffer);

    transports[i]->start = sem_open( transports[i]->start_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 0 );
    transports[i]->stop = sem_open( transports[i]->stop_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 0 );
    transports[i]->used = sem_open( transports[i]->used_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 1 );

    if( transports[i]->gp_rpc_data == NULL || transports[i]->start == SEM_FAILED || transports[i]->stop == SEM_FAILED || transports[i]->used == SEM_FAILED )
      { g_warning( "srpc_common: sem_open( %s.transport.%u ) failed ( %s )", address, i, strerror( errno ) ); goto srpc_create_control_fail; }

    }

  return control;

  srpc_create_control_fail:

    if( transports != NULL )
      {

      for( i = 0; i < threads_num; i++ )
        {

        if( transports[i] == NULL ) break;

        if( transports[i]->start != SEM_FAILED ) { sem_close( transports[i]->start ); unlink( transports[i]->start_name ); }
        if( transports[i]->stop != SEM_FAILED ) { sem_close( transports[i]->stop ); unlink( transports[i]->stop_name ); }
        if( transports[i]->used != SEM_FAILED ) { sem_close( transports[i]->used ); unlink( transports[i]->used_name ); }
        if( transports[i]->gp_rpc_data != NULL ) g_object_unref( transports[i]->gp_rpc_data);

        g_free( transports[i]->start_name );
        g_free( transports[i]->stop_name );
        g_free( transports[i]->used_name );
        g_free( transports[i] );

        }

      g_free( transports );

      }

    if( control != NULL )
      {

      if( control->control_shm_ptr != MAP_FAILED ) munmap( control->control_shm_ptr, control->control_shm_size );
      if( control->transport_shm_ptr != MAP_FAILED ) munmap( control->transport_shm_ptr, control->transport_shm_size );

      if( control->control_shm_id > 0 ) { close( control->control_shm_id ); shm_unlink( control->control_shm_name ); }
      if( control->transport_shm_id > 0 ) { close( control->transport_shm_id ); shm_unlink( control->transport_shm_name ); }

      if( control->access != SEM_FAILED ) { sem_close( control->access ); sem_unlink( control->access_name ); }

      g_free( control->control_shm_name );
      g_free( control->transport_shm_name );
      g_free( control->access_name );
      g_free( control );

      }

  return NULL;

}


SRpcControl *srpc_open_control( gchar *uri )
{

  SRpcControl *control = NULL;
  SRpcControlSHM *control_shm = NULL;
  SRpcTransport **transports = NULL;

  gchar *address = gp_rpc_get_address (uri);

  guint i;
  guint32 threads_num;
  guint32 data_size;
  guint buffer_size;
  gpointer ibuffer;
  gpointer obuffer;

  // Память под структуру.
  control = g_malloc( sizeof( SRpcControl ) );

  control->control_shm_name = g_strdup_printf( "%s.control", address );
  control->control_shm_id = -1;
  control->control_shm_ptr = MAP_FAILED;
  control->control_shm_size = 0;

  control->transport_shm_name = g_strdup_printf( "%s.transport", address );
  control->transport_shm_id = -1;
  control->transport_shm_ptr = MAP_FAILED;
  control->transport_shm_size = 0;

  control->access = SEM_FAILED;
  control->access_name = g_strdup_printf( "%s.access", address );

  control->threads_num = 0;
  control->created = FALSE;

  // Считываем информацию о сервере.
  control->control_shm_size = sizeof( SRpcControlSHM );
  control->control_shm_id = shm_open( control->control_shm_name,  O_RDONLY, 0 );
  if( control->control_shm_id < 0 )
    { g_warning( "srpc_common: shm_open( %s ) failed ( %s )", control->control_shm_name, strerror( errno ) ); goto srpc_open_control_fail; }
  control->control_shm_ptr = mmap( NULL, control->control_shm_size, PROT_READ, MAP_SHARED, control->control_shm_id, 0 );
  if( control->control_shm_ptr == MAP_FAILED )
    { g_warning( "srpc_common: mmap( %s ) failed ( %s )", control->control_shm_name, strerror( errno ) ); goto srpc_open_control_fail; }

  control_shm = control->control_shm_ptr;
  threads_num = control_shm->threads_num;
  data_size = control_shm->data_size;
  buffer_size = data_size + GP_RPC_HEADER_SIZE;
  control->threads_num = threads_num;

  // Открываем семафор доступа к управляющему сегменту.
  control->access = sem_open( control->access_name, O_RDWR );
  if( control->access == SEM_FAILED )
    { g_warning( "srpc_common: sem_open( %s ) failed ( %s )", control->access_name, strerror( errno ) ); goto srpc_open_control_fail; }

  // Подключаемся к транспортному сегменту shared memory сервера.
  // Для клиента входящий и исходящий буферы меняем местами.
  control->transport_shm_size = threads_num * 2 * buffer_size;
  control->transport_shm_id = shm_open( control->transport_shm_name,  O_RDWR, 0 );
  if( control->transport_shm_id < 0 )
    { g_warning( "srpc_common: shm_open( %s ) failed ( %s )", control->transport_shm_name, strerror( errno ) ); goto srpc_open_control_fail; }
  control->transport_shm_ptr = mmap( NULL, control->transport_shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, control->transport_shm_id, 0 );
  if( control->transport_shm_ptr == MAP_FAILED )
    { g_warning( "srpc_common: mmap( %s ) failed ( %s )", control->transport_shm_name, strerror( errno ) ); goto srpc_open_control_fail; }

  // Буферы приема и передачи, семафоры вызова функций.
  transports = g_malloc( threads_num * sizeof( SRpcTransport* ) );
  control->transports = transports;

  for( i = 0; i < threads_num; i++ )
    {

    transports[i] = g_malloc( sizeof( SRpcTransport ) );
    transports[i]->start_name = g_strdup_printf( "%s.transport.%u.start", address, i );
    transports[i]->stop_name = g_strdup_printf( "%s.transport.%u.stop", address, i );
    transports[i]->used_name = g_strdup_printf( "%s.transport.%u.used", address, i );

    obuffer = control->transport_shm_ptr + i * 2 * buffer_size;
    ibuffer = obuffer +  buffer_size;
    transports[i]->gp_rpc_data = gp_rpc_data_new (buffer_size, GP_RPC_HEADER_SIZE, ibuffer, obuffer);

    transports[i]->start = sem_open( transports[i]->start_name, O_RDWR );
    transports[i]->stop = sem_open( transports[i]->stop_name, O_RDWR );
    transports[i]->used = sem_open( transports[i]->used_name, O_RDWR );

    if( transports[i]->gp_rpc_data == NULL || transports[i]->start == SEM_FAILED || transports[i]->stop == SEM_FAILED || transports[i]->used == SEM_FAILED )
      { g_warning( "srpc_common: sem_open( %s.transport.%u ) failed ( %s )", address, i, strerror( errno ) ); goto srpc_open_control_fail; }

    }

  return control;

  srpc_open_control_fail:

    if( transports != NULL )
      {

      for( i = 0; i < threads_num; i++ )
        {

        if( transports[i] == NULL ) break;

        if( transports[i]->start != SEM_FAILED ) sem_close( transports[i]->start );
        if( transports[i]->stop != SEM_FAILED ) sem_close( transports[i]->stop );
        if( transports[i]->used != SEM_FAILED ) sem_close( transports[i]->used );
        if( transports[i]->gp_rpc_data) g_object_unref( transports[i]->gp_rpc_data);

        g_free( transports[i]->start_name );
        g_free( transports[i]->stop_name );
        g_free( transports[i]->used_name );
        g_free( transports[i] );

        }

      g_free( transports );

      }

    if( control != NULL )
      {

      if( control->control_shm_ptr != MAP_FAILED ) munmap( control->control_shm_ptr, control->control_shm_size );
      if( control->transport_shm_ptr != MAP_FAILED ) munmap( control->transport_shm_ptr, control->transport_shm_size );

      if( control->control_shm_id > 0 ) close( control->control_shm_id );
      if( control->transport_shm_id > 0 ) close( control->transport_shm_id );

      if( control->access != SEM_FAILED ) sem_close( control->access );

      g_free( control->control_shm_name );
      g_free( control->transport_shm_name );
      g_free( control->access_name );
      g_free( control );

      }

  return NULL;

}


void srpc_close_control( SRpcControl *control )
{

  guint i;

  if( !control ) return;

  SRpcTransport **transports = control->transports;

  for( i = 0; i < control->threads_num; i++ )
    {

    sem_close( transports[i]->start );
    sem_close( transports[i]->stop );
    sem_close( transports[i]->used );
    if( control->created ) sem_unlink( transports[i]->start_name );
    if( control->created ) sem_unlink( transports[i]->stop_name );
    if( control->created ) sem_unlink( transports[i]->used_name );
    g_object_unref( transports[i]->gp_rpc_data);
    g_free( transports[i]->start_name );
    g_free( transports[i]->stop_name );
    g_free( transports[i]->used_name );
    g_free( transports[i] );

    }
  g_free( transports );

  munmap( control->control_shm_ptr, control->control_shm_size );
  munmap( control->transport_shm_ptr, control->transport_shm_size );

  close( control->control_shm_id );
  close( control->transport_shm_id );
  if( control->created ) shm_unlink( control->control_shm_name );
  if( control->created ) shm_unlink( control->transport_shm_name );

  sem_close( control->access );
  if( control->created ) sem_unlink( control->access_name );

  g_free( control->control_shm_name );
  g_free( control->transport_shm_name );
  g_free( control->access_name );
  g_free( control );

}


SRpcSemStatus srpc_sem_wait( SEM_TYPE semaphore, gdouble time )
{

  GTimeVal cur_time;
  struct timespec sem_wait_time;

  g_get_current_time( &cur_time );
  g_time_val_add( &cur_time, time * G_USEC_PER_SEC );
  sem_wait_time.tv_sec = cur_time.tv_sec;
  sem_wait_time.tv_nsec = 1000 * cur_time.tv_usec;

  while( TRUE )
    {

    if( sem_timedwait( semaphore, &sem_wait_time ) < 0 )
      {
      if( errno == EINTR ) continue;
      else if( errno == ETIMEDOUT ) return SRPC_SEM_TIMEOUT;
      else return SRPC_SEM_FAIL;
      }
    break;

    }

  return SRPC_SEM_OK;

}


gboolean srpc_sem_try( SEM_TYPE semaphore )
{

  return sem_trywait( semaphore ) == 0 ? TRUE : FALSE;

}


void srpc_sem_post( SEM_TYPE semaphore )
{

  sem_post( semaphore );

}


#elif defined G_OS_WIN32


static gchar error_str[ 1024 ];


gchar *srpc_strerror( void )
{


  FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ), (LPTSTR)&error_str, sizeof( error_str ), NULL );

  return error_str;

}


SRpcControl *srpc_create_control( gchar *uri, guint32 threads_num, guint32 data_size )
{

  SRpcControl *control = NULL;
  SRpcControlSHM *control_shm = NULL;
  SRpcTransport **transports = NULL;

  gchar *address = gp_rpc_get_address( uri );

  guint i;
  guint buffer_size = data_size + GP_RPC_HEADER_SIZE;
  gpointer ibuffer;
  gpointer obuffer;

  // Память под структуру.
  control = g_malloc( sizeof( SRpcControl ) );

  control->control_shm_name = g_strdup_printf( "%s.control", address );
  control->control_shm_id = NULL;
  control->control_shm_ptr = NULL;
  control->control_shm_size = sizeof( SRpcControlSHM );

  control->transport_shm_name = g_strdup_printf( "%s.transport", address );
  control->transport_shm_id = NULL;
  control->transport_shm_ptr = NULL;
  control->transport_shm_size = threads_num * 2 * buffer_size;

  control->access = NULL;
  control->access_name = g_strdup_printf( "%s.access", address );

  control->threads_num = threads_num;
  control->created = TRUE;

  // Создаем управляющий сегмент shared memory сервера.
  control->control_shm_id = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, control->control_shm_size, control->control_shm_name );
  if( control->control_shm_id == NULL )
    { g_warning( "srpc_common: CreateFileMapping( %s ) failed ( %s )", control->control_shm_name, srpc_strerror( ) ); goto srpc_create_control_fail; }
  control->control_shm_ptr = MapViewOfFile( control->control_shm_id, FILE_MAP_WRITE, 0, 0, 0 );
  if( control->control_shm_ptr == NULL )
    { g_warning( "srpc_common: MapViewOfFile( %s ) failed ( %s )", control->control_shm_name, srpc_strerror( ) ); goto srpc_create_control_fail; }

  // Создаем семафор доступа к управляющему сегменту.
  control->access = CreateSemaphore( NULL, threads_num, threads_num, control->access_name );
  if( control->access == NULL )
    { g_warning( "srpc_common: CreateSemaphore( %s ) failed ( %s )", control->access_name, srpc_strerror( ) ); goto srpc_create_control_fail; }

  // Параметры сервера.
  control_shm = control->control_shm_ptr;
  control_shm->pid = 0;
  control_shm->data_size = data_size;
  control_shm->threads_num = threads_num;

  // Создаем транспортный сегмент shared memory сервера.
  // Сегмент содержит по два буфера размером data_size и место для переменной gint32 для каждого потока.
  // Буферы используются для обмена информацией с клиентом, а переменная для сигнализирования об использовании.
  control->transport_shm_id = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, control->transport_shm_size, control->transport_shm_name );
  if( control->transport_shm_id == NULL )
    { g_warning( "srpc_common: CreateFileMapping( %s ) failed ( %s )", control->transport_shm_name, srpc_strerror( ) ); goto srpc_create_control_fail; }
  control->transport_shm_ptr = MapViewOfFile( control->transport_shm_id, FILE_MAP_WRITE, 0, 0, 0 );
  if( control->transport_shm_ptr == NULL )
    { g_warning( "srpc_common: MapViewOfFile( %s ) failed ( %s )", control->transport_shm_name, srpc_strerror( ) ); goto srpc_create_control_fail; }

  // Буферы приема и передачи, семафоры вызова функций.
  transports = g_malloc( threads_num * sizeof( SRpcTransport* ) );
  control->transports = transports;

  for( i = 0; i < threads_num; i++ )
    {

    transports[i] = g_malloc( sizeof( SRpcTransport ) );
    transports[i]->start_name = g_strdup_printf( "%s.transport.%u.start", address, i );
    transports[i]->stop_name = g_strdup_printf( "%s.transport.%u.stop", address, i );
    transports[i]->used_name = g_strdup_printf( "%s.transport.%u.used", address, i );

    ibuffer = control->transport_shm_ptr + i * 2 * buffer_size;
    obuffer = ibuffer +  buffer_size;
    transports[i]->gp_rpc_data = gp_rpc_data_new( buffer_size, GP_RPC_HEADER_SIZE, ibuffer, obuffer );

    transports[i]->start = CreateSemaphore( NULL, 0, 1, transports[i]->start_name );
    transports[i]->stop = CreateSemaphore( NULL, 0, 1, transports[i]->stop_name );
    transports[i]->used = CreateSemaphore( NULL, 1, 1, transports[i]->used_name );

    if( transports[i]->gp_rpc_data == NULL || transports[i]->start == NULL || transports[i]->stop == NULL || transports[i]->used == NULL )
      { g_warning( "srpc_common: CreateSemaphore(  %s.transport.%u ) failed ( %s )", address, i, srpc_strerror( ) ); goto srpc_create_control_fail; }

    }

  return control;

  srpc_create_control_fail:

    if( transports != NULL )
      {

      for( i = 0; i < threads_num; i++ )
        {

        if( transports[i] == NULL ) break;

        if( transports[i]->start != NULL ) CloseHandle( transports[i]->start );
        if( transports[i]->stop != NULL ) CloseHandle( transports[i]->stop );
        if( transports[i]->used != NULL ) CloseHandle( transports[i]->used );
        if( transports[i]->gp_rpc_data ) g_object_unref( transports[i]->gp_rpc_data );

        g_free( transports[i]->start_name );
        g_free( transports[i]->stop_name );
        g_free( transports[i]->used_name );
        g_free( transports[i] );

        }

      g_free( transports );

      }

    if( control != NULL )
      {

      if( control->control_shm_ptr != NULL ) UnmapViewOfFile( control->control_shm_ptr );
      if( control->transport_shm_ptr != NULL ) UnmapViewOfFile( control->transport_shm_ptr );

      if( control->control_shm_id > 0 ) CloseHandle( control->control_shm_id );
      if( control->transport_shm_id > 0 ) CloseHandle( control->transport_shm_id );

      if( control->access != NULL ) CloseHandle( control->access );

      g_free( control->control_shm_name );
      g_free( control->transport_shm_name );
      g_free( control->access_name );
      g_free( control );

      }

  return NULL;

}


SRpcControl *srpc_open_control( gchar *uri )
{

  SRpcControl *control = NULL;
  SRpcControlSHM *control_shm = NULL;
  SRpcTransport **transports = NULL;

  gchar *address = gp_rpc_get_address( uri );

  guint i;
  guint32 threads_num;
  guint32 data_size;
  guint buffer_size;
  gpointer ibuffer;
  gpointer obuffer;

  // Память под структуру.
  control = g_malloc( sizeof( SRpcControl ) );

  control->control_shm_name = g_strdup_printf( "%s.control", address );
  control->control_shm_id = NULL;
  control->control_shm_ptr = NULL;
  control->control_shm_size = sizeof( SRpcControlSHM );

  control->transport_shm_name = g_strdup_printf( "%s.transport", address );
  control->transport_shm_id = NULL;
  control->transport_shm_ptr = NULL;
  control->transport_shm_size = 0;

  control->access = NULL;
  control->access_name = g_strdup_printf( "%s.access", address );

  control->threads_num = 0;
  control->created = FALSE;

  // Считываем информацию о сервере.
  control->control_shm_id = OpenFileMapping( FILE_MAP_READ, FALSE, control->control_shm_name );
  if( control->control_shm_id == NULL )
    { g_warning( "srpc_common: OpenFileMapping( %s ) failed ( %s )", control->control_shm_name, srpc_strerror( ) ); goto srpc_open_control_fail; }
  control->control_shm_ptr = MapViewOfFile( control->control_shm_id, FILE_MAP_READ, 0, 0, 0 );
  if( control->control_shm_ptr == NULL )
    { g_warning( "srpc_common: MapViewOfFile( %s ) failed ( %s )", control->control_shm_name, srpc_strerror( ) ); goto srpc_open_control_fail; }

  control_shm = control->control_shm_ptr;
  threads_num = control_shm->threads_num;
  data_size = control_shm->data_size;
  buffer_size = data_size + GP_RPC_HEADER_SIZE;
  control->threads_num = threads_num;

  // Открываем семафор доступа к управляющему сегменту.
  control->access = OpenSemaphore( SEMAPHORE_ALL_ACCESS, FALSE, control->access_name );
  if( control->access == NULL )
    { g_warning( "srpc_common: OpenSemaphore( %s ) failed ( %s )", control->access_name, srpc_strerror( ) ); goto srpc_open_control_fail; }

  // Подключаемся к транспортному сегменту shared memory сервера.
  // Для клиента входящий и исходящий буферы меняем местами.
  control->transport_shm_id = OpenFileMapping( FILE_MAP_WRITE, FALSE, control->transport_shm_name );
  if( control->transport_shm_id == NULL )
    { g_warning( "srpc_common: OpenFileMapping( %s ) failed ( %s )", control->transport_shm_name, srpc_strerror( ) ); goto srpc_open_control_fail; }
  control->transport_shm_ptr = MapViewOfFile( control->transport_shm_id, FILE_MAP_WRITE, 0, 0, 0 );
  if( control->transport_shm_ptr == NULL )
    { g_warning( "srpc_common: MapViewOfFile( %s ) failed ( %s )", control->transport_shm_name, srpc_strerror( ) ); goto srpc_open_control_fail; }

  // Буферы приема и передачи, семафоры вызова функций.
  transports = g_malloc( threads_num * sizeof( SRpcTransport* ) );
  control->transports = transports;

  for( i = 0; i < threads_num; i++ )
    {

    transports[i] = g_malloc( sizeof( SRpcTransport ) );
    transports[i]->start_name = g_strdup_printf( "%s.transport.%u.start", address, i );
    transports[i]->stop_name = g_strdup_printf( "%s.transport.%u.stop", address, i );
    transports[i]->used_name = g_strdup_printf( "%s.transport.%u.used", address, i );

    obuffer = control->transport_shm_ptr + i * 2 * buffer_size;
    ibuffer = obuffer +  buffer_size;
    transports[i]->gp_rpc_data = gp_rpc_data_new( buffer_size, GP_RPC_HEADER_SIZE, ibuffer, obuffer );

    transports[i]->start = OpenSemaphore( SEMAPHORE_ALL_ACCESS, FALSE, transports[i]->start_name );
    transports[i]->stop = OpenSemaphore( SEMAPHORE_ALL_ACCESS, FALSE, transports[i]->stop_name );
    transports[i]->used = OpenSemaphore( SEMAPHORE_ALL_ACCESS, FALSE, transports[i]->used_name );

    if( transports[i]->gp_rpc_data == NULL || transports[i]->start == NULL || transports[i]->stop == NULL || transports[i]->used == NULL )
      { g_warning( "srpc_common: OPenSemaphore(  %s.transport.%u ) failed ( %s )", address, i, srpc_strerror( ) ); goto srpc_open_control_fail; }

    }

  return control;

  srpc_open_control_fail:

    if( transports != NULL )
      {

      for( i = 0; i < threads_num; i++ )
        {

        if( transports[i] == NULL ) break;

        if( transports[i]->start != NULL ) CloseHandle( transports[i]->start );
        if( transports[i]->stop != NULL ) CloseHandle( transports[i]->stop );
        if( transports[i]->used != NULL ) CloseHandle( transports[i]->used );
        if( transports[i]->gp_rpc_data ) g_object_unref( transports[i]->gp_rpc_data );

        g_free( transports[i]->start_name );
        g_free( transports[i]->stop_name );
        g_free( transports[i]->used_name );
        g_free( transports[i] );

        }

      g_free( transports );

      }

    if( control != NULL )
      {

      if( control->control_shm_ptr != NULL ) UnmapViewOfFile( control->control_shm_ptr );
      if( control->transport_shm_ptr != NULL ) UnmapViewOfFile( control->transport_shm_ptr );

      if( control->control_shm_id > 0 ) CloseHandle( control->control_shm_id );
      if( control->transport_shm_id > 0 ) CloseHandle( control->transport_shm_id );

      if( control->access != NULL ) CloseHandle( control->access );

      g_free( control->control_shm_name );
      g_free( control->transport_shm_name );
      g_free( control->access_name );
      g_free( control );

      }

  return NULL;

}


void srpc_close_control( SRpcControl *control )
{

  guint i;

  if( !control ) return;

  SRpcTransport **transports = control->transports;

  for( i = 0; i < control->threads_num; i++ )
    {

    CloseHandle( transports[i]->start );
    CloseHandle( transports[i]->stop );
    CloseHandle( transports[i]->used );
    g_object_unref( transports[i]->gp_rpc_data );
    g_free( transports[i]->start_name );
    g_free( transports[i]->stop_name );
    g_free( transports[i]->used_name );
    g_free( transports[i] );

    }
  g_free( transports );

  UnmapViewOfFile( control->control_shm_ptr );
  UnmapViewOfFile( control->transport_shm_ptr );

  CloseHandle( control->control_shm_id );
  CloseHandle( control->transport_shm_id );
  CloseHandle( control->access );

  g_free( control->control_shm_name );
  g_free( control->transport_shm_name );
  g_free( control->access_name );
  g_free( control );

}


SRpcSemStatus srpc_sem_wait( SEM_TYPE semaphore, gdouble time )
{

  DWORD wait_time;
  DWORD wait_stat;

  wait_time = 1000 * time;
  wait_stat = WaitForSingleObject( semaphore, wait_time );

  if( wait_stat == WAIT_OBJECT_0 ) return SRPC_SEM_OK;
  if( wait_stat == WAIT_TIMEOUT ) return SRPC_SEM_TIMEOUT;

  return SRPC_SEM_OK;

}


gboolean srpc_sem_try( SEM_TYPE semaphore )
{

  return WaitForSingleObject( semaphore, 0L ) == WAIT_OBJECT_0 ? TRUE : FALSE;

}


void srpc_sem_post( SEM_TYPE semaphore )
{

  ReleaseSemaphore( semaphore, 1, NULL );

}


#endif
