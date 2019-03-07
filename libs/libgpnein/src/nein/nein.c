/*
 * This file is based on gssdp-client.c from https://github.com/GNOME/gssdp.
 *
 * Copyright (C) 2006, 2007, 2008 OpenedHand Ltd.
 * Copyright (C) 2009 Nokia Corporation.
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *         Zeeshan Ali (Khattak) <zeeshanak@gnome.org>
 *                               <zeeshan.ali@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gp-nein.h>

#include <sys/types.h>

#ifndef G_OS_WIN32
  #include <sys/socket.h>
  #include <sys/utsname.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
#else
  #define _WIN32_WINNT 0x502
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <iphlpapi.h>
  typedef int socklen_t;
  /* from the return value of inet_addr */
  typedef unsigned long in_addr_t;
#endif

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#ifndef G_OS_WIN32
  #include <arpa/inet.h>
  #include <net/if.h>
  #include <ifaddrs.h>
#endif

#ifndef INET6_ADDRSTRLEN
  #define INET6_ADDRSTRLEN 46
#endif


G_DEFINE_BOXED_TYPE(GpNein, gp_nein, gp_nein_copy, gp_nein_free)


GpNein *gp_nein_new(const gchar *iface, const gchar *ip)
{
  GpNein *nein = g_new(GpNein, 1);

  nein->iface = g_strdup(iface);
  nein->ip = g_strdup(ip);

  return nein;
}


GpNein *gp_nein_copy(GpNein *src)
{
  return gp_nein_new(src->iface, src->ip);
}


void gp_nein_free(GpNein *nein)
{
  g_free(nein->iface);
  g_free(nein->ip);
  g_free(nein);
}

GQuark gp_nein_error_quark(void)
{
  return g_quark_from_static_string("gpnein-error");
}



#ifdef G_OS_WIN32
  static gboolean is_primary_adapter(PIP_ADAPTER_ADDRESSES adapter)
  {
    int family = adapter->FirstUnicastAddress->Address.lpSockaddr->sa_family;
    return !(adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK || family == AF_INET6);
  }
#endif

GpNein *gp_nein_collect(GSocketFamily family, guint *len, GError **error)
{
  g_return_val_if_fail(len, NULL);
  GPtrArray *rval = g_ptr_array_new();

  #ifdef G_OS_WIN32
    gboolean wsa_started_up = FALSE;
    GList *up_ifaces = NULL, *ifaceptr = NULL;
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_MULTICAST;
    DWORD size = 15360; /* Use 15k buffer initially as documented in MSDN */
    DWORD ret;
    PIP_ADAPTER_ADDRESSES adapters_addresses;
    PIP_ADAPTER_ADDRESSES adapter;
    WSADATA wsaData = {0};

    if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
      gchar *message = g_win32_error_message (WSAGetLastError());
        g_set_error_literal(error, GP_NEIN_ERROR, GP_NEIN_ERROR_FAILED, message);
      g_free (message);

      goto end;
    }
    else
      wsa_started_up = TRUE;

    ULONG lfamily;
    switch(family)
    {
      case G_SOCKET_FAMILY_INVALID:
        lfamily = AF_UNSPEC;
      break;

      case G_SOCKET_FAMILY_IPV4:
        lfamily = AF_INET;
      break;

      case G_SOCKET_FAMILY_IPV6:
        lfamily = AF_INET6;
      break;

      default:
        g_set_error(error, GP_NEIN_ERROR, GP_NEIN_ERROR_FAILED, "Wrong GSocketFamily: %d.", family);
      goto end;
    }

    do
    {
      adapters_addresses = (PIP_ADAPTER_ADDRESSES)g_malloc0(size);
      ret = GetAdaptersAddresses(lfamily, flags, NULL, adapters_addresses, &size);

      if(ret == ERROR_BUFFER_OVERFLOW)
        g_free(adapters_addresses);
    }
    while(ret == ERROR_BUFFER_OVERFLOW);

    if(ret == ERROR_SUCCESS)
      for(adapter = adapters_addresses; adapter != NULL; adapter = adapter->Next)
      {
        if(adapter->FirstUnicastAddress == NULL)
          continue;

        if(adapter->OperStatus != IfOperStatusUp)
          continue;

        /* skip Point-to-Point devices */
        if(adapter->IfType == IF_TYPE_PPP)
          continue;

        /* I think that IPv6 is done via pseudo-adapters, so
         * that there are either IPv4 or IPv6 addresses defined
         * on the adapter.
         * Loopback-Devices and IPv6 go to the end of the list,
         * IPv4 to the front
         */
        if(is_primary_adapter(adapter))
          up_ifaces = g_list_prepend(up_ifaces, adapter);
        else
          up_ifaces = g_list_append(up_ifaces, adapter);
      }

    for(ifaceptr = up_ifaces; ifaceptr != NULL; ifaceptr = ifaceptr->next)
    {
      char ip[INET6_ADDRSTRLEN];
      PIP_ADAPTER_UNICAST_ADDRESS address;

      PIP_ADAPTER_ADDRESSES adapter = (PIP_ADAPTER_ADDRESSES)ifaceptr->data;

      for(address = adapter->FirstUnicastAddress; address != NULL; address = address->Next)
      {
        DWORD len = INET6_ADDRSTRLEN;

        switch(address->Address.lpSockaddr->sa_family)
        {
          case AF_INET:
          case AF_INET6:
            if(WSAAddressToStringA(address->Address.lpSockaddr, address->Address.iSockaddrLength, NULL, ip, &len) == 0)
              g_ptr_array_add(rval, gp_nein_new(adapter->AdapterName, ip));
          break;

          default:
            continue;
        }
      }
    }

    g_list_free(up_ifaces);
    g_free(adapters_addresses);

  #else
    struct ifaddrs *ifa_list, *ifa;
    GList *up_ifaces, *ifaceptr;

    up_ifaces = NULL;

    if(getifaddrs(&ifa_list) != 0)
    {
      g_set_error(error, GP_NEIN_ERROR, GP_NEIN_ERROR_FAILED,
        "Failed to retrieve list of network interfaces: %s\n", strerror (errno));
      goto end;
    }

    for(ifa = ifa_list; ifa != NULL; ifa = ifa->ifa_next)
    {
      if(ifa->ifa_addr == NULL)
        continue;

      if(!(ifa->ifa_flags & IFF_UP))
        continue;
      else
        if((ifa->ifa_flags & IFF_POINTOPOINT))
          continue;

      /* Loopback and IPv6 interfaces go at the bottom on the list */
      if(ifa->ifa_flags & IFF_LOOPBACK || ifa->ifa_addr->sa_family == AF_INET6)
        up_ifaces = g_list_append(up_ifaces, ifa);
      else
        up_ifaces = g_list_prepend(up_ifaces, ifa);
    }

    for(ifaceptr = up_ifaces; ifaceptr != NULL; ifaceptr = ifaceptr->next)
    {
      char ip[INET6_ADDRSTRLEN];
      const char *p = NULL;

      ifa = ifaceptr->data;

      switch(ifa->ifa_addr->sa_family)
      {
        case AF_INET:
          if(family == G_SOCKET_FAMILY_IPV4 || family == G_SOCKET_FAMILY_INVALID)
          {
            struct sockaddr_in *s4;
            s4 = (struct sockaddr_in *)ifa->ifa_addr;
            p = inet_ntop(AF_INET, &s4->sin_addr, ip, sizeof(ip));
          }
        break;

        case AF_INET6:
          if(family == G_SOCKET_FAMILY_IPV6 || family == G_SOCKET_FAMILY_INVALID)
          {
            struct sockaddr_in6 *s6;
            s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
            p = inet_ntop(AF_INET6, &s6->sin6_addr, ip, sizeof(ip));
          }
        break;

        default:
          continue; /* Unknown: ignore */
      }

      if(p != NULL)
        g_ptr_array_add(rval, gp_nein_new(ifa->ifa_name, p));
    }

    g_list_free(up_ifaces);
    freeifaddrs(ifa_list);
  #endif

end:

  #ifdef G_OS_WIN32
    if(wsa_started_up)
      WSACleanup();
  #endif

  *len = rval->len;
  return (GpNein*)g_ptr_array_free(rval, rval->len == 0);
}
