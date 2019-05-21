#include <stdint.h>

#include "openthread-system.h"
#include <openthread/ip6.h>
#include <openthread/message.h>

#define COAP_SERVER_ADDRESS_MAX_SIZE 39
// static char nat64_prefix[] = "2018:db8:1:ffff::";
// static char coap_server_ip[] = "ac11:1"; //ipv4: 172.17.0.1
extern char coap_server_address[COAP_SERVER_ADDRESS_MAX_SIZE];
extern char CID; // identifies this specific device

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

light_changed_handler_t m_light_changed;

/**@brief Structure holding CoAP resources.
 */
typedef struct
{
    otCoapResource light_resource; /**< CoAP light resource. */
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

void coap_light_mesh_local_multicast_request_send(otInstance *p_instance);

void light_changed_default(coap_light_command_t light_command);

void coap_light_response_send(void *p_context, otMessage *p_request_message, const otMessageInfo *p_message_info);

void coap_light_request_handler(void *p_context, otMessage *p_message, const otMessageInfo *p_message_info);

extern coap_resources_t m_coap_resources;