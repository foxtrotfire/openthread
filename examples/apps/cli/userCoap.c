
#include <assert.h>
#include <openthread-core-config.h>
#include <openthread/config.h>
#include <openthread/cli.h>
#include <openthread/instance.h>
#include <openthread/thread.h>
#include <openthread/thread_ftd.h>
#include <openthread/coap.h>
#include <openthread/message.h>
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "userCoap.h"

char coap_server_address[] = "2018:db8:1:ffff::ac11:1";
char CID                   = '1'; // identifies this specific device


void coap_print_payload(otMessage *aMessage){
    uint8_t buf[16];
    uint16_t bytesToPrint;
    uint16_t bytesPrinted = 0;
    uint16_t length = otMessageGetLength(aMessage) - otMessageGetOffset(aMessage);

    if (length > 0)
    {
        while (length > 0)
        {
            bytesToPrint = (length < sizeof(buf)) ? length : sizeof(buf);
            otMessageRead(aMessage, otMessageGetOffset(aMessage) + bytesPrinted, buf, bytesToPrint);
            otCliOutput((const char *)buf, bytesToPrint);

            length -= bytesToPrint;
            bytesPrinted += bytesToPrint;
        }
    }
    otCliOutputFormat("\r\n");
}

void coap_print_payload_raw(otMessage *aMessage){
    uint8_t buf[16];
    uint16_t bytesToPrint;
    uint16_t bytesPrinted = 0;
    uint16_t length = otMessageGetLength(aMessage) - otMessageGetOffset(aMessage);

    if (length > 0)
    {
        otCliOutputFormat("Payload(raw): ");
        while (length > 0)
        {
            bytesToPrint = (length < sizeof(buf)) ? length : sizeof(buf);
            otMessageRead(aMessage, otMessageGetOffset(aMessage) + bytesPrinted, buf, bytesToPrint);
            otCliOutputBytes(buf, (uint8_t)bytesToPrint);

            length -= bytesToPrint;
            bytesPrinted += bytesToPrint;
        }
    }
    otCliOutputFormat("\r\n");
}

void coap_put_login_response_handler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aError)
{
    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aMessageInfo);

    if (aError == OT_ERROR_NONE)
    {
        coap_print_payload(aMessage);
        char payload[2];
        payload[0] = CID;
        payload[1] = '\0';
        coap_get_inuse_request_send(aContext, payload, coap_server_address);
    }
    else
    {
        otCliOutputFormat("<error> CoAP response handler: failed to receive request: %d\r\n", aError);
    }
}

void coap_put_login_request_send(otInstance * p_instance, char * coapPayload, char * destinationAddress)
{
    otError error = OT_ERROR_NONE;
    otMessage * p_message;
    otMessageInfo message_info;
    otIp6Address coapDestinationIp;
    otCoapType coapType = OT_COAP_TYPE_CONFIRMABLE;
    otCoapCode coapCode = OT_COAP_CODE_PUT;
    char coapUri[32] = "login";

    do{
        
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

        error = otCoapSendRequest(p_instance, p_message, &message_info, coap_put_login_response_handler, p_instance);
    } while(false);

    if (error != OT_ERROR_NONE && p_message != NULL)
    {
        otCliOutputFormat("<error> Login request: failed to send request: %d\r\n", error);
        otMessageFree(p_message);
    }
}

void coap_put_logout_response_handler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aError)
{
    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aMessageInfo);

    if (aError == OT_ERROR_NONE)
    {
        coap_print_payload(aMessage);
        char payload[2];
        payload[0] = CID;
        payload[1] = '\0';
        coap_get_inuse_request_send(aContext, payload, coap_server_address);
    }
    else
    {
        otCliOutputFormat("<error> CoAP response handler: failed to receive request: %d\r\n", aError);
    }
}

void coap_put_logout_request_send(otInstance * p_instance, char * coapPayload, char * destinationAddress)
{
    otError error = OT_ERROR_NONE;
    otMessage * p_message;
    otMessageInfo message_info;
    otIp6Address coapDestinationIp;
    otCoapType coapType = OT_COAP_TYPE_CONFIRMABLE;
    otCoapCode coapCode = OT_COAP_CODE_PUT;
    char coapUri[32] = "logout";

    do{
        
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

        error = otCoapSendRequest(p_instance, p_message, &message_info, coap_put_logout_response_handler, p_instance);
    } while(false);

    if (error != OT_ERROR_NONE && p_message != NULL)
    {
        otCliOutputFormat("<error> Login request: failed to send request: %d\r\n", error);
        otMessageFree(p_message);
    }
}

void coap_get_inuse_response_handler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aError)
{
    OT_UNUSED_VARIABLE(aMessageInfo);
    OT_UNUSED_VARIABLE(aContext);
    uint8_t response_val[256];

    if (aError == OT_ERROR_NONE)
    {
        otMessageRead(aMessage, otMessageGetOffset(aMessage), response_val, 1);

        coap_print_payload(aMessage);
        if(response_val[0] == '0')
        {
            otSysLedSet(3, false);
        }
        else
        {
            otSysLedSet(3, true);
        }
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

void coap_get_charging_response_handler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aError)
{
    OT_UNUSED_VARIABLE(aMessageInfo);
    OT_UNUSED_VARIABLE(aContext);
    uint8_t response_val[256];
    if (aError == OT_ERROR_NONE)
    {
        otMessageRead(aMessage, otMessageGetOffset(aMessage), response_val, 1);

        coap_print_payload(aMessage);
        if(response_val[0] == '0')
        {
            otSysLedSet(0, false);
        }
        else
        {
            otSysLedSet(0, true);
        }
    }
    else
    {
        otCliOutputFormat("<error> CoAP response handler: failed to receive response: %d\r\n", aError);
    }
}

void coap_get_charging_request_send(otInstance * p_instance, char * coapPayload, char * destinationAddress)
{
    otError error = OT_ERROR_NONE;
    otMessage * p_message;
    otMessageInfo message_info;
    otIp6Address coapDestinationIp;
    otCoapType coapType = OT_COAP_TYPE_CONFIRMABLE;
    otCoapCode coapCode = OT_COAP_CODE_GET;
    char coapUri[32] = "charging";

    do{
        
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

        error = otCoapSendRequest(p_instance, p_message, &message_info, coap_get_charging_response_handler, p_instance);
    } while(false);

    if (error != OT_ERROR_NONE && p_message != NULL)
    {
        otCliOutputFormat("<error> CoAP Request: failed to send request: %d\r\n", error);
        otMessageFree(p_message);
    }
}

void coap_put_charging_response_handler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aError)
{
    OT_UNUSED_VARIABLE(aMessageInfo);
    OT_UNUSED_VARIABLE(aContext);

    if (aError == OT_ERROR_NONE)
    {
        coap_print_payload(aMessage);
        char payload[2];
        payload[0] = CID;
        payload[1] = '\0';
        coap_get_charging_request_send(aContext, payload, coap_server_address);
    }
    else
    {
        otCliOutputFormat("<error> CoAP response handler: failed to receive response: %d\r\n", aError);
    }
}

void coap_put_charging_request_send(otInstance * p_instance, char * coapPayload, char * destinationAddress)
{
    otError error = OT_ERROR_NONE;
    otMessage * p_message;
    otMessageInfo message_info;
    otIp6Address coapDestinationIp;
    otCoapType coapType = OT_COAP_TYPE_CONFIRMABLE;
    otCoapCode coapCode = OT_COAP_CODE_PUT;
    char coapUri[32] = "charging";

    do{
        
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

        error = otCoapSendRequest(p_instance, p_message, &message_info, coap_put_charging_response_handler, p_instance);
    } while(false);

    if (error != OT_ERROR_NONE && p_message != NULL)
    {
        otCliOutputFormat("<error> CoAP Request: failed to send request: %d\r\n", error);
        otMessageFree(p_message);
    }
}

void coap_get_chargecurrent_response_handler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aError)
{
    OT_UNUSED_VARIABLE(aMessageInfo);
    OT_UNUSED_VARIABLE(aContext);

    if (aError == OT_ERROR_NONE)
    {
        coap_print_payload(aMessage);
    }
    else
    {
        otCliOutputFormat("<error> CoAP response handler: failed to receive response: %d\r\n", aError);
    }
}

void coap_get_chargecurrent_request_send(otInstance * p_instance, char * coapPayload, char * destinationAddress)
{
    otError error = OT_ERROR_NONE;
    otMessage * p_message;
    otMessageInfo message_info;
    otIp6Address coapDestinationIp;
    otCoapType coapType = OT_COAP_TYPE_CONFIRMABLE;
    otCoapCode coapCode = OT_COAP_CODE_GET;
    char coapUri[32] = "chargecurrent";

    do{
        
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

        error = otCoapSendRequest(p_instance, p_message, &message_info, coap_get_chargecurrent_response_handler, p_instance);
    } while(false);

    if (error != OT_ERROR_NONE && p_message != NULL)
    {
        otCliOutputFormat("<error> CoAP Request: failed to send request: %d\r\n", error);
        otMessageFree(p_message);
    }
}


void coap_test_response_handler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aError)
{
    OT_UNUSED_VARIABLE(aMessageInfo);
    OT_UNUSED_VARIABLE(aContext);
    if (aError == OT_ERROR_NONE)
    {
        coap_print_payload(aMessage);
    }
    else
    {
        otCliOutputFormat("<error> CoAP response handler: failed to receive response: %d\r\n", aError);
    }
}

void coap_test_mesh_local_multicast_request_send(otInstance * p_instance)
{
    otError error = OT_ERROR_NONE;
    otMessage * p_message;
    otMessageInfo message_info;
    otIp6Address coapDestinationIp;
    //parameters
    char coapUri[32] = "test";
    char coapPayload[1] = "2";
    otCoapType coapType = OT_COAP_TYPE_CONFIRMABLE;
    otCoapCode coapCode = OT_COAP_CODE_PUT;
    char destinationAddress[] = "ff03::1";
    
    do{
        p_message = otCoapNewMessage(p_instance, NULL);
        if (p_message == NULL)
        {
            otCliOutputFormat("<error> test MLM Request: failed to allocate message\r\n");
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

        error = otCoapSendRequest(p_instance, p_message, &message_info, coap_test_response_handler, p_instance);
    } while(false);

    if (error != OT_ERROR_NONE && p_message != NULL)
    {
        otCliOutputFormat("<error> Test MLM request: failed to send request: %d\r\n", error);
        otMessageFree(p_message);
    }
}



void coap_test_response_send(void                * p_context,
                                           otMessage * p_request_message,
                                           const otMessageInfo * p_message_info)
{
    otError      error = OT_ERROR_NO_BUFS;
    otError result;
    otMessage  * p_response;
    do
    {
        
        p_response = otCoapNewMessage(p_context, NULL);
            otCoapMessageInit(p_response, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CHANGED);
            otCoapMessageSetMessageId(p_response, otCoapMessageGetMessageId(p_request_message));
            otCoapMessageSetToken(p_response,
                                otCoapMessageGetToken(p_request_message),
                                otCoapMessageGetTokenLength(p_request_message));

            otCoapMessageSetPayloadMarker(p_response);
            char payload[24];
            snprintf(payload, sizeof(payload), "Test response from CID%c", CID);
            result = otMessageAppend(p_response, payload, sizeof(payload));
            if (result == OT_ERROR_NONE){
                if (p_response == NULL)
                {
                    break;
                }

                error = otCoapSendResponse(p_context, p_response, p_message_info);
            }
            else{
                otCliOutputFormat("<error> Error appending response\r\n");
            }


    } while (false);

    if ((error != OT_ERROR_NONE) && (p_response != NULL))
    {
        otMessageFree(p_response);
    }
}

void coap_test_request_handler(void                * p_context,
                                                    otMessage           * p_message,
                                                    const otMessageInfo * p_message_info)
{
    do
    {

        if (otCoapMessageGetType(p_message) != OT_COAP_TYPE_CONFIRMABLE &&
            otCoapMessageGetType(p_message) != OT_COAP_TYPE_NON_CONFIRMABLE)
        {
            break;
        }

        if (otCoapMessageGetCode(p_message) != OT_COAP_CODE_PUT)
        {
            break;
        }

        if (otCoapMessageGetType(p_message) == OT_COAP_TYPE_CONFIRMABLE)
        {
            coap_test_response_send(p_context, p_message, p_message_info);
        }

    } while (false);
}

void coap_default_response_handler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    OT_UNUSED_VARIABLE(aMessageInfo);
    OT_UNUSED_VARIABLE(aContext);
    coap_print_payload(aMessage);
}

coap_resources_t m_coap_resources = {
    .test_resource        = {"test", coap_test_request_handler, NULL, NULL},
};