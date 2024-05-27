/*
 * sim7600.c
 *
 *  Created on: May 18, 2024
 *      Author: Selina
 */

#include "sim7600.h"

#define PRIME 101 // Số nguyên tố để tính mã băm
int findSubstring(char *str, char *substr) {
    int len_str = strlen(str);
    int len_substr = strlen(substr);
    int i, j;
    int hash_str = 0; // Mã băm của chuỗi str
    int hash_substr = 0; // Mã băm của chuỗi substr
    int h = 1;
    // Tính h = PRIME^(len_substr-1) % PRIME
    for (i = 0; i < len_substr - 1; i++) {
        h = (h * 256) % PRIME;
    }
    // Tính mã băm cho chuỗi substr và len_substr ký tự đầu tiên của chuỗi str
    for (i = 0; i < len_substr; i++) {
        hash_substr = (256 * hash_substr + substr[i]) % PRIME;
        hash_str = (256 * hash_str + str[i]) % PRIME;
    }
    // Duyệt qua các ký tự của chuỗi str
    for (i = 0; i <= len_str - len_substr; i++) {
        // Nếu mã băm của chuỗi substr và len_substr ký tự tiếp theo của chuỗi str trùng nhau
        if (hash_substr == hash_str) {
            // Kiểm tra từng ký tự của chuỗi substr và chuỗi con tương ứng trong chuỗi str
            for (j = 0; j < len_substr; j++) {
                if (str[i + j] != substr[j])
                    break;
            }
            // Nếu tất cả các ký tự đều trùng khớp
            if (j == len_substr)
                return i;
        }
        // Tính lại mã băm cho len_substr ký tự tiếp theo của chuỗi str
        if (i < len_str - len_substr) {
            hash_str = (256 * (hash_str - str[i] * h) + str[i + len_substr]) % PRIME;
            if (hash_str < 0)
                hash_str = (hash_str + PRIME);
        }
    }
    return -1; // Trả về -1 nếu không tìm thấy chuỗi con
}
// Thuật toán tìm kím chuỗi con

// Uart DMA CallBack
void SIM_UART_DMA_CallBack(sim7600_t *_sim){
	HAL_UARTEx_ReceiveToIdle_DMA(_sim->hardware.huart, (uint8_t *)_sim->buffer, SIM_RX_BUFF_SIZE);
}

void CheckSIMStatus(sim7600_t *_sim){
	GetLBS(_sim);
	GetSignal(_sim);
}

// Clear UART Buffer
void ClearBuffer(sim7600_t *_sim){
	HAL_UART_DMAStop(_sim->hardware.huart);
	HAL_UARTEx_ReceiveToIdle_DMA(_sim->hardware.huart, (uint8_t *)_sim->buffer, SIM_RX_BUFF_SIZE);
	memset(_sim->buffer, 0, SIM_RX_BUFF_SIZE);
}

// Wait until SIMCOM module response data
// If timeout: return -1
// If find value: return the first char's address
int WaitForResponse(sim7600_t *_sim, char *response, long long timeout, uint8_t clear_buffer){
    int found;
    long long time = timeout;
#ifdef SIM_Debug
    strcpy(_sim->DebugStr, response);
#endif
    while(time >= 0){
        found = findSubstring(_sim->buffer, response);
		_sim->x = found;

        if(found != -1){
            if(clear_buffer){
                ClearBuffer(_sim);
            }
#ifdef SIM_Debug
//	strcpy(_sim->response,"FoundSubStr");
#endif
            return found;
        }
        time = time - 10;
        SIM_Delay(10);
    }

#ifdef SIM_Debug
//	strcpy(_sim->response,"TIME OUT");
#endif

    if(clear_buffer){
        ClearBuffer(_sim);
    }
    return -1;
}

// Wait until SIMCOM module response data and check which data recieved
// If timeout, return -1
// If found response true, return 1
// If found response false, return 0
int WaitForResponceTrueFalse(sim7600_t *_sim, char *responceTrue, char *responceFalse, long long timeout, uint8_t clear_buffer){
    int foundTrue;
    int foundFalse;
    long long time = timeout;

    while(time >= 0){
        foundTrue = findSubstring(_sim->buffer, responceTrue);

        if(foundTrue != -1){
            if(clear_buffer){
                ClearBuffer(_sim);
            }
#ifdef SIM_Debug
	strcpy(_sim->response,"FoundSubStrTrue");
#endif
            return 1;
        }

        foundFalse = findSubstring(_sim->buffer, responceFalse);

        if(foundFalse != -1){
            if(clear_buffer){
                ClearBuffer(_sim);
            }
#ifdef SIM_Debug
	strcpy(_sim->response,"FoundSubStrFalse");
#endif
            return 0;
        }

        time = time - 10;
        SIM_Delay(10);
    }

    if(clear_buffer){
        ClearBuffer(_sim);
    }
    return -1;
}

int SendCommand(sim7600_t *_sim, char *command, char *response, long long timeout, uint8_t clear_buffer){
	uint16_t length = strlen(command);
	if(HAL_UART_Transmit(_sim->hardware.huart, (uint8_t*)command, length, 1000) != HAL_OK){
		return SIM7600_ERROR;
	}
	else{
		return WaitForResponse(_sim, response, timeout, clear_buffer);
	}
}

int SendCommandCheckTrueFalse(sim7600_t *_sim, char *command, char *responceTrue, char *responceFalse, long long timeout, uint8_t clear_buffer){
	uint16_t length = strlen(command);
	if(HAL_UART_Transmit(_sim->hardware.huart, (uint8_t*)command, length, 1000) != HAL_OK){
		return SIM7600_ERROR;
	}
	else{
		return WaitForResponceTrueFalse(_sim, responceTrue, responceFalse, timeout, clear_buffer);
	}
}

// Init module
int SIMInit(sim7600_t *_sim, UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma, uint16_t PowerPin, GPIO_TypeDef *PowerPort, uint16_t ResetPin, GPIO_TypeDef *ResetPort, uint16_t StatusPin, GPIO_TypeDef *StatusPort){
	_sim->hardware.huart = huart;
	_sim->hardware.hdma = hdma;
// Power pin
	_sim->hardware.PowerPin = PowerPin;
	_sim->hardware.PowerPort = PowerPort;
// Reset Pin
	_sim->hardware.ResetPin = ResetPin;
	_sim->hardware.ResetPort = ResetPort;

// Status Pin
	_sim->hardware.StatusPin = StatusPin;
	_sim->hardware.StatusPort = StatusPort;

	HAL_UARTEx_ReceiveToIdle_DMA(_sim->hardware.huart, (uint8_t *)_sim->buffer, SIM_RX_BUFF_SIZE);
	__HAL_DMA_DISABLE_IT(_sim->hardware.hdma, DMA_IT_HT);

	return StartModuleService(_sim);
}

// When call this function, It'll power on module
int PowerOn(sim7600_t *_sim){
#ifdef SIM_Debug
	strcpy(_sim->response,"POWER ON");
#endif
	HAL_GPIO_WritePin(_sim->hardware.PowerPort, _sim->hardware.PowerPin, GPIO_PIN_SET);
	SIM_Delay(600);
	HAL_GPIO_WritePin(_sim->hardware.PowerPort, _sim->hardware.PowerPin, GPIO_PIN_RESET);
	SIM_Delay(1000);
	return SIM7600_OK;
}

// When call this function, It will pull high Module's reset pin and pull low
// to reset module.
int ResetModule(sim7600_t *_sim){
#ifdef SIM_Debug
	strcpy(_sim->response,"RESET MODULE");
#endif
	HAL_GPIO_WritePin(_sim->hardware.ResetPort, _sim->hardware.ResetPin, GPIO_PIN_SET);
	SIM_Delay(300);
	HAL_GPIO_WritePin(_sim->hardware.ResetPort, _sim->hardware.ResetPin, GPIO_PIN_RESET);

	while(WaitForResponse(_sim, "PB DONE", 5000, 1) == -1){
#ifdef SIM_Debug
		strcpy(_sim->response,"WAIT FOR MODULE READY");
#endif
		SIM_Delay(1000);
	}

#ifdef SIM_Debug
	strcpy(_sim->response,"RESET MODULE OK");
	++_sim->SIMResetCount;
#endif
	SIM_Delay(2000);
	return SIM7600_OK;
}

int StartModuleService(sim7600_t *_sim){

	if(PowerOn(_sim) == SIM7600_OK){
		if(ResetModule(_sim) == SIM7600_OK){
			_sim->ModulePower = 1;
		}
		else{
			_sim->ModulePower = 0;
			return SIM7600_ERROR;
		}
	}

	if(ConnectToModule(_sim) == SIM7600_OK){
		_sim->ModuleConnect = 1;
	}
	else{
		_sim->ModuleConnect = 0;
		return SIM7600_ERROR;
	}

	CheckSimConnect(_sim);
	return SIM7600_OK;
}

// When call this function, It'll send "AT" command to module
// If connect to module complete, It'll responce "OK"
int ConnectToModule(sim7600_t *_sim){
//	Send AT
	int Check = 0;
	while(SendCommand(_sim, SIM_AT, SIM_OK, 1000, 1) == -1){
		++Check;
		if(Check > 30){
#ifdef SIM_Debug
			strcpy(_sim->response,"Send AT error");
#endif
			_sim->ModuleConnect = 0;
			return SIM7600_ERROR;
		}
	}
#ifdef SIM_Debug
	strcpy(_sim->response,"Send AT OK");
#endif

// Send ATE0
	if(SendCommand(_sim, SIM_ATE0, SIM_OK, 1000, 1) != -1){
#ifdef SIM_Debug
		strcpy(_sim->response,"Send ATE0 OK");
#endif
	}
	else{
#ifdef SIM_Debug
		strcpy(_sim->response,"Send ATE0 error");
#endif
		_sim->ModuleConnect = 0;
		return SIM7600_ERROR;
	}
	_sim->ModuleConnect = 1;
	return SIM7600_OK;
}

// This function'll check the sim card connection.
int CheckSimConnect(sim7600_t *_sim){
//	//	Setup Hotswap Level
//	if(SendCommand(_sim, "AT+UIMHOTSWAPLEVEL=1\r\n", SIM_OK, 1000, 1) == -1){
//#ifdef SIM_Debug
//		strcpy(_sim->response,"SETUP HOTSWAP LEVEL ERROR");
//#endif
//		return SIM7600_ERROR;
//	}
//	Check SIM connect
	if(SendCommand(_sim, "AT+CPIN?\r\n", "+CPIN: READY", 1000, 1) == -1){
#ifdef SIM_Debug
		strcpy(_sim->response,"NO SIM");
#endif
		_sim->SimConnect = 0;
		return SIM7600_ERROR;
	}

#ifdef SIM_Debug
	strcpy(_sim->response,"SIM CONNECTED");
#endif

	GetPhoneNumber(_sim);

	_sim->SimConnect = 1;
	return SIM7600_OK;
}

// Get SIMCardPhoneNumber
int GetPhoneNumber(sim7600_t *_sim){
	int foundRes = SendCommand(_sim, "AT+CNUM\r\n", "+CNUM:", 2000, 0);
	if(foundRes == -1){
#ifdef SIM_Debug
		strcpy(_sim->response,"Get Phone Number ERROR");
#endif
		ClearBuffer(_sim);
		return SIM7600_ERROR;
	}

	char DataResponseStr[60];
	int count = 0;
	for(int i = foundRes; i <= SIM_RX_BUFF_SIZE; ++i){
		if(_sim->buffer[i] == '\r') break;
		DataResponseStr[count] = _sim->buffer[i];
		++count;
	}
#ifdef SIM_Debug
	strcpy(_sim->DebugResponceStr, DataResponseStr);
#endif

	int AnalysisPointer = 0;
	count = 0;
	for(int i = AnalysisPointer; i <= 60; ++i){
		if(DataResponseStr[i] == '\"') ++count;
		if(count == 3){
			AnalysisPointer = i + 1;
			break;
		}
	}
	count = 0;
	memset(_sim->PhoneNumber,0,strlen(_sim->PhoneNumber));

	for(int i = AnalysisPointer; i <= 60; ++i){
		if(DataResponseStr[i] == '\"') break;
		_sim->PhoneNumber[count] = DataResponseStr[i];
		++count;
	}

	return SIM7600_OK;
//	int DKD;
//	sscanf(DataResponseStr, "+CNUM: \"My Number\",\"%s\",%d", &_sim->PhoneNumber, DKD);
}

int dayOfWeek(int day, int month, int year) {
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (month < 3) {
        year -= 1;
    }
    return (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
}

int get_timezone_offset(float longitude) {
    return (int)round(longitude / 15.0);
}

// Get LBS
int GetLBS(sim7600_t *_sim){
	GetSignal(_sim);
	if(_sim->Signal == 0){
#ifdef SIM_Debug
		strcpy(_sim->response,"No Signal");
#endif
		return SIM7600_ERROR;
	}

	int foundRes = SendCommand(_sim, "AT+CLBS=4\r\n", "+CLBS:", 2000, 0);
	if(foundRes == -1){
#ifdef SIM_Debug
		strcpy(_sim->response,"Get LBS Location Error");
#endif
		ClearBuffer(_sim);
		return SIM7600_ERROR;
	}
	HAL_Delay(50);

	char DataResponseStr[60];
	int count = 0;
	for(int i = foundRes; i <= SIM_RX_BUFF_SIZE; ++i){
		if(_sim->buffer[i] == '\r') break;
		DataResponseStr[count] = _sim->buffer[i];
		++count;
	}
#ifdef SIM_Debug
	strcpy(_sim->DebugLBSResponceStr, DataResponseStr);
#endif
	int DKD;
	sscanf(DataResponseStr, "+CLBS: 0,%f,%f,%d,%d/%d/%d,%d:%d:%d", &_sim->LBS_Loc.Latitude, &_sim->LBS_Loc.Longitude, &DKD, &_sim->LBS_Date.Year, &_sim->LBS_Date.Month, &_sim->LBS_Date.Date, &_sim->LBS_Date.Hour, &_sim->LBS_Date.Minute, &_sim->LBS_Date.Second);
	_sim->LBS_Date.Day = dayOfWeek(_sim->LBS_Date.Date, _sim->LBS_Date.Month, _sim->LBS_Date.Year) + 1;
	_sim->LBS_Date.Year -= 2000;
	_sim->LBS_Date.Hour += get_timezone_offset(_sim->LBS_Loc.Longitude);

	ClearBuffer(_sim);
#ifdef SIM_Debug
		strcpy(_sim->response,"Get LBS Location OK");
#endif

	return SIM7600_OK;
}

// Get RSSI
int GetRSSI(sim7600_t *_sim){
	int foundRes = SendCommand(_sim, "AT+CSQ\r\n", "+CSQ:", 2000, 0);
	if(foundRes == -1){
#ifdef SIM_Debug
		strcpy(_sim->response,"Get RSSI Error");
#endif
		ClearBuffer(_sim);
		return SIM7600_ERROR;
	}

	char DataResponseStr[50];
	int count = 0;
	for(int i = foundRes; i <= SIM_RX_BUFF_SIZE; ++i){
		if(_sim->buffer[i] == '\r') break;
		DataResponseStr[count] = _sim->buffer[i];
		++count;
	}

	int Ber;
	sscanf(DataResponseStr, "+CSQ: %d,%d", &_sim->RSSI, &Ber);
	ClearBuffer(_sim);
#ifdef SIM_Debug
//		strcpy(_sim->response,"Get RSSI OK");
#endif

	return SIM7600_OK;
}

int GetSignal(sim7600_t *_sim){
	if(GetRSSI(_sim) == SIM7600_OK){
		if((_sim->RSSI == 99) || (_sim->RSSI == 199) || (_sim->RSSI == 0) || (_sim->RSSI == 1) || (_sim->RSSI == 100) || (_sim->RSSI == 101)){
			_sim->Signal = 0;
			_sim->NetTech = 0;
		}
		else if((_sim->RSSI >= 2) && (_sim->RSSI <= 31)){
			_sim->NetTech = 4;
			if((_sim->RSSI >= 2) && (_sim->RSSI <= 7)) _sim->Signal = 1;
			else if((_sim->RSSI >= 8) && (_sim->RSSI <= 15)) _sim->Signal = 2;
			else if((_sim->RSSI >= 16) && (_sim->RSSI <= 23)) _sim->Signal = 3;
			else if((_sim->RSSI >= 24) && (_sim->RSSI <= 31)) _sim->Signal = 4;
		}
		else if((_sim->RSSI >= 102) && (_sim->RSSI <= 191)){
			_sim->NetTech = 3;
			if((_sim->RSSI >= 102) && (_sim->RSSI <= 122)) _sim->Signal = 1;
			else if((_sim->RSSI >= 123) && (_sim->RSSI <= 145)) _sim->Signal = 2;
			else if((_sim->RSSI >= 146) && (_sim->RSSI <= 168)) _sim->Signal = 3;
			else if((_sim->RSSI >= 169) && (_sim->RSSI <= 191)) _sim->Signal = 4;
		}
		return SIM7600_OK;
	}
	return SIM7600_ERROR;
}

// HTTP GET FUNCTION
int http_get(sim7600_t *_sim, char *url, char *response, int *httpCode){
	int ErrCheck = SIM7600_OK;

	GetSignal(_sim);
	if(_sim->Signal == 0){
#ifdef SIM_Debug
		strcpy(_sim->response,"No Signal");
#endif
		return SIM7600_ERROR;
	}

//	Init HTTP
	if(SendCommand(_sim, "AT+HTTPINIT\r\n", SIM_OK, 2000, 1) == -1){
#ifdef SIM_Debug
		strcpy(_sim->response,"Init HTTP error");
#endif
		ErrCheck = SIM7600_ERROR;
		goto StopHttpGet;
	}

//	Setup HTTP URL
	char urlStr[200];
	sprintf(urlStr,"AT+HTTPPARA=\"URL\",\"%s\"\r\n", url);
	if(SendCommand(_sim, urlStr, SIM_OK, 1000, 1) == -1){
#ifdef SIM_Debug
		strcpy(_sim->response,"Setup HTTP URL error");
#endif
		ErrCheck = SIM7600_ERROR;
	}

//	Send HTTP GET request
	int foundRes = SendCommand(_sim, "AT+HTTPACTION=0\r\n", "+HTTPACTION: 0", 50000, 0);
	if(foundRes == -1){
#ifdef SIM_Debug
		strcpy(_sim->response,"Send HTTP request error");
#endif
		ErrCheck = SIM7600_ERROR;
	}
	SIM_Delay(10);

	char DataResponseStr[50];
	int count = 0;
	for(int i = foundRes; i <= SIM_RX_BUFF_SIZE; ++i){
		if(_sim->buffer[i] == '\r') break;
		DataResponseStr[count] = _sim->buffer[i];
		++count;
	}
	int DataResponseLen = -1;

	int analysisHttpCode;

	sscanf(DataResponseStr, "+HTTPACTION: 0,%d,%d", &analysisHttpCode, &DataResponseLen);
	*httpCode = analysisHttpCode;

	if(*httpCode == 200){
#ifdef SIM_Debug
		strcpy(_sim->DebugResponceStr, DataResponseStr);
		_sim->DebugHTTPGetCode = *httpCode;
		_sim->DebugHTTPGetDataLen = DataResponseLen;
#endif
		ClearBuffer(_sim);
		char readDataStr[50];
		sprintf(readDataStr,"AT+HTTPREAD=0,%d\r\n", *httpCode);

		foundRes = SendCommand(_sim, readDataStr, "+HTTPREAD:", 2000, 0);
		if(foundRes == -1){
#ifdef SIM_Debug
			strcpy(_sim->response,"Read HTTP data ERROR");
#endif
			ErrCheck = SIM7600_ERROR;
		}

		while(WaitForResponse(_sim, "+HTTPREAD: 0", 1000, 0) == -1);

		for(int i = foundRes; i <= SIM_RX_BUFF_SIZE; ++i){
			if(_sim->buffer[i] == '\n'){
				foundRes = i + 1;
				break;
			}
		}
		count = 0;
		for(int i = foundRes; i <= SIM_RX_BUFF_SIZE; ++i){
			if(_sim->buffer[i] == '\r') break;
			response[count] = _sim->buffer[i];
			++count;
		}

		ClearBuffer(_sim);
	}
	else{
#ifdef SIM_Debug
		strcpy(_sim->response,"HTTP CODE RETURN ERROR");
#endif
	}

//	Stop HTTP request
	StopHttpGet:
	if(SendCommand(_sim, "AT+HTTPTERM\r\n", SIM_OK, 1000, 1) == -1){
#ifdef SIM_Debug
		strcpy(_sim->response,"Stop HTTP error");
#endif
		ErrCheck = SIM7600_ERROR;
	}

	if(ErrCheck == SIM7600_ERROR){
		++_sim->SuperviseHTTPError;

		if(_sim->SuperviseHTTPError > MAX_HTTP_ERROR_COUNT){
			StartModuleService(_sim);
			_sim->SuperviseHTTPError = 0;
		}
#ifdef SIM_Debug
//		strcpy(_sim->response,"HTTP GET ERROR");
		++_sim->CountHTTPGetError;
#endif
		return SIM7600_ERROR;
	}

	_sim->SuperviseHTTPError = 0;
#ifdef SIM_Debug
	strcpy(_sim->response,"HTTP GET OK");
	++_sim->CountHTTPGetOK;
#endif
	return SIM7600_OK;
}


// HTTP POST FUNCTION
int http_post(sim7600_t *_sim, char *url, char *data){
	int ErrCheck = SIM7600_OK;

	GetSignal(_sim);
	if(_sim->Signal == 0){
#ifdef SIM_Debug
		strcpy(_sim->response,"No Signal");
#endif
		return SIM7600_ERROR;
	}

//	Init HTTP
	if(SendCommand(_sim, "AT+HTTPINIT\r\n", SIM_OK, 2000, 1) == -1){
#ifdef SIM_Debug
		strcpy(_sim->response,"Init HTTP error1");
#endif
		ErrCheck = SIM7600_ERROR;
		goto StopHttpPost;
	}

//	Setup HTTP URL
	char urlStr[200];
	sprintf(urlStr,"AT+HTTPPARA=\"URL\",\"%s\"\r\n", url);
	if(SendCommand(_sim, urlStr, SIM_OK, 2000, 1) == -1){
#ifdef SIM_Debug
		strcpy(_sim->response,"Setup HTTP URL error");
#endif
		ErrCheck = SIM7600_ERROR;
	}

	if(SendCommand(_sim, "AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n", SIM_OK, 2000, 1) == -1){
#ifdef SIM_Debug
		strcpy(_sim->response,"Set HTTPPARA ERROR");
#endif
		ErrCheck = SIM7600_ERROR;
	}

// Send HTTP DATA
	char httpDataStr[200];
	sprintf(httpDataStr,"AT+HTTPDATA=%d,1000\r\n", strlen(data));
#ifdef SIM_Debug
	strcpy(_sim->DebugStr, httpDataStr);
#endif
	SendCommand(_sim, httpDataStr, "DOWNLOAD", 2000, 1);
	SendCommand(_sim, data, SIM_OK, 2000, 1);

//	Send HTTP POST request
	int responce = SendCommandCheckTrueFalse(_sim, "AT+HTTPACTION=1\r\n", "+HTTPACTION: 1,200", SIM_ERROR, 50000, 0);

	if(responce != 1){
#ifdef SIM_Debug
		strcpy(_sim->response,"Send HTTP request error");
#endif
//		return A7680_ERROR;
		ErrCheck = SIM7600_ERROR;
	}

	ClearBuffer(_sim);


//	Stop HTTP request
	StopHttpPost:
	if(SendCommand(_sim, "AT+HTTPTERM\r\n", SIM_OK, 2000, 1) == -1){
#ifdef SIM_Debug
		strcpy(_sim->response,"Stop HTTP error");
#endif
		ErrCheck = SIM7600_ERROR;
	}

	if(ErrCheck == SIM7600_ERROR){
		++_sim->SuperviseHTTPError;
		if(_sim->SuperviseHTTPError > MAX_HTTP_ERROR_COUNT){
			StartModuleService(_sim);
			_sim->SuperviseHTTPError = 0;
		}
#ifdef SIM_Debug
//		strcpy(_sim->response,"HTTP POST ERROR");
		++_sim->CountHTTPPostError;
#endif
		return SIM7600_ERROR;
	}
	_sim->SuperviseHTTPError = 0;
#ifdef SIM_Debug
	strcpy(_sim->response,"HTTP POST OK");
	++_sim->CountHTTPPostOK;
#endif
	return SIM7600_OK;
}



