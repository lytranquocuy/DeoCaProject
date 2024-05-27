/*
 * 24LCxx.c
 *
 *  Created on: May 26, 2024
 *      Author: Selina
 */
#include"24LCxx.h"

void EEPROM_init(eeprom_24LCxx_t *_eeprom, I2C_HandleTypeDef *i2c, uint8_t address, uint16_t eeprom_size){
	_eeprom->hardware.i2c = i2c;
	_eeprom->hardware.address = address << 1;
	_eeprom->hardware.eeprom_size = eeprom_size;
	_eeprom->hardware.bytes = (_eeprom->hardware.eeprom_size * 128); //(EEPROM SIZE * 1024)/8

}

uint16_t EEPROM_Length(eeprom_24LCxx_t *_eeprom){
	return (_eeprom->hardware.bytes-1);
}

uint8_t i2c_write_bytes(eeprom_24LCxx_t *_eeprom, uint16_t data_addr, uint8_t *data, uint16_t data_len){
	if(HAL_I2C_Mem_Write(_eeprom->hardware.i2c, _eeprom->hardware.address, data_addr, 0x10, data, data_len, EEPROM_WRITE_TIMEOUT)!= HAL_OK){
		return EEPROM_ERROR;
	}
	return EEPROM_OK;
}

uint8_t EEPROM_write_byte(eeprom_24LCxx_t *_eeprom, uint16_t start_data_addr, uint8_t *data_write){
	return i2c_write_bytes(_eeprom, start_data_addr, data_write, 1);
}

uint8_t EEPROM_write_bytes(eeprom_24LCxx_t *_eeprom, uint16_t start_data_addr, uint8_t *data_write, uint16_t data_len){
	return i2c_write_bytes(_eeprom, start_data_addr, data_write, data_len);
}

uint8_t i2c_read_bytes(eeprom_24LCxx_t *_eeprom, uint16_t data_addr, uint8_t *data, uint16_t data_len){
	if(HAL_I2C_Mem_Read(_eeprom->hardware.i2c, _eeprom->hardware.address, data_addr, 0x10, data, data_len, EEPROM_READ_TIMEOUT)!= HAL_OK){
		return EEPROM_ERROR;
	}
	return EEPROM_OK;
}

uint8_t EEPROM_read_byte(eeprom_24LCxx_t *_eeprom, uint16_t start_data_addr, uint8_t *data_write){
	return i2c_read_bytes(_eeprom, start_data_addr, data_write, 1);
}

uint8_t EEPROM_read_bytes(eeprom_24LCxx_t *_eeprom, uint16_t start_data_addr, uint8_t *data_write, uint16_t data_len){
	return i2c_read_bytes(_eeprom, start_data_addr, data_write, data_len);
}

//void clear_EEPROM(eeprom_24LCxx_t *_eeprom){
//    uint8_t *data_write;
//    data_write = (uint8_t *)calloc(EEPROM_Length(_eeprom), sizeof(uint8_t));
//    EEPROM_write_bytes(_eeprom, 0, (uint8_t *)data_write, EEPROM_Length(_eeprom));
//    free(data_write);
////    return ESP_OK;
//
////	return EEPROM_write_bytes(_eeprom, 0, (uint8_t *)0, EEPROM_Length(_eeprom));
//}





