#include <masterbusConsole.h>
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "esp_console.h"
#include <memory.h>
#include <mvParser.hpp>

extern int hex2int(char input);


static struct {
	struct arg_lit* doRead;
	struct arg_int *canId;
	struct arg_str *data;
	struct arg_end *end;
} mvParseArgs;

static int mvparse(int argc, char **argv) {
	esp_log_level_set("parse", ESP_LOG_DEBUG);

	int nerrors = arg_parse(argc, argv, (void **) &mvParseArgs);
    if (nerrors != 0) {
        arg_print_errors(stderr, mvParseArgs.end, argv[0]);
        return 1;
    }
    ESP_LOGW(__FUNCTION__, "%s\n", MastervoltDictionary::getInstance()->toString().c_str());

    while(true){
		MvParser parser;
		MastervoltMessage* parsedMessage;
		if(mvParseArgs.doRead->count){
			ESP_LOGD(__FUNCTION__, "Reading packet");
			CANBusPacket packetRead;
			if(g_mbctl->readPacket(packetRead)==false){
				continue;
			}
			if(mvParseArgs.canId->count && packetRead.canId==mvParseArgs.canId->ival[0]){
				continue;
			}

			std::string packetData=packetRead.getData();
			parsedMessage=parser.parse(packetRead.canId, packetData);
		}
		else{
			long int canId=mvParseArgs.canId->ival[0];
			printf("mvParseArgs.canId=0x%lx\n", canId);
			printf("mvParseArgs.data=%s\n", mvParseArgs.data->sval[0]);
			uint8_t nbHexChars=strlen(mvParseArgs.data->sval[0]);
			if(nbHexChars%2!=0){
				fprintf(stderr, "Odd number of hex chars in '%s'", mvParseArgs.data->sval[0]);
				return 1;
			}
			size_t nbCanBytes=nbHexChars/2;
			std::string canBytes;
			const char* hexPtr=mvParseArgs.data->sval[0];
			for(int i=0; i<nbCanBytes; i++){
				canBytes.push_back((char)(hex2int(hexPtr[0])*16 + hex2int(hexPtr[1])));
				hexPtr+=2;
			}
			parsedMessage=parser.parse(canId, canBytes);
		}
		if(parsedMessage->type==MastervoltMessage::REQUEST){
			delete parsedMessage;
			continue;
		}
		const MastervoltAttributeKind* resolvedAttribute=MastervoltDictionary::getInstance()->resolveAttribute(parsedMessage->deviceKindId, parsedMessage->attributeId);
		printf("resolvedAttribute is %s\n", resolvedAttribute->toString().c_str());
		printf("%s\n", parsedMessage->toString().c_str());
		delete parsedMessage;
		break;
    }

	return 0;
}

void registerMastervoltParse(){
	mvParseArgs.doRead = arg_lit0("r", "read", "Read from mastervolt bus");
	mvParseArgs.canId = arg_int0("i", "id", "<t>", "Attribute Id");
	mvParseArgs.data = arg_str0("d", "data", "<d>", "Data to send");
	mvParseArgs.end = arg_end(3);

	const esp_console_cmd_t cmd = {
        .command = "mvparse",
        .help = "parse mastervolt data",
        .hint = NULL,
        .func = &mvparse,
		.argtable = &mvParseArgs,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
