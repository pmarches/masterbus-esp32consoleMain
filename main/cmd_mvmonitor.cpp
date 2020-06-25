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

#include <mvParser.hpp>

std::map<std::string, std::vector<std::string>> allValueHistory;

void printValueHistory(std::string key, std::vector<std::string> valueHistory){
	printf("%-40s	", key.c_str());
	for(auto it=valueHistory.begin(); it!=valueHistory.end(); it++){
		printf("%s	", it->c_str());
	}
	printf("\n");
}

void printValueHistory(){
	printf("\033cDeviceKind/DeviceUnique/AttId------\n");
	std::vector<std::string> sortedKeys;

	auto it=allValueHistory.begin();
	for(;it!=allValueHistory.end(); it++){
		sortedKeys.push_back(it->first);
	}
	std::sort(sortedKeys.begin(), sortedKeys.end());


	auto sortedKeyIt=sortedKeys.begin();
	for(;sortedKeyIt<sortedKeys.end(); sortedKeyIt++){
		std::vector<std::string>& history=allValueHistory[*sortedKeyIt];
		printValueHistory(*sortedKeyIt, history);
	}
}

std::string makeKeyFromMVPacket(MastervoltMessage* parsedMessage){
	std::stringstream ss;
	ss << std::setw(4) << std::setfill('0') << std::hex;
	ss<<parsedMessage->deviceKindId
		<<"/"<<parsedMessage->deviceUniqueId
		<<"/"<<parsedMessage->attributeId
		<<"\t";

	const MastervoltDeviceKind* resolvedDevice=MastervoltDictionary::getInstance()->resolveDevice(parsedMessage->deviceKindId);
	if(resolvedDevice){
		ss<<resolvedDevice->textDescription;
	}
	else{
		ss<<"NullDevice";
	}

	const MastervoltAttributeKind* resolvedAttribute=MastervoltDictionary::getInstance()->resolveAttribute(parsedMessage->deviceKindId, parsedMessage->attributeId);
	if(resolvedAttribute){
		ss<<"/"<<resolvedAttribute->textDescription;
	}

	return ss.str();
}

std::string makeValueFromMVPacket(MastervoltMessage* parsedMessage){
	std::stringstream ss;
	if(parsedMessage->type==MastervoltMessage::FLOAT){
		ss <<((MastervoltMessageFloat*) parsedMessage)->floatValue;
	}
	else if(parsedMessage->type==MastervoltMessage::DATE){
		MastervoltMessageDate* dateMsg=((MastervoltMessageDate*) parsedMessage);
		ss <<(uint16_t)dateMsg->day<<"/"<<(uint16_t)dateMsg->month<<"/"<<(uint16_t)dateMsg->year;
	}
	else if(parsedMessage->type==MastervoltMessage::TIME){
		MastervoltMessageTime* timeMsg=((MastervoltMessageTime*) parsedMessage);
		ss <<(uint16_t)timeMsg->hour<<":"<<(uint16_t)timeMsg->minute<<":"<<(uint16_t)timeMsg->second;
	}
	else if(parsedMessage->type==MastervoltMessage::LABEL){
		MastervoltMessageLabel* labelMsg=((MastervoltMessageLabel*) parsedMessage);
		ss <<labelMsg->segmentNumber<<"#"<<labelMsg->label;
	}
	else if(parsedMessage->type==MastervoltMessage::UNKNOWN){
		MastervoltMessageUnknown* uknMsg=((MastervoltMessageUnknown*) parsedMessage);
		ss<< " Unknown:" << std::setw(2) << std::setfill('0') << std::hex << std::uppercase;
		std::copy(uknMsg->unknownBytes.begin(), uknMsg->unknownBytes.end(), std::ostream_iterator<uint32_t>(ss, "_"));
	}
	else{
		ss <<"else "<<parsedMessage->toString();
	}
	return ss.str();
}

static int mvmonitor(int argc, char **argv) {
	esp_log_level_set("*", ESP_LOG_ERROR);

	CANBusPacket packetRead;
	MvParser parser;
	for(uint32_t i=0;i<200000; i++){
		if(g_mbctl->readPacket(packetRead)){
			std::string packetData=packetRead.getData();

			MastervoltMessage* parsedMessage=parser.parse(packetRead.canId, packetData);
			if(parsedMessage){
//				if(parsedMessage->type==MastervoltMessage::REQUEST){
//					continue;
//				}
				std::string key=makeKeyFromMVPacket(parsedMessage);
				std::string value=makeValueFromMVPacket(parsedMessage);
				std::vector<std::string>& values=allValueHistory[key];

//				if(values.size()>=6){
//					values.erase(values.begin());
//				}
#if 0
				values.push_back(value);
#else
				if(std::find(values.begin(), values.end(), value)==values.end()){
					values.push_back(value);
				}
				delete parsedMessage;
#endif
			}
			if(i%10==0) printValueHistory();
		}
	}

	return 0;
}

void registerMvMonitor(){
	const esp_console_cmd_t cmd = {
        .command = "mvmonitor",
        .help = "Display live mastervolt data",
        .hint = NULL,
        .func = &mvmonitor,
		.argtable = NULL,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
