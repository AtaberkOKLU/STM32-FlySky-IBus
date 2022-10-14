/*
 * Telemetry.c
 *
 *  Created on: Aug 18, 2022
 *      Author: Ataberk OKLU
 */

#include "main.h"

#include <Telemetry.h>

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;

// Transiever Variables
volatile uint8_t Transiever_RX_Buffer[SENSOR_BUFFER_SIZE];
volatile uint8_t TX_Buffer[SENSOR_TX_BUF_SIZE_8] = {0};

TelemetryServoStruct ServoList;
volatile uint8_t Transiever_TX_Buffer[SERVO_BUFFER_SIZE];
volatile struct __FLAGS FLAGS;

extern TelemetrySensorStruct SensorList[NUM_SENSORS];

void Sensor_UART_Telemetry_Init(UART_HandleTypeDef *huart) {
	  HAL_UART_RegisterCallback(huart, HAL_UART_RX_COMPLETE_CB_ID, Sensor_UART_RxComplete_Callback);
	  HAL_UART_RegisterCallback(huart, HAL_UART_TX_COMPLETE_CB_ID, Sensor_UART_TxComplete_Callback);
	  HAL_UART_RegisterCallback(huart, HAL_UART_ERROR_CB_ID, Sensor_UART_Error_Callback);
	  HAL_HalfDuplex_EnableReceiver(huart);	// Enable USART1 Receive
	  HAL_UART_Receive_DMA(huart, (uint8_t*) &Transiever_RX_Buffer[0], 1);
}

void Servo_UART_Telemetry_Init(UART_HandleTypeDef *huart) {
	  HAL_UART_RegisterCallback(huart, HAL_UART_RX_COMPLETE_CB_ID, Servo_UART_RxComplete_Callback);
	  HAL_UART_RegisterCallback(huart, HAL_UART_ERROR_CB_ID, Servo_UART_Error_Callback);
	  HAL_HalfDuplex_EnableReceiver(huart);	// Enable USART2 Receive
	  FLAGS.TELEMETRY_SYNC_STATES = TELEMETRY_SYNC_SYNC0;
	  HAL_UART_Receive_DMA(huart, (uint8_t*) &Transiever_TX_Buffer[0], 1);
}


void Sensor_UART_Error_Callback(struct __UART_HandleTypeDef *huart){
	__HAL_UART_CLEAR_PEFLAG(huart);
	HAL_UART_DMAStop(huart);
	HAL_UART_MspDeInit(huart);
	HAL_UART_MspInit(huart);

	printf("USART1 error %ld DMA RX/TX error 0x%lx-0x%lx \r\n", HAL_UART_GetError(huart), HAL_DMA_GetError(&hdma_usart1_rx), HAL_DMA_GetError(&hdma_usart1_tx));
	FLAGS.Transiever_RX_Sync = 0;
	while(HAL_UART_Receive_DMA(huart, (uint8_t*) &Transiever_RX_Buffer[0], 1) != HAL_OK);
}

void Servo_UART_Error_Callback(struct __UART_HandleTypeDef *huart){
	__HAL_UART_CLEAR_PEFLAG(huart);
	HAL_UART_DMAStop(huart);
	HAL_UART_MspDeInit(huart);
	HAL_UART_MspInit(huart);
	printf("USART2 error %ld DMA error 0x%lx \r\n", HAL_UART_GetError(huart), HAL_DMA_GetError(&hdma_usart2_rx));
	FLAGS.TELEMETRY_SYNC_STATES = TELEMETRY_SYNC_SYNC0;
	while(HAL_UART_Receive_DMA(huart, (uint8_t*) &Transiever_TX_Buffer[0], 1) != HAL_OK);
}

void Sensor_UART_RxComplete_Callback(struct __UART_HandleTypeDef *huart){
	uint16_t t, chksum = 0;

	if(!FLAGS.Transiever_RX_Sync) {
		FLAGS.Transiever_RX_Sync = Transiever_RX_Buffer[0] == SENSOR_HEADER;
		if(!FLAGS.Transiever_RX_Sync) {
			while(HAL_UART_Receive_DMA(&huart1, (uint8_t *) &Transiever_RX_Buffer[0], 1) != HAL_OK);
		} else {
			Transiever_RX_Buffer[0] = SENSOR_HEADER;
			while(HAL_UART_Receive_DMA(&huart1, (uint8_t *) &Transiever_RX_Buffer[1], 3) != HAL_OK);
		}
		return;
	} else {

		FLAGS.Transiever_RX_Sync = Transiever_RX_Buffer[0] == SENSOR_HEADER;
		if(!FLAGS.Transiever_RX_Sync) {
			while(HAL_UART_Receive_DMA(&huart1, (uint8_t *) &Transiever_RX_Buffer[0], 1) != HAL_OK);
			return;
		}

		uint8_t CHNL = (Transiever_RX_Buffer[1] & SENSOR_CHNL_MASK);
		uint8_t SIZE = 0;

		if((CHNL <= NUM_SENSORS) && (CHNL > 0)) {
			SIZE = (SensorList[CHNL-1].SensorType & 0x80) ? SENSOR_TX_BUF_SIZE_8 : SENSOR_TX_BUF_SIZE_6;
		} else {
			while(HAL_UART_Receive_DMA(&huart1, (uint8_t *) Transiever_RX_Buffer, SENSOR_BUFFER_SIZE) != HAL_OK);
			return;
		}

		switch(Transiever_RX_Buffer[1] & SENSOR_CMND_MASK) {
			case SENSOR_CMD_DSCVR:
				memcpy((uint8_t *) TX_Buffer, (uint8_t*) Transiever_RX_Buffer, SENSOR_BUFFER_SIZE);
				break;

			case SENSOR_CMD_TYPE:
				TX_Buffer[0] = SENSOR_TX_BUF_SIZE_6;
				chksum = TELM_CHECKSUM_CONST - SENSOR_TX_BUF_SIZE_6;
				TX_Buffer[1] = t = Transiever_RX_Buffer[1];
				chksum -= t;
				TX_Buffer[2] = t = SensorList[CHNL-1].SensorType;
				chksum -= t;
				TX_Buffer[3] = t = SIZE - 0x04;
				chksum -= t;
				TX_Buffer[4] = chksum;
				TX_Buffer[5] = chksum >> 8;
				break;

			case SENSOR_CMD_MEAS:
				TX_Buffer[0] = SIZE;
				chksum = TELM_CHECKSUM_CONST - SIZE;
				TX_Buffer[1] = t = Transiever_RX_Buffer[1];
				chksum -= t;
				TX_Buffer[2] = t = (uint8_t) (SensorList[CHNL-1].SensorMeas);
				chksum -= t;
				TX_Buffer[3] = t = (uint8_t) (SensorList[CHNL-1].SensorMeas >> 8);
				chksum -= t;
				if(SIZE == SENSOR_TX_BUF_SIZE_6) {
					TX_Buffer[4] = chksum;
					TX_Buffer[5] = chksum >> 8;
				} else {
					TX_Buffer[4] = t = (uint8_t) (SensorList[CHNL-1].SensorMeas >> 16);
					chksum -= t;
					TX_Buffer[5] = t = (uint8_t) (SensorList[CHNL-1].SensorMeas >> 24);
					chksum -= t;

					TX_Buffer[6] = chksum;
					TX_Buffer[7] = chksum >> 8;
				}
				break;

			default:
				while(HAL_UART_Receive_DMA(&huart1, (uint8_t *) Transiever_RX_Buffer, SENSOR_BUFFER_SIZE) != HAL_OK);
				return;
		}

			while(HAL_HalfDuplex_EnableTransmitter(&huart1) != HAL_OK);
			while(HAL_UART_Transmit_DMA(&huart1, (uint8_t *) TX_Buffer, TX_Buffer[0]) != HAL_OK);
			return;
	}
}

void Sensor_UART_TxComplete_Callback(struct __UART_HandleTypeDef *huart){
	while(HAL_HalfDuplex_EnableReceiver(&huart1) != HAL_OK);
	while(HAL_UART_Receive_DMA(&huart1, (uint8_t*) Transiever_RX_Buffer, SENSOR_BUFFER_SIZE) != HAL_OK);
}

void Servo_UART_RxComplete_Callback(struct __UART_HandleTypeDef *huart){
	uint16_t t, chksum = 0;

	switch (FLAGS.TELEMETRY_SYNC_STATES) {
		case TELEMETRY_SYNC_SYNC0:
			if(Transiever_TX_Buffer[0] == SERVO_HEADER_1) {
				FLAGS.TELEMETRY_SYNC_STATES = TELEMETRY_SYNC_SYNC1;
			}
			HAL_UART_Receive_DMA(&huart2, (uint8_t *) &Transiever_TX_Buffer[1], 1);
			break;

		case TELEMETRY_SYNC_SYNC1:
			if(Transiever_TX_Buffer[1] == SERVO_HEADER_2) {
				FLAGS.TELEMETRY_SYNC_STATES = TELEMETRY_SYNC_SYNCED;
				HAL_UART_Receive_DMA(&huart2, (uint8_t *) &Transiever_TX_Buffer[2], SERVO_BUFFER_SIZE-2);
			} else {
				FLAGS.TELEMETRY_SYNC_STATES = TELEMETRY_SYNC_SYNC0;
				HAL_UART_Receive_DMA(&huart2, (uint8_t *) &Transiever_TX_Buffer[0], 1);
			}
			break;

		case TELEMETRY_SYNC_VERIFIED:
			if((Transiever_TX_Buffer[0] != SERVO_HEADER_1) || (Transiever_TX_Buffer[1] != SERVO_HEADER_2)) {
				FLAGS.TELEMETRY_SYNC_STATES = TELEMETRY_SYNC_SYNC0;
				HAL_UART_Receive_DMA(&huart2, (uint8_t *) &Transiever_TX_Buffer[0], 1);
				break;
			}
		case TELEMETRY_SYNC_SYNCED:
			t = 0;
			for(uint8_t i = 0; i < SERVO_BUFFER_SIZE-2; i++)
				t += Transiever_TX_Buffer[i];

			t = TELM_CHECKSUM_CONST-t;
			chksum = (Transiever_TX_Buffer[31]<<8 | Transiever_TX_Buffer[30]);
			if(t == chksum) {
				memcpy(&ServoList, (uint8_t *) Transiever_TX_Buffer, SERVO_BUFFER_SIZE);
				FLAGS.FAIL_SAFE = (ServoList.Channel_11 > 1975);
				FLAGS.TELEMETRY_SYNC_STATES = TELEMETRY_SYNC_VERIFIED;
				HAL_UART_Receive_DMA(&huart2, (uint8_t *) &Transiever_TX_Buffer[0], SERVO_BUFFER_SIZE);
			} else {
				FLAGS.TELEMETRY_SYNC_STATES = TELEMETRY_SYNC_SYNC0;
				HAL_UART_Receive_DMA(&huart2, (uint8_t *) &Transiever_TX_Buffer[0], 1);
			}
			break;
	}
}
