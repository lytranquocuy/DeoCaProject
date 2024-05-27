/*
 * 24LCxx.h
 *
 *  Created on: May 26, 2024
 *      Author: Selina
 */

#ifndef INC_24LCXX_H_
#define INC_24LCXX_H_

#include <stdio.h>
#include "main.h"

#define EEPROM_WRITE_TIMEOUT	500
#define EEPROM_READ_TIMEOUT		500
#define EEPROM_OK				1
#define EEPROM_ERROR			0

typedef struct{
	I2C_HandleTypeDef *i2c;
	uint8_t address;
	uint16_t eeprom_size;
	uint16_t bytes;
}_24LCxx_hardware_t;

typedef struct{
	_24LCxx_hardware_t hardware;
}eeprom_24LCxx_t;

void EEPROM_init(eeprom_24LCxx_t *_eeprom, I2C_HandleTypeDef *i2c, uint8_t address, uint16_t eeprom_size);
uint16_t EEPROM_Length(eeprom_24LCxx_t *_eeprom);

uint8_t EEPROM_write_byte(eeprom_24LCxx_t *_eeprom, uint16_t start_data_addr, uint8_t *data_write);
uint8_t EEPROM_write_bytes(eeprom_24LCxx_t *_eeprom, uint16_t start_data_addr, uint8_t *data_write, uint16_t data_len);

uint8_t EEPROM_read_byte(eeprom_24LCxx_t *_eeprom, uint16_t start_data_addr, uint8_t *data_write);
uint8_t EEPROM_read_bytes(eeprom_24LCxx_t *_eeprom, uint16_t start_data_addr, uint8_t *data_write, uint16_t data_len);

//void clear_EEPROM(eeprom_24LCxx_t *_eeprom);

#endif /* INC_24LCXX_H_ */
