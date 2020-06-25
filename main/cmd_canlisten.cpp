#include <masterbusConsole.h>
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "esp_console.h"

static struct {
	struct arg_end *end;
} canListenArgs;

void print_packet(CANBusPacket packet){
	printf("DATA	0x%08lx	%d	", packet.canId, packet.dataLen);
	for(size_t i=0; i<packet.dataLen; i++){
		printf("%02x", packet.data[i]);
	}
	printf("\n");
}

static int canlisten(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &canListenArgs);
    if (nerrors != 0) {
        arg_print_errors(stderr, canListenArgs.end, argv[0]);
        return 1;
    }

    CANBusPacket packet;
    while(true){
    	bool hasPacket=g_mbctl->readPacket(packet);
    	if(hasPacket){
    		print_packet(packet);
    	}
//		vTaskDelay(5);
	}

	return 0;
}

void registerCanlisten(){
	canListenArgs.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "canlisten",
        .help = "Listens for data over the canbus",
        .hint = NULL,
        .func = &canlisten,
		.argtable = &canListenArgs,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
