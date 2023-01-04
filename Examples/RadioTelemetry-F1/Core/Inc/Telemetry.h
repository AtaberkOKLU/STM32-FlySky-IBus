#ifndef __TELEMETRY_H
#define __TELEMETRY_H

#ifdef __cplusplus
extern "C" {
#endif


#include <string.h>
#include <stdio.h>

#define NUM_SENSORS 15

#define USE_HAL_USE_HAL_UART_REGISTER_CALLBACKS 1U

#define SERVO_HEADER_1 		0x20
#define SERVO_HEADER_2 		0x40
#define SERVO_BUFFER_SIZE 	0x20

#define SENSOR_HEADER  		0x04
#define SENSOR_BUFFER_SIZE	0x04
#define SENSOR_TX_BUF_SIZE_6  0x06
#define SENSOR_TX_BUF_SIZE_8  0x08
#define SENSOR_CHNL_MASK 	0x0F
#define SENSOR_CMND_MASK 	0xF0
#define SENSOR_CMD_DSCVR 	0x80
#define SENSOR_CMD_TYPE 	0x90
#define SENSOR_CMD_MEAS 	0xA0
#define TELM_CHECKSUM_CONST 0xFFFF



typedef struct TelemetrySensorStruct {
	uint8_t 			SensorType;
	volatile int32_t	SensorMeas;
} TelemetrySensorStruct;

typedef struct TelemetryServoStruct {
	uint16_t 	Header;
	uint16_t 	Channel_1;
	uint16_t 	Channel_2;
	uint16_t 	Channel_3;
	uint16_t 	Channel_4;
	uint16_t 	Channel_5;
	uint16_t 	Channel_6;
	uint16_t 	Channel_7;
	uint16_t 	Channel_8;
	uint16_t 	Channel_9;
	uint16_t 	Channel_10;
	uint16_t 	Channel_11;
	uint16_t 	Channel_12;
	uint16_t 	Channel_13;
	uint16_t 	Channel_14;
	uint16_t 	Checksum;
} TelemetryServoStruct;

struct __FLAGS {
	volatile uint8_t MOTOR_ARMING:1;
	volatile uint8_t FAIL_SAFE:1;
	volatile uint8_t Transiever_RX_Sync:1;
	volatile enum TELEMETRY_SYNC_STATES {
		TELEMETRY_SYNC_SYNC0,
		TELEMETRY_SYNC_SYNC1,
		TELEMETRY_SYNC_SYNCED,
		TELEMETRY_SYNC_VERIFIED} TELEMETRY_SYNC_STATES:2;
	volatile uint8_t UNUSED:3;
};

extern TelemetryServoStruct ServoList;
extern volatile struct __FLAGS FLAGS;

void Sensor_UART_Telemetry_Init(UART_HandleTypeDef *huart);
void Servo_UART_Telemetry_Init(UART_HandleTypeDef *huart);
void Sensor_UART_TxComplete_Callback(struct __UART_HandleTypeDef *huart);
void Sensor_UART_RxComplete_Callback(struct __UART_HandleTypeDef *huart);
void Servo_UART_RxComplete_Callback(struct __UART_HandleTypeDef *huart);
void Sensor_UART_Error_Callback(struct __UART_HandleTypeDef *huart);
void Servo_UART_Error_Callback(struct __UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif
#endif /* __TELEMETRY_H */
