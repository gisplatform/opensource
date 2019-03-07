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
 * \file gp-rpc-auth-cram-common.h
 *
 * \author Andrei Fadeev
 * \date 24.02.2014
 * \brief Общие константы класса взаимной аутентификации на сервере по механизму запрос-ответ.
 *
*/

#ifndef _grpc_auth_cram_common_h
#define _gp_rpc_auth_cram_common_h


#define GP_RPC_AUTH_CRAM_MAX_HASH_SIZE       4096                            // Максимальный возможный размер хэша в байтах.

#define GP_RPC_AUTH_CRAM_MIN_KEY_SIZE           8                            // Минимально возможный размер ключа.
#define GP_RPC_AUTH_CRAM_MAX_KEY_SIZE        1024                            // Максимально возможный размер ключа.

#define GP_RPC_AUTH_CRAM_MIN_CHALENGE_SIZE      8                            // Минимально возможный размер строки запроса.
#define GP_RPC_AUTH_CRAM_MAX_CHALENGE_SIZE   1024                            // Максимально возможный размер строки запроса.


// Идентификаторы параметров модуля аутентификации.
#define GP_RPC_AUTH_CRAM_PARAM_STATUS        ( GP_RPC_PARAM_AUTH_TYPE + 1 )    // Статус аутентификации.
#define GP_RPC_AUTH_CRAM_PARAM_HASH          ( GP_RPC_PARAM_AUTH_TYPE + 2 )    // Тип используемой хэш функциию.
#define GP_RPC_AUTH_CRAM_PARAM_SIGNED_SIZE   ( GP_RPC_PARAM_AUTH_TYPE + 3 )    // Размер подписаных данных.
#define GP_RPC_AUTH_CRAM_PARAM_CHALENGE_S    ( GP_RPC_PARAM_AUTH_TYPE + 4 )    // Строка запроса сервера.
#define GP_RPC_AUTH_CRAM_PARAM_CHALENGE_C    ( GP_RPC_PARAM_AUTH_TYPE + 5 )    // Строка запроса клиента.
#define GP_RPC_AUTH_CRAM_PARAM_RESPONSE_S    ( GP_RPC_PARAM_AUTH_TYPE + 6 )    // Строка ответа сервера.
#define GP_RPC_AUTH_CRAM_PARAM_RESPONSE_C    ( GP_RPC_PARAM_AUTH_TYPE + 7 )    // Строка ответа клиента.


// Идентификаторы значений параметров.
#define GP_RPC_AUTH_CRAM_TYPE_ID             0x4352414D                      // Идентификатор типа аутентификации - строка 'CRAM';

#define GP_RPC_AUTH_CRAM_HASH_MD5            0x4D443520                      // Идентифиакторы типа хэш функции MD5 - строка 'MD5 ',
#define GP_RPC_AUTH_CRAM_HASH_SHA1           0x53484131                      // SHA1 - строка 'SHA1',
#define GP_RPC_AUTH_CRAM_HASH_SHA256         0x53484132                      // SHA256 - строка 'SHA2'.
#define GP_RPC_AUTH_CRAM_HASH_UNKNOWN        0x0

#define GP_RPC_AUTH_CRAM_STATUS_ZERO         0x0                             // Начальное состояние до аутентификации.
#define GP_RPC_AUTH_CRAM_STATUS_BEGIN        0x1000                          // Получение строки запроса от сервера.
#define GP_RPC_AUTH_CRAM_STATUS_ONLY_ONE     0x2000                          // Ответ сервера на получение строки запроса.
#define GP_RPC_AUTH_CRAM_STATUS_AUTH         0x3000                          // Аутентификация верна, сгенерирована новая строка запроса.
#define GP_RPC_AUTH_CRAM_STATUS_RESPONSE     0x4000                          // Ответ сервера/клиента.
#define GP_RPC_AUTH_CRAM_STATUS_CHECKED      0x5000                          // Ответ проверен и верен.
#define GP_RPC_AUTH_CRAM_STATUS_FAIL         0x6000                          // Аутентификация провалена.


#endif // _gp_rpc_auth_cram_common_h
