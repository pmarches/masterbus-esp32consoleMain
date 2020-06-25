#include <masterbusConsole.h>
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "esp_console.h"
#include <memory.h>
#include <mvParser.hpp>

#include <freertos/task.h>

extern int hex2int(char input);

static struct {
	struct arg_int *canId;
	struct arg_int *attributeId;
	struct arg_end *end;
} mvrrArgs;

static int mastervoltRequestResponse(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &mvrrArgs);
    if (nerrors != 0) {
        arg_print_errors(stderr, mvrrArgs.end, argv[0]);
        return 1;
    }

    long int canIdRequest=0x10000000|mvrrArgs.canId->ival[0];
	printf("mvrrArgs.canId=0x%lx\n", canIdRequest);
	uint8_t attributeId=mvrrArgs.attributeId->ival[0];
	uint8_t requestBytes[]={attributeId, 0};

	/**
	 * The Request response mechanism does not work like that. :-(
	 * I think the canId plays a bigger role than I tought.
	 * After any kind of broadcast, I get a response for "08 3f"
	 */
//	vTaskDelay(50);
	g_mbctl->send(canIdRequest, requestBytes, 2);
	while(true){
		CANBusPacket packet;
		if(g_mbctl[0].readPacket(packet)){
			if(true || packet.canId==mvrrArgs.canId->ival[0]){
				MvParser p;
				std::string str=packet.getData();
				MastervoltMessage* msg=p.parse(packet.canId, str);
				if(true || attributeId==msg->attributeId){
					printf("%s\n", msg->toString().c_str());
//					break;
				}
			}
//			break;
		}
	}
	return 0;
}

void registerMVRR(){
	mvrrArgs.canId = arg_int1("i", "id", "<canId>", "CANBus device Id");
	mvrrArgs.attributeId = arg_int1("a", "aId", "<attributeId>", "Attribute Id");
	mvrrArgs.end = arg_end(3);

	const esp_console_cmd_t cmd = {
        .command = "mvrr",
        .help = "Request/response",
        .hint = NULL,
        .func = &mastervoltRequestResponse,
		.argtable = &mvrrArgs,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
