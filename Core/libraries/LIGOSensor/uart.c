#include "main.h"
#include "stm32f4xx_hal.h"
#include "uart.h"
#include "ringbuffer.h"

extern UART_HandleTypeDef huart1;

#define RING_UART_LEN 64
uint8_t buff_uart[RING_UART_LEN];
RingBuff ring_buff;

void receive_rx(uint8_t data_rx)
{
	ring_buff_push(&ring_buff, data_rx);
}

uint16_t uart_available()
{
	return ring_buff_available(&ring_buff);
}

uint8_t uart_read()
{
	uint8_t data;
	ring_buff_pop(&ring_buff, &data);
	return data;
}

void uart_init(void)
{
	ring_buff_init(&ring_buff, buff_uart, RING_UART_LEN);
}

uint8_t CRC8(uint8_t data, uint8_t crc)
{
	uint8_t i = data ^ crc;
	crc = 0;
	if (i & 0x01) crc ^= 0x5e;
	if (i & 0x02) crc ^= 0xbc;
	if (i & 0x04) crc ^= 0x61;
	if (i & 0x08) crc ^= 0xc2;
	if (i & 0x10) crc ^= 0x9d;
	if (i & 0x20) crc ^= 0x23;
	if (i & 0x40) crc ^= 0x46;
	if (i & 0x80) crc ^= 0x8c;
	return crc;
}

uint8_t data_txCommand_Checksum(uint8_t *crcData, uint8_t lengData)
{
	uint8_t Checksum = 0;
	for(int i =0; i< lengData; i++)
	{
		Checksum = CRC8(crcData[i], Checksum);
	}
	return Checksum;
}

void Command_Format(uint8_t *data_txcommand, uint8_t *data_rxresponse)
{
	data_txcommand[0] = 0x31;
	data_txcommand[1] = 0x02;
	data_txcommand[2] = 0x07;
	//  data_txCommand[3] = data_txCommand_Checksum(data_txCommand[0],data_txCommand[1], data_txCommand[2], 0x02);
	data_txcommand[3] = data_txCommand_Checksum(data_txcommand, 3);
	HAL_UART_Transmit(&huart1, data_txcommand, 4, HAL_MAX_DELAY);
	uart_init();
	HAL_UART_Receive_IT(&huart1, data_rxresponse, 5);
}

void Process_Received_Data(uint8_t *Data, FrameData *frame)
{
    frame->rxPrefix = Data[0];
    frame->rxNetwork_address = Data[1];
    frame->rxOperation_code = Data[2];
    frame->rxTemperature = (int8_t)Data[3];
    frame->rxRelative_level = (uint16_t)(Data[5] << 8) | Data[4];
    frame->rxFrequency_value = (uint32_t)(Data[9] << 24) | (Data[8] << 16) | (Data[7] << 8) | Data[6];
    frame->rxChecksum = Data[10];
}
