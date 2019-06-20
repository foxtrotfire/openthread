#include <stdint.h>

#include "openthread-system.h"
#include <openthread/ip6.h>
#include <openthread/message.h>

#define COAP_SERVER_ADDRESS_MAX_SIZE 39
// static char nat64_prefix[] = "2018:db8:1:ffff::";
// static char coap_server_ip[] = "ac11:1"; //ipv4: 172.17.0.1
extern char coap_server_address[COAP_SERVER_ADDRESS_MAX_SIZE];
extern char CID; // identifies this specific device


/**@brief Structure holding CoAP resources.
 */
typedef struct
{
    otCoapResource test_resource; /**< CoAP test resource. */
} coap_resources_t;

void coap_print_payload(otMessage *aMessage);
void coap_print_payload_raw(otMessage *aMessage);

void coap_put_login_response_handler(void *               aContext,
                                     otMessage *          aMessage,
                                     const otMessageInfo *aMessageInfo,
                                     otError              aError);

void coap_put_login_request_send(otInstance *p_instance, char *coapPayload, char *destinationAddress);
void coap_put_logout_response_handler(void *               aContext,
                                     otMessage *          aMessage,
                                     const otMessageInfo *aMessageInfo,
                                     otError              aError);

void coap_put_logout_request_send(otInstance *p_instance, char *coapPayload, char *destinationAddress);

void coap_get_inuse_response_handler(void *               aContext,
                                     otMessage *          aMessage,
                                     const otMessageInfo *aMessageInfo,
                                     otError              aError);

void coap_get_inuse_request_send(otInstance *p_instance, char *coapPayload, char *destinationAddress);

void coap_get_charging_response_handler(void *               aContext,
                                     otMessage *          aMessage,
                                     const otMessageInfo *aMessageInfo,
                                     otError              aError);

void coap_get_charging_request_send(otInstance *p_instance, char *coapPayload, char *destinationAddress);
void coap_put_charging_response_handler(void *               aContext,
                                     otMessage *          aMessage,
                                     const otMessageInfo *aMessageInfo,
                                     otError              aError);

void coap_put_charging_request_send(otInstance *p_instance, char *coapPayload, char *destinationAddress);
void coap_get_chargecurrent_response_handler(void *               aContext,
                                     otMessage *          aMessage,
                                     const otMessageInfo *aMessageInfo,
                                     otError              aError);

void coap_get_chargecurrent_request_send(otInstance *p_instance, char *coapPayload, char *destinationAddress);

void coap_test_response_handler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aError);

void coap_test_mesh_local_multicast_request_send(otInstance *p_instance);

void coap_test_response_send(void *p_context, otMessage *p_request_message, const otMessageInfo *p_message_info);

void coap_test_request_handler(void *p_context, otMessage *p_message, const otMessageInfo *p_message_info);

void coap_default_response_handler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);

extern coap_resources_t m_coap_resources;