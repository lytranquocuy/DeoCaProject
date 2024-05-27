#ifndef UART_H_
#define UART_H_
#include <stdint.h>
#include "string.h"
#define FRAM_SIZE 11

typedef struct
{
	uint8_t 	rxPrefix;
	uint8_t 	rxNetwork_address;
	uint8_t 	rxOperation_code;
	uint8_t 	rxTemperature;
	uint16_t 	rxRelative_level;
	uint32_t 	rxFrequency_value;
	uint8_t 	rxChecksum;
} FrameData;

void Process_Received_Data(uint8_t *Data, FrameData *frame);

uint16_t uart_available(void);
uint8_t uart_read(void);
void receive_rx(uint8_t data_rx);
void uart_init(void);
uint8_t CRC8(uint8_t data, uint8_t crc);
uint8_t data_txCommand_Checksum(uint8_t *crcData, uint8_t lengData);
void Command_Format(uint8_t *data_txcommand, uint8_t *data_rxresponse);
#endif
