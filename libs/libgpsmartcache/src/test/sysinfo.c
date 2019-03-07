/* \file sysinfo.c
 *
 *  Created on: 31.05.2018
 *      Author: volkhin.s
 */

#include <gp-smartcache.h>

int main(int argc, char **argv)
{
  gchar *human_readable_sys_memory = NULL;
    guint64 sys_memory = gp_smart_cache_get_sys_memory();
    human_readable_sys_memory = g_format_size(sys_memory);
    g_print("System memory: %s (%"G_GUINT64_FORMAT" bytes)\n", human_readable_sys_memory, sys_memory);
  g_free(human_readable_sys_memory);

  return 0;
}
