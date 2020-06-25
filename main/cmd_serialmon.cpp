#include <masterbusConsole.h>
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "esp_console.h"


#include <map>
#include <vector>
#include <algorithm>

#include <sstream>
#include <iomanip>
#include <iterator>

#include <cstring>
#include <freertos/task.h>

#include "driver/uart.h"
extern "C" {
	#include <vedirect_parser.h>
}

#define BUF_SIZE (1024)

std::map<std::string, std::vector<std::string>> allVeDirectValueHistory;

void printVeDirectValueHistory(std::string key, std::vector<std::string> valueHistory){
	printf("%s	", key.c_str());
	for(auto it=valueHistory.begin(); it!=valueHistory.end(); it++){
		printf("%s	", it->c_str());
	}
	printf("\n");
}

void printVeDirectValueHistory(){
	printf("\033cBegin------\n");
	std::vector<std::string> sortedKeys;

	auto it=allVeDirectValueHistory.begin();
	for(;it!=allVeDirectValueHistory.end(); it++){
		sortedKeys.push_back(it->first);
	}
	std::sort(sortedKeys.begin(), sortedKeys.end());


	auto sortedKeyIt=sortedKeys.begin();
	for(;sortedKeyIt<sortedKeys.end(); sortedKeyIt++){
		std::vector<std::string>& history=allVeDirectValueHistory[*sortedKeyIt];
		printVeDirectValueHistory(*sortedKeyIt, history);
	}
}

std::string makeKeyFromVeDirectPacket(VEDirectStatus& veDirectReport){
	std::stringstream ss;
//	ss << "0x" << std::setw(2) << std::setfill('0') << std::hex << (int) mastervoltPacket.dataType << std::dec << '/';
//	ss << MastervoltDictionary::getInstance()->resolveAttributeIdToLabel(mastervoltPacket.attributeId);
	ss<<"Key goes here";
	return ss.str();
}

std::string makeValueFromVeDirectPacket(VEDirectStatus& veDirectReport){
	std::stringstream ss;
	ss<<"Value goes here";
	return ss.str();
}

void configureUart(uart_port_t uartPort, int txPin, int rxPin){
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 19200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.rx_flow_ctrl_thresh = 0,
		.use_ref_tick = false
    };
    uart_param_config(uartPort, &uart_config);
    uart_set_pin(uartPort, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(uartPort, BUF_SIZE * 2, 0, 0, NULL, 0);
}

bool readAndParseStatusReportFromUART(VEDirectStatus* report, uart_port_t uartPort){
	bool reportParsedOk=false;
    uint8_t *receptionBuffer = (uint8_t *) malloc(BUF_SIZE);
	int nbBytesReceived = uart_read_bytes(uartPort, receptionBuffer, BUF_SIZE, 100 / portTICK_RATE_MS);
	ESP_LOGD(__FUNCTION__, "Read %d bytes", nbBytesReceived);
	if(nbBytesReceived>0){
		ESP_LOG_BUFFER_HEXDUMP(__FUNCTION__, receptionBuffer, nbBytesReceived, ESP_LOG_WARN);
		parseVEDirectPacket(report, receptionBuffer, nbBytesReceived);
		reportParsedOk=true;
    }
    free(receptionBuffer);
    return reportParsedOk;
}

static int serialmonitor(int argc, char **argv) {
//	esp_log_level_set("*", ESP_LOG_ERROR);

	configureUart(UART_NUM_1, GPIO_NUM_23, GPIO_NUM_4);
	configureUart(UART_NUM_2, GPIO_NUM_23, GPIO_NUM_23);

	for(uint32_t i=0;i<200000; i++){
		VEDirectStatus leftReport;
		if(readAndParseStatusReportFromUART(&leftReport, UART_NUM_1)){
			std::string key=makeKeyFromVeDirectPacket(leftReport);
			std::string value=makeValueFromVeDirectPacket(leftReport);
			std::vector<std::string>& values=allVeDirectValueHistory[key];

			if(std::find(values.begin(), values.end(), value)==values.end()){
				values.push_back(value);
			}

			if(i%10==0) printVeDirectValueHistory();
		}
	}

	return 0;
}

void registerSerialMonitor(){
	const esp_console_cmd_t cmd = {
        .command = "serialmon",
        .help = "Display live serial data",
        .hint = NULL,
        .func = &serialmonitor,
		.argtable = NULL,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
