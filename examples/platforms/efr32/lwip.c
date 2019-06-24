

#include "stdio.h"

#include "em_assert.h"
#include "em_cmu.h"
#include "em_device.h"
#include <stdlib.h>
#include <string.h>
#include "lwip/api.h"
#include "lwip/autoip.h"
#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/netifapi.h"
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"
#include "lwip/tcp_impl.h"
#include "lwip/udp.h"


#define IPADDR_USE_STATIC 0
#define IPADDR_USE_DHCP 1
#define IPADDR_USE_AUTOIP 2

#define IPADDR(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | d)

extern void httpd_init(void);

uint32_t INT_LockCnt = 0;

uint32_t printAddr;

/**************************************************************************/ /**
                                                                              * The lwIP network interface structure for
                                                                              *the KSZ8851SNL Ethernet MAC.
                                                                              *****************************************************************************/
struct netif lwip_netif;

/**************************************************************************/ /**
                                                                              * The local time for the lwIP Library
                                                                              *Abstraction layer, used to support the
                                                                              * Host and lwIP periodic callback
                                                                              *functions.
                                                                              *****************************************************************************/


/**************************************************************************/ /**
                                                                              * The local time when the HOST timer was
                                                                              *last serviced.
                                                                              *****************************************************************************/
#if HOST_TMR_INTERVAL
static unsigned long g_ulHostTimer = 0;
#endif

/**************************************************************************/ /**
                                                                              * The local time when the ARP timer was
                                                                              *last serviced.
                                                                              *****************************************************************************/
#if LWIP_ARP
static unsigned long g_ulARPTimer = 0;
#endif

/**************************************************************************/ /**
                                                                              * The local time when the AutoIP timer was
                                                                              *last serviced.
                                                                              *****************************************************************************/
#if LWIP_AUTOIP
static unsigned long g_ulAutoIPTimer = 0;
#endif

/**************************************************************************/ /**
                                                                              *The local time when the DHCP Coarse timer
                                                                              *was last serviced.
                                                                              *****************************************************************************/
#if LWIP_DHCP
static unsigned long g_ulDHCPCoarseTimer = 0;
#endif

/**************************************************************************/ /**
                                                                              *The local time when the DHCP Fine timer
                                                                              *was last serviced.
                                                                              *****************************************************************************/
#if LWIP_DHCP
static unsigned long g_ulDHCPFineTimer = 0;
#endif

/**************************************************************************/ /**
                                                                              * @brief SysTick_Handler
                                                                              * Interrupt Service Routine for system
                                                                              *tick counter.
                                                                              *****************************************************************************/



    /* Service the ARP timer.*/
#if LWIP_ARP
    if ((g_ulLocalTimer - g_ulARPTimer) >= ARP_TMR_INTERVAL)
    {
        g_ulARPTimer = g_ulLocalTimer;
        etharp_tmr();
    }
#endif

    /* Service the AutoIP timer.*/
#if LWIP_AUTOIP
    if ((g_ulLocalTimer - g_ulAutoIPTimer) >= AUTOIP_TMR_INTERVAL)
    {
        g_ulAutoIPTimer = g_ulLocalTimer;
        autoip_tmr();
    }
#endif

    /* Service the DCHP Coarse Timer. */
#if LWIP_DHCP
    if ((g_ulLocalTimer - g_ulDHCPCoarseTimer) >= DHCP_COARSE_TIMER_MSECS)
    {
        g_ulDHCPCoarseTimer = g_ulLocalTimer;
        dhcp_coarse_tmr();
    }
#endif

    /* Service the DCHP Fine Timer.*/
#if LWIP_DHCP
    if ((g_ulLocalTimer - g_ulDHCPFineTimer) >= DHCP_FINE_TIMER_MSECS)
    {
        g_ulDHCPFineTimer = g_ulLocalTimer;
        dhcp_fine_tmr();
    }
#endif


void lwIPInit(const unsigned char *pucMAC,
              unsigned long        ulIPAddr,
              unsigned long        ulNetMask,
              unsigned long        ulGWAddr,
              unsigned long        ulIPMode)
{
    struct ip_addr ip_addr;
    struct ip_addr net_mask;
    struct ip_addr gw_addr;

/* Check the parameters.*/
#if LWIP_DHCP && LWIP_AUTOIP
    EFM_ASSERT((ulIPMode == IPADDR_USE_STATIC) || (ulIPMode == IPADDR_USE_DHCP) || (ulIPMode == IPADDR_USE_AUTOIP));
#elif LWIP_DHCP
    EFM_ASSERT((ulIPMode == IPADDR_USE_STATIC) || (ulIPMode == IPADDR_USE_DHCP));
#elif LWIP_AUTOIP
    EFM_ASSERT((ulIPMode == IPADDR_USE_STATIC) || (ulIPMode == IPADDR_USE_AUTOIP));
#else
    EFM_ASSERT(ulIPMode == IPADDR_USE_STATIC);
#endif

    /* Initialize lwIP library modules */
    lwip_init();

    /* Setup the network address values.*/
    if (ulIPMode == IPADDR_USE_STATIC)
    {
        ip_addr.addr  = htonl(ulIPAddr);
        net_mask.addr = htonl(ulNetMask);
        gw_addr.addr  = htonl(ulGWAddr);
    }
#if LWIP_DHCP || LWIP_AUTOIP
    else
    {
        ip_addr.addr  = 0;
        net_mask.addr = 0;
        gw_addr.addr  = 0;
    }
#endif

    /* Create, configure and add the Ethernet controller interface with
     * default settings.*/
    netif_add(&lwip_netif, &ip_addr, &net_mask, &gw_addr, NULL, *ksz8851snl_driver_init, ip_input);

    netif_set_default(&lwip_netif);

    /* Start DHCP, if enabled.*/
#if LWIP_DHCP
    if (ulIPMode == IPADDR_USE_DHCP)
    {
        dhcp_start(&lwip_netif);
    }
#endif

    /* Start AutoIP, if enabled and DHCP is not.*/
#if LWIP_AUTOIP
    if (ulIPMode == IPADDR_USE_AUTOfIP)
    {
        autoip_start(&lwip_netif);
    }
#endif

    /* Bring the interface up.*/
    netif_set_up(&lwip_netif);
}

/**************************************************************************/ /**
                                                                              * @brief  Main function
                                                                              *****************************************************************************/


void otSysLwipInit()
{
    unsigned char pucMACArray[8];

    /* Initialze the lwIP library, using Static IP.*/
    lwIPInit(pucMACArray, IPADDR(192, 168, 80, 90), IPADDR(255, 255, 255, 0), IPADDR(192, 168, 80, 1),
             IPADDR_USE_STATIC);


    /* Initialize a sample httpd server.*/
    httpd_init();
};
