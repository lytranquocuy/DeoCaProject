/*
 * TAU1201.c
 *
 *  Created on: Oct 26, 2023
 *      Author: selina
 */

#include "TAU1201.h"

#include <stdio.h>
#include <string.h>

#include "main.h"

#if (GPS_DEBUG == 1)
#include <usbd_cdc_if.h>
#endif

uint8_t rx_data = 0;
uint8_t rx_buffer[GPSBUFSIZE];
uint8_t rx_index = 0;

#if (GPS_DEBUG == 1)
void GPS_print(char *data){
	char buf[GPSBUFSIZE] = {0,};
	sprintf(buf, "%s\n", data);
	CDC_Transmit_FS((unsigned char *) buf, (uint16_t) strlen(buf));
}
#endif

void GPS_Init(TAU1201_t *_gps, UART_HandleTypeDef *_huart){
	_gps->huart = _huart;
	HAL_UART_Receive_IT(_gps->huart, &rx_data, 1);
}

void GPS_UART_CallBack(TAU1201_t *_gps){
	if (rx_data != '\n' && rx_index < sizeof(rx_buffer)) {
		rx_buffer[rx_index++] = rx_data;
	}
	else {

		#if (GPS_DEBUG == 1)
		GPS_print((char*)rx_buffer);
		#endif

		if(GPS_validate(_gps, (char*) rx_buffer))
			GPS_parse(_gps, (char*) rx_buffer);
		rx_index = 0;
		memset(rx_buffer, 0, sizeof(rx_buffer));
	}
	HAL_UART_Receive_IT(_gps->huart, &rx_data, 1);
}


int GPS_validate(TAU1201_t *_gps, char *nmeastr){
    char check[3];
    char checkcalcstr[3];
    int i;
    int calculated_check;

    i=0;
    calculated_check=0;

    // check to ensure that the string starts with a $
    if(nmeastr[i] == '$')
        i++;
    else
        return 0;

    //No NULL reached, 75 char largest possible NMEA message, no '*' reached
    while((nmeastr[i] != 0) && (nmeastr[i] != '*') && (i < 75)){
        calculated_check ^= nmeastr[i];// calculate the checksum
        i++;
    }

    if(i >= 75){
        return 0;// the string was too long so return an error
    }

    if (nmeastr[i] == '*'){
        check[0] = nmeastr[i+1];    //put hex chars in check string
        check[1] = nmeastr[i+2];
        check[2] = 0;
    }
    else
        return 0;// no checksum separator found there for invalid

    sprintf(checkcalcstr,"%02X",calculated_check);
    return((checkcalcstr[0] == check[0])
        && (checkcalcstr[1] == check[1])) ? 1 : 0 ;
	return 0;
}
//
void GPS_parse(TAU1201_t *_gps, char *GPSstrParse){
    if(!strncmp(GPSstrParse, "$GNGGA", 6)){
    	if (sscanf(GPSstrParse, "$GNGGA,%f,%f,%c,%f,%c,%d,%d,%f,%f,%c", &_gps->utc_time, &_gps->nmea_latitude, &_gps->ns, &_gps->nmea_longitude, &_gps->ew, &_gps->lock, &_gps->satelites, &_gps->hdop, &_gps->msl_altitude, &_gps->msl_units) >= 1){
    		_gps->dec_latitude = GPS_nmea_to_dec(_gps->nmea_latitude, _gps->ns);
    		_gps->dec_longitude = GPS_nmea_to_dec(_gps->nmea_longitude, _gps->ew);
    		return;
    	}
    }
    else if (!strncmp(GPSstrParse, "$GPRMC", 6)){
    	if(sscanf(GPSstrParse, "$GPRMC,%f,%f,%c,%f,%c,%f,%f,%d", &_gps->utc_time, &_gps->nmea_latitude, &_gps->ns, &_gps->nmea_longitude, &_gps->ew, &_gps->speed_k, &_gps->course_d, &_gps->date) >= 1)
    		return;

    }
    else if (!strncmp(GPSstrParse, "$GNGLL", 6)){
        if(sscanf(GPSstrParse, "$GNGLL,%f,%c,%f,%c,%f,%c", &_gps->nmea_latitude, &_gps->ns, &_gps->nmea_longitude, &_gps->ew, &_gps->utc_time, &_gps->gll_status) >= 1)
            return;
    }
    else if (!strncmp(GPSstrParse, "$GNVTG", 6)){
        if(sscanf(GPSstrParse, "$GNVTG,%f,%c,%f,%c,%f,%c,%f,%c", &_gps->course_t, &_gps->course_t_unit, &_gps->course_m, &_gps->course_m_unit, &_gps->speed_k, &_gps->speed_k_unit, &_gps->speed_km, &_gps->speed_km_unit) >= 1)
            return;
    }
}

float GPS_nmea_to_dec(float deg_coord, char nsew){
    int degree = (int)(deg_coord/100);
    float minutes = deg_coord - degree*100;
    float dec_deg = minutes / 60;
    float decimal = degree + dec_deg;
    if (nsew == 'S' || nsew == 'W') { // return negative
        decimal *= -1;
    }
    return decimal;
}

float getLatitude(TAU1201_t *_gps){
	return _gps->dec_latitude;
}

float getLongitude(TAU1201_t *_gps){
	return _gps->dec_longitude;
}

float getSpeed(TAU1201_t *_gps){
	return _gps->speed_k;
}

float getSatelitesNum(TAU1201_t *_gps){
	return _gps->satelites;
}
