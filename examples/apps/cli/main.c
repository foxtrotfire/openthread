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
#include <openthread/instance.h>
#include <openthread/thread.h>
#include <openthread/thread_ftd.h>
#include <openthread/coap.h>
#include <openthread/message.h>
#include "stdlib.h"
#include "string.h"
#include "em_gpio.h"

// static char nat64_prefix[] = "2018:db8:1:ffff::";
// static char coap_server_ip[] = "ac11:1"; //ipv4: 172.17.0.1
static char coap_server_address[] = "2018:db8:1:ffff::ac11:1";
static char CID = '0'; //identifies this specific device

/**@brief Enumeration describing light commands.
 */
typedef enum
{
    LIGHT_OFF = '0',
    LIGHT_ON,
    LIGHT_TOGGLE
} coap_light_command_t;

/**@brief Type definition of the function used to handle light resource change.
 */
typedef void (*light_changed_handler_t)(coap_light_command_t light_state);

/**@brief Structure holding CoAP resources.
 */
typedef struct
{
    otCoapResource light_resource;        /**< CoAP light resource. */
} coap_resources_t;


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
static void handleButtonInterrupt(otInstance *aInstance);
static void coap_put_inuse_response_handler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aError);

static void coap_get_inuse_response_handler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aError);

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

void coap_put_inuse_response_handler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aError)
{
    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aMessage);
    OT_UNUSED_VARIABLE(aMessageInfo);
    if (aError == OT_ERROR_NONE)
    {
        otCliOutputFormat("CoAP PUT Response received\r\n");
    }
    else
    {
        otCliOutputFormat("<error> CoAP response handler: failed to receive request: %d\r\n", aError);
    }
}

void coap_put_inuse_request_send(otInstance * p_instance, char * coapPayload, char * destinationAddress)
{
    otError error = OT_ERROR_NONE;
    otMessage * p_message;
    otMessageInfo message_info;
    otIp6Address coapDestinationIp;
    otCoapType coapType = OT_COAP_TYPE_CONFIRMABLE;
    otCoapCode coapCode = OT_COAP_CODE_PUT;
    char coapUri[32] = "inuse";

    do{
        
        otCliOutputFormat("<info> CoAP sent %s request with payload %s to %s\r\n", coapUri, coapPayload, destinationAddress);
        p_message = otCoapNewMessage(p_instance, NULL);
        if (p_message == NULL)
        {
            otCliOutputFormat("<error> CoAP Request: failed to allocate message\r\n");
            break;
        }
        otCoapMessageInit(p_message, coapType, coapCode);
        otCoapMessageGenerateToken(p_message, 2);
        error = otCoapMessageAppendUriPathOptions(p_message, coapUri);
        if (error != OT_ERROR_NONE)
        {
            break;
        }
        otCoapMessageSetPayloadMarker(p_message);
        error = otMessageAppend(p_message, coapPayload, sizeof(coapPayload));
        if (error != OT_ERROR_NONE)
        {
            break;
        }
        memset(&message_info, 0, sizeof(message_info));
        error = otIp6AddressFromString(destinationAddress, &coapDestinationIp);
        if (error != OT_ERROR_NONE)
        {
            break;
        }
        message_info.mPeerAddr = coapDestinationIp;
        message_info.mPeerPort = OT_DEFAULT_COAP_PORT;
        message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;

        error = otCoapSendRequest(p_instance, p_message, &message_info, coap_put_inuse_response_handler, p_instance);
    } while(false);

    if (error != OT_ERROR_NONE && p_message != NULL)
    {
        otCliOutputFormat("<error> Light MLM request: failed to send request: %d\r\n", error);
        otMessageFree(p_message);
    }
}


void coap_get_inuse_response_handler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aError)
{
    OT_UNUSED_VARIABLE(aMessageInfo);
    uint8_t response_val[16];
    char payload[4];
    payload[0] = CID;
    payload[1] = ':';
    payload[3] = 0;

    if (aError == OT_ERROR_NONE)
    {
        otMessageRead(aMessage, otMessageGetOffset(aMessage), response_val, 1);

        otCliOutputFormat("CoAP response_value: %c\r\n", response_val[0]);
        if(response_val[0] == '0')
        {
            payload[2] = '1';
            otSysLedSet(3, false);
        }
        else
        {
            payload[2] = '0';
            otSysLedSet(3, true);
        }
        coap_put_inuse_request_send(aContext, payload, coap_server_address);
    }
    else
    {
        otCliOutputFormat("<error> CoAP response handler: failed to receive request: %d\r\n", aError);
    }
}

void coap_get_inuse_request_send(otInstance * p_instance, char * coapPayload, char * destinationAddress)
{
    otError error = OT_ERROR_NONE;
    otMessage * p_message;
    otMessageInfo message_info;
    otIp6Address coapDestinationIp;
    otCoapType coapType = OT_COAP_TYPE_CONFIRMABLE;
    otCoapCode coapCode = OT_COAP_CODE_GET;
    char coapUri[32] = "inuse";

    do{
        
        otCliOutputFormat("<info> CoAP sent %s request with payload %s to %s\r\n", coapUri, coapPayload, destinationAddress);
        p_message = otCoapNewMessage(p_instance, NULL);
        if (p_message == NULL)
        {
            otCliOutputFormat("<error> CoAP Request: failed to allocate message\r\n");
            break;
        }
        otCoapMessageInit(p_message, coapType, coapCode);
        otCoapMessageGenerateToken(p_message, 2);
        error = otCoapMessageAppendUriPathOptions(p_message, coapUri);
        if (error != OT_ERROR_NONE)
        {
            break;
        }
        otCoapMessageSetPayloadMarker(p_message);
        error = otMessageAppend(p_message, coapPayload, sizeof(coapPayload));
        if (error != OT_ERROR_NONE)
        {
            break;
        }
        memset(&message_info, 0, sizeof(message_info));
        error = otIp6AddressFromString(destinationAddress, &coapDestinationIp);
        if (error != OT_ERROR_NONE)
        {
            break;
        }
        message_info.mPeerAddr = coapDestinationIp;
        message_info.mPeerPort = OT_DEFAULT_COAP_PORT;
        message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;

        error = otCoapSendRequest(p_instance, p_message, &message_info, coap_get_inuse_response_handler, p_instance);
    } while(false);

    if (error != OT_ERROR_NONE && p_message != NULL)
    {
        otCliOutputFormat("<error> CoAP Request: failed to send request: %d\r\n", error);
        otMessageFree(p_message);
    }
}





/**
 * WARNING: DO NOT PUT CLI OUTPUT CALLS IN THIS METHOD OTHER THAN ERRORS
 *              IT WILL CAUSE CRASHES
 */
void coap_light_mesh_local_multicast_request_send(otInstance * p_instance)
{
    otError error = OT_ERROR_NONE;
    otMessage * p_message;
    otMessageInfo message_info;
    otIp6Address coapDestinationIp;
    //parameters
    char coapUri[32] = "light";
    char coapPayload[1] = "2";
    otCoapType coapType = OT_COAP_TYPE_NON_CONFIRMABLE;
    otCoapCode coapCode = OT_COAP_CODE_PUT;
    char destinationAddress[] = "ff03::1";
    
    do{
        
        otCliOutputFormat("<info> CoAP sent %s request with payload %c to %s\r\n", coapUri, coapPayload[0], destinationAddress);
        p_message = otCoapNewMessage(p_instance, NULL);
        if (p_message == NULL)
        {
            otCliOutputFormat("<error> Light MLM Request: failed to allocate message\r\n");
            break;
        }
        otCoapMessageInit(p_message, coapType, coapCode);
        otCoapMessageGenerateToken(p_message, 2);
        error = otCoapMessageAppendUriPathOptions(p_message, coapUri);
        if (error != OT_ERROR_NONE)
        {
            break;
        }
        otCoapMessageSetPayloadMarker(p_message);
        error = otMessageAppend(p_message, coapPayload, sizeof(coapPayload));
        if (error != OT_ERROR_NONE)
        {
            break;
        }
        memset(&message_info, 0, sizeof(message_info));
        error = otIp6AddressFromString(destinationAddress, &coapDestinationIp);
        if (error != OT_ERROR_NONE)
        {
            break;
        }
        message_info.mPeerAddr = coapDestinationIp;
        message_info.mPeerPort = OT_DEFAULT_COAP_PORT;
        message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;

        error = otCoapSendRequest(p_instance, p_message, &message_info, NULL, NULL);
    } while(false);

    if (error != OT_ERROR_NONE && p_message != NULL)
    {
        otCliOutputFormat("<error> Light MLM request: failed to send request: %d\r\n", error);
        otMessageFree(p_message);
    }
}

void handleButtonInterrupt(otInstance *aInstance)
{
    GPIO_IntClear(1<<7U);
    GPIO_IntDisable(1<<7U);
    coap_get_inuse_request_send(aInstance, "0", coap_server_address);
    delayMs(10);
    GPIO_IntEnable(1<<7U);
}

static void light_changed_default(coap_light_command_t light_command)
{
    otCliOutputFormat("<info> Light changed: %s\r\n", light_command);
    switch (light_command)
    {
        case LIGHT_ON:
            otSysLedSet(3, true);
            break;

        case LIGHT_OFF:
            otSysLedSet(3, false);
            break;

        case LIGHT_TOGGLE:
            otSysLedToggle(3);
            break;

        default:
            otCliOutputFormat("<error> Light changed: unknown command\r\n");
            break;
    }
}

static light_changed_handler_t m_light_changed = light_changed_default;


void coap_light_response_send(void                * p_context,
                                           otMessage * p_request_message,
                                           const otMessageInfo * p_message_info)
{
    otError      error = OT_ERROR_NO_BUFS;
    otMessage  * p_response;
    do
    {
        otCliOutputFormat("<info> Light response called\r\n");
        
        p_response = otCoapNewMessage(p_context, NULL);
        otCoapMessageInit(p_response, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CHANGED);
        otCoapMessageSetMessageId(p_response, otCoapMessageGetMessageId(p_request_message));
        otCoapMessageSetToken(p_response,
                             otCoapMessageGetToken(p_request_message),
                             otCoapMessageGetTokenLength(p_request_message));

        if (p_response == NULL)
        {
            break;
        }

        error = otCoapSendResponse(p_context, p_response, p_message_info);


    } while (false);

    if ((error != OT_ERROR_NONE) && (p_response != NULL))
    {
        otMessageFree(p_response);
    }
}

static void coap_light_request_handler(void                * p_context,
                                                    otMessage           * p_message,
                                                    const otMessageInfo * p_message_info)
{
    uint8_t command;
    do
    {

        otCliOutputFormat("<info> Light request received\r\n");
        if (otCoapMessageGetType(p_message) != OT_COAP_TYPE_CONFIRMABLE &&
            otCoapMessageGetType(p_message) != OT_COAP_TYPE_NON_CONFIRMABLE)
        {
            break;
        }

        if (otCoapMessageGetCode(p_message) != OT_COAP_CODE_PUT)
        {
            break;
        }

        if (otMessageRead(p_message, otMessageGetOffset(p_message), &command, 1) != 1)
        {
            otCliOutputFormat("<error> Light request: missing command\r\n");
        }

        m_light_changed((coap_light_command_t)command);

        if (otCoapMessageGetType(p_message) == OT_COAP_TYPE_CONFIRMABLE)
        {
            coap_light_response_send(p_context, p_message, p_message_info);
        }

    } while (false);
}


static coap_resources_t m_coap_resources = {
    .light_resource        = {"light", coap_light_request_handler, NULL, NULL},
};
/**
 * /Altered
 */

int main(int argc, char *argv[])
{
    otInstance *instance;

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
#else
    instance = otInstanceInitSingle();
#endif
    assert(instance);

    otCliUartInit(instance);
/**
 * Altered
 */
    /* Register Thread state change handler */
    otSetStateChangedCallback(instance, handleNetifStateChanged, instance);
    /* init GPIO LEDs */
    otSysLedInit();
    /* init GPIO BTN0 */
    otSysButtonInit(handleButtonInterrupt);
    /* Init CoAP */
    otCoapStart(instance, OT_DEFAULT_COAP_PORT);
    m_coap_resources.light_resource.mContext = instance;
    otCoapAddResource(instance, &m_coap_resources.light_resource);

    otCliOutputFormat("<info> Initialized\r\n");
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
/**
 * Altered
 */
        otSysButtonProcess(instance);
/**
 * /Altered
 */        
    }

    otInstanceFinalize(instance);
#if OPENTHREAD_ENABLE_MULTIPLE_INSTANCES
    free(otInstanceBuffer);
#endif

    goto pseudo_reset;

    return 0;
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
