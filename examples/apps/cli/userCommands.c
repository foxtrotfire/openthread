
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
#include "userCommands.h"
#include "userCoap.h"

void cli_userCommands_init(otInstance *aInstance){
    thisInstance = aInstance;
}

void cli_hello_world(int argc, char *argv[]){
    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);
    otCliOutputFormat("Hello World!\r\n");
}

void cli_login(int argc, char *argv[]){
    char payload[19];
    if(argc == 1){
        payload[18] = 0;
        payload[1] = ':';
        payload[0] = CID;
        for(int i = 2; i < 18; i++){
            payload[i] = argv[0][i-2];
        }
        coap_put_login_request_send(thisInstance, payload, coap_server_address);
    }
    else{
        otCliOutputFormat("Invalid argument.\r\n Use login [16 char UID]\r\n");
    }
}

void cli_logout(int argc, char *argv[]){
    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);
    char payload[2];
    payload[0] = CID;
    payload[1] = 0;
    coap_put_logout_request_send(thisInstance, payload, coap_server_address);
}

void cli_charge(int argc, char *argv[]){
    char payload[4];
    if(argc == 1){
        if(strcmp(argv[0], "start") == 0){
            payload[0] = CID;
            payload[1] = ':';
            payload[2] = '1';
            payload[3] = 0;
            coap_put_charging_request_send(thisInstance, payload, coap_server_address);
        }
        else if(strcmp(argv[0], "stop") == 0){
            payload[0] = CID;
            payload[1] = ':';
            payload[2] = '0';
            payload[3] = 0;
            coap_put_charging_request_send(thisInstance, payload, coap_server_address);
        }
        else if(strcmp(argv[0], "current") == 0){
            payload[0] = CID;
            payload[1] = 0;
            coap_get_chargecurrent_request_send(thisInstance, payload, coap_server_address);
        }
        else if(strcmp(argv[0], "state") == 0){
            payload[0] = CID;
            payload[1] = 0;
            coap_get_charging_request_send(thisInstance, payload, coap_server_address);
        }
        else{
            otCliOutputFormat("Invalid argument.\r\n Use charge start, stop, state or current\r\n");
        }
    }
    else{
        otCliOutputFormat("Invalid argument.\r\n Use charge start, stop, state or current\r\n");
    }
}

otCliCommand userCommands[] = { 
    { .mName = "login", .mCommand = &cli_login},
    { .mName = "logout", .mCommand = &cli_logout},
    { .mName = "charge", .mCommand = &cli_charge},
    { .mName = "hello", .mCommand = &cli_hello_world}
};
