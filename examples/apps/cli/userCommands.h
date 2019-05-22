#include <stdarg.h>
#include <stdint.h>
#include <openthread/instance.h>
#include <openthread/cli.h>

otInstance *thisInstance;

void cli_userCommands_init(otInstance *aInstance);
void cli_hello_world(int argc, char *argv[]);
void cli_login(int argc, char *argv[]);
void cli_charge(int argc, char *argv[]);

extern otCliCommand userCommands[];