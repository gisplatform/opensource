/*
 * GP_SMART_CACHE - data caching library, this library is part of GRTL.
 *
 * Copyright 2010 Zankov Peter
 *
 * This file is part of GRTL.
 *
 * GRTL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * GRTL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with GRTL. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gp-smartcache.h>

#ifdef G_OS_WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif

// Idea src: https://stackoverflow.com/a/26639774
guint64 gp_smart_cache_get_sys_memory()
{
#ifdef G_OS_WIN32
  MEMORYSTATUSEX status;
  status.dwLength = sizeof(status);
  GlobalMemoryStatusEx(&status);
  // На win можно также посмотреть размер свапа в поле status.ullTotalPageFile
  return (guint64)status.ullTotalPhys;
#else
  long pages = sysconf(_SC_PHYS_PAGES);
  long page_size = sysconf(_SC_PAGE_SIZE);
  return (guint64)pages * page_size;
#endif
}

