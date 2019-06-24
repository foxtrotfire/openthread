/*
 *  Copyright (c) 2016, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <openthread-core-config.h>
#include <openthread/config.h>

#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>
#include <openthread/platform/logging.h>

#include "openthread-system.h"

/**
 * Altered
 */
#include "em_gpio.h"
#include "stdlib.h"
#include "string.h"
#include <openthread/coap.h>
#include <openthread/instance.h>
#include <openthread/message.h>
#include <openthread/thread.h>
#include <openthread/thread_ftd.h>
#include <openthread/coap.h>
#include <openthread/message.h>
#include "stdlib.h"
#include "string.h"
#include "em_gpio.h"
#include "em_cmu.h"

#include "userCoap.h"
#include "userCommands.h"

#define PORTIO_GPIO_AT86RST_PIN     (6U)
#define PORTIO_GPIO_AT86RST_PORT    (gpioPortF)
#define PORTIO_GPIO_AT86RST2_PIN     (7U)
#define PORTIO_GPIO_AT86RST2_PORT    (gpioPortF)

void OTCALL handleNetifStateChanged(uint32_t aFlags, void *aContext);

/**
 * /Altered
 */

#if OPENTHREAD_EXAMPLES_POSIX
#include <setjmp.h>
#include <unistd.h>

jmp_buf gResetJump;

void __gcov_flush();
#endif

#if OPENTHREAD_ENABLE_MULTIPLE_INSTANCES
void *otPlatCAlloc(size_t aNum, size_t aSize)
{
    return calloc(aNum, aSize);
}

void otPlatFree(void *aPtr)
{
    free(aPtr);
}
#endif

void otTaskletsSignalPending(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
}

/**
 * Altered
 */


void OTCALL handleNetifStateChanged(uint32_t aFlags, void *aContext)
{
    if ((aFlags & OT_CHANGED_THREAD_ROLE) != 0)
    {
        otDeviceRole changedRole = otThreadGetDeviceRole(aContext);

        switch (changedRole)
        {
        case OT_DEVICE_ROLE_LEADER:
            otSysLedSet(0, true);
            otSysLedSet(1, true);
            otSysLedSet(2, true);
            break;

        case OT_DEVICE_ROLE_ROUTER:
            otSysLedSet(0, true);
            otSysLedSet(1, false);
            otSysLedSet(2, true);
            break;

        case OT_DEVICE_ROLE_CHILD:
            otSysLedSet(0, true);
            otSysLedSet(1, true);
            otSysLedSet(2, false);
            break;

        case OT_DEVICE_ROLE_DETACHED:
            otSysLedSet(0, true);
            otSysLedSet(1, false);
            otSysLedSet(2, false);
            break;

        case OT_DEVICE_ROLE_DISABLED:
            otSysLedSet(0, false);
            otSysLedSet(1, false);
            otSysLedSet(2, false);
            break;
        }
    }
}



/**
 * /Altered
 */

int main(int argc, char *argv[])
{
    otInstance *instance;
#if OPENTHREAD_ENABLE_MULTIPLE_INSTANCES
    otInstance *instance2;
#endif

#if OPENTHREAD_EXAMPLES_POSIX
    if (setjmp(gResetJump))
    {
        alarm(0);
#if OPENTHREAD_ENABLE_COVERAGE
        __gcov_flush();
#endif
        execvp(argv[0], argv);
    }
#endif

#if OPENTHREAD_ENABLE_MULTIPLE_INSTANCES
    size_t   otInstanceBufferLength = 0;
    uint8_t *otInstanceBuffer       = NULL;
#endif

pseudo_reset:

    otSysInit(argc, argv);

#if OPENTHREAD_ENABLE_MULTIPLE_INSTANCES
    // Call to query the buffer size
    (void)otInstanceInit(NULL, &otInstanceBufferLength);

    // Call to allocate the buffer
    otInstanceBuffer = (uint8_t *)malloc(otInstanceBufferLength);
    assert(otInstanceBuffer);

    // Initialize OpenThread with the buffer
    instance = otInstanceInit(otInstanceBuffer, &otInstanceBufferLength);
    instance2 = otInstanceInit(otInstanceBuffer, &otInstanceBufferLength);
#else
    instance = otInstanceInitSingle();
#endif
    assert(instance);
#if OPENTHREAD_ENABLE_MULTIPLE_INSTANCES
    assert(instance2);
#endif

   otCliUartInit(instance);
#if OPENTHREAD_ENABLE_MULTIPLE_INSTANCES
    otCliUartInit(instance2);
#endif
    otCliSetUserCommands(userCommands, 5);
    cli_userCommands_init(instance);
    /* Register Thread state change handler */
    //    otSetStateChangedCallback(instance, handleNetifStateChanged, instance);
    /* init GPIO LEDs */
    otSysLedInit();
    /* init Ethernet */
      otSysEthernetInit();
    /* LwIP does not compile yet so it's disabled and removed from Makefile.am */
    //   otSysLwipInit(); 
    /* init Subg Radio */
      otSysSubgRadioInit();
    /* Init CoAP */    
    otCoapSetDefaultHandler(instance, coap_default_response_handler, NULL);    
    otCoapAddResource(instance, &m_coap_resources.test_resource);    
    m_coap_resources.test_resource.mContext = instance;
    otCoapStart(instance, OT_DEFAULT_COAP_PORT);

    otCliOutputFormat("<info> Initialized\r\n");

    /* perform subg radio test once */
    //   otSysSubgRadioTest();

/**
 * /Altered
 */
#if OPENTHREAD_ENABLE_DIAG
    otDiagInit(instance);
#endif


    while (!otSysPseudoResetWasRequested())
    {
        otTaskletsProcess(instance);
        otSysProcessDrivers(instance);
        
#if OPENTHREAD_ENABLE_MULTIPLE_INSTANCES
        otTaskletsProcess(instance2);
        otSysProcessDrivers(instance2);
#endif
    }

    otInstanceFinalize(instance);
#if OPENTHREAD_ENABLE_MULTIPLE_INSTANCES
    otInstanceFinalize(instance2);
    free(otInstanceBuffer);
#endif

    goto pseudo_reset;

    return 0;
    while(true){

    }
}

/*
 * Provide, if required an "otPlatLog()" function
 */
#if OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_APP
void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, ...)
{
    OT_UNUSED_VARIABLE(aLogLevel);
    OT_UNUSED_VARIABLE(aLogRegion);
    OT_UNUSED_VARIABLE(aFormat);

    va_list ap;
    va_start(ap, aFormat);
    otCliPlatLogv(aLogLevel, aLogRegion, aFormat, ap);
    va_end(ap);
}
#endif
