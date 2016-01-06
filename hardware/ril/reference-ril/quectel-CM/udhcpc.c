/*
 * Copyright 2008, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *     http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifdef ANDROID
#include "QMIThread.h"
#include <netutils/ifc.h>
#include <cutils/properties.h>
extern int do_dhcp();
extern int ifc_init();
extern void ifc_close();
extern char *dhcp_lasterror();
extern void get_dhcp_info();
static const char *ipaddr_to_string(in_addr_t addr)
{
    struct in_addr in_addr;

    in_addr.s_addr = addr;
    return inet_ntoa(in_addr);
}

void udhcpc_start(const char *ifname) {
    if (fork() == 0) {
        uint32_t ipaddr, gateway, prefixLength, dns1, dns2, server, lease;
        char propKey[PROPERTY_VALUE_MAX];
    
        if(ifc_init()) {
            dbg_time("failed to ifc_init(%s): %s\n", ifname, strerror(errno));
        }

        if (do_dhcp(ifname) < 0) {
            dbg_time("failed to do_dhcp(%s): %s\n", ifname, strerror(errno));
        }

        ifc_close();

        get_dhcp_info(&ipaddr,  &gateway,  &prefixLength, &dns1, &dns2, &server, &lease);
        snprintf(propKey, sizeof(propKey), "net.%s.gw", ifname);
        property_set(propKey, gateway ? ipaddr_to_string(gateway) : "0.0.0.0"); 
        exit(0); 
    }
}

void udhcpc_stop(const char *ifname) {
    ifc_init();
    //ifc_remove_host_routes(ifname); //Android will do it by itself
    ifc_set_addr(ifname, 0);
    ifc_close();
}

#else

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <net/if.h>
extern unsigned int if_nametoindex(const char *);
#include <netdb.h>

#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/netlink.h>
#include <linux/route.h>
#include <linux/ipv6_route.h>
#include <linux/rtnetlink.h>
#include <linux/sockios.h>
#include "QMIThread.h"

static int ifc_ctl_sock = -1;

static int ifc_init(void)
{
    int ret;
    if (ifc_ctl_sock == -1) {
        ifc_ctl_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (ifc_ctl_sock < 0) {
            dbg_time("socket() failed: %s\n", strerror(errno));
        }
    }

    ret = ifc_ctl_sock < 0 ? -1 : 0;
    return ret;
}

static void ifc_close(void)
{
    if (ifc_ctl_sock != -1) {
        (void)close(ifc_ctl_sock);
        ifc_ctl_sock = -1;
    }
}

static void ifc_init_ifr(const char *name, struct ifreq *ifr)
{
    memset(ifr, 0, sizeof(struct ifreq));
    strncpy(ifr->ifr_name, name, IFNAMSIZ);
    ifr->ifr_name[IFNAMSIZ - 1] = 0;
}

static int ifc_set_flags(const char *name, unsigned set, unsigned clr)
{
    struct ifreq ifr;
    ifc_init_ifr(name, &ifr);

    if(ioctl(ifc_ctl_sock, SIOCGIFFLAGS, &ifr) < 0) return -1;
    ifr.ifr_flags = (ifr.ifr_flags & (~clr)) | set;
    return ioctl(ifc_ctl_sock, SIOCSIFFLAGS, &ifr);
}

static int ifc_up(const char *name)
{
    int ret = ifc_set_flags(name, IFF_UP, 0);
    return ret;
}

static void init_sockaddr_in(struct sockaddr *sa, in_addr_t addr)
{
    struct sockaddr_in *sin = (struct sockaddr_in *) sa;
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    sin->sin_addr.s_addr = addr;
}

static int ifc_set_addr(const char *name, in_addr_t addr)
{
    struct ifreq ifr;
    int ret;

    ifc_init_ifr(name, &ifr);
    init_sockaddr_in(&ifr.ifr_addr, addr);

    ret = ioctl(ifc_ctl_sock, SIOCSIFADDR, &ifr);
    return ret;
}

/*
 * Remove the routes associated with the named interface.
 */
static int ifc_remove_host_routes(const char *name)
{
    char ifname[64];
    in_addr_t dest, gway, mask;
    int flags, refcnt, use, metric, mtu, win, irtt;
    struct rtentry rt;
    FILE *fp;
    struct in_addr addr;

    fp = fopen("/proc/net/route", "r");
    if (fp == NULL)
        return -1;
    /* Skip the header line */
    if (fscanf(fp, "%*[^\n]\n") < 0) {
        fclose(fp);
        return -1;
    }
    //ifc_init();
    for (;;) {
        int nread = fscanf(fp, "%63s%X%X%X%d%d%d%X%d%d%d\n",
                           ifname, &dest, &gway, &flags, &refcnt, &use, &metric, &mask,
                           &mtu, &win, &irtt);
        if (nread != 11) {
            break;
        }
        if ((flags & (RTF_UP|RTF_HOST)) != (RTF_UP|RTF_HOST)
                || strcmp(ifname, name) != 0) {
            continue;
        }
        memset(&rt, 0, sizeof(rt));
        rt.rt_dev = (void *)name;
        init_sockaddr_in(&rt.rt_dst, dest);
        init_sockaddr_in(&rt.rt_gateway, gway);
        init_sockaddr_in(&rt.rt_genmask, mask);
        addr.s_addr = dest;
        if (ioctl(ifc_ctl_sock, SIOCDELRT, &rt) < 0) {
            dbg_time("failed to remove route for %s to %s: %s\n", ifname, inet_ntoa(addr), strerror(errno));
        }
    }
    fclose(fp);
    //ifc_close();
    return 0;
}

static pthread_attr_t udhcpc_thread_attr;
static pthread_t udhcpc_thread_id;
static void* udhcpc_thread_function(void*  arg) {
    FILE * udhcpc_fp;
    char udhcpc_cmd[64];
    
    //-f,--foreground	Run in foreground
    //-b,--background	Background if lease is not obtained
    //-n,--now		Exit if lease is not obtained
    //-q,--quit		Exit after obtaining lease
    snprintf(udhcpc_cmd, sizeof(udhcpc_cmd), "udhcpc -f -n -q -i %s", (char *)arg);
    
    udhcpc_fp = popen(udhcpc_cmd, "r");
    if (udhcpc_fp) {
        char buf[0xff];
        while((fgets(buf, sizeof(buf), udhcpc_fp)) != NULL) {
            if ((strlen(buf) > 1) && (buf[strlen(buf) - 1] == '\n'))
                buf[strlen(buf) - 1] = '\0';
            dbg_time("%s", buf);
        }
        pclose(udhcpc_fp);
    }

    return NULL;
}

void udhcpc_start(const char *ifname) {
    ifc_init();
    if (ifc_set_addr(ifname, 0)) {
        dbg_time("failed to set ip addr for %s to 0.0.0.0: %s\n", ifname, strerror(errno));
        return;
    }

    if (ifc_up(ifname)) {
        dbg_time("failed to bring up interface %s: %s\n", ifname, strerror(errno));
        return;
    }
    ifc_close();

    pthread_attr_init(&udhcpc_thread_attr);
    pthread_attr_setdetachstate(&udhcpc_thread_attr, PTHREAD_CREATE_DETACHED);
    if(pthread_create(&udhcpc_thread_id, &udhcpc_thread_attr, udhcpc_thread_function, (void*)ifname) !=0 ) {
        dbg_time("failed to create udhcpc_thread for %s: %s\n", ifname, strerror(errno));
    }
    pthread_attr_destroy(&udhcpc_thread_attr);
}

void udhcpc_stop(const char *ifname) {
    ifc_init();
    ifc_remove_host_routes(ifname);
    ifc_set_addr(ifname, 0);
    ifc_close();
}
#endif
