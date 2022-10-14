# Project Components
- STM32 Microcontroller
- FlySky FS - i6x (OpenI6x Software)
- FS-iA#B AFHDS2A (# : 6 or 10)

# Features
-	Can receive 14 Channels of the FlySky FS - i6x
-	Can send up to 14 sensor measurements 
- 	Only iBus ports are used, minimazing the pin usage
- 	Each Servo and Sensor iBus requires one U(S)ART peripheral (Half-Dublex)
- 	Every transmission and reception is handled by state machine guided DMA operations, minimazing CPU load

# Example Project

SMT32F103C8T6 Based Example Project
 - 14 Channel Servo Reception (DMA)
 - 14 Channel Sensor Transmission (DMA)
 - USART1 (iBus - Sensor) (Half-Dublex)
 - USART2 (iBus - Servo) (Half-Dublex)
 - Prints Servo Channels over SWO ITM
 
## SystemCore

### RCC
- HSE -> Crystal / Ceramic Resonator

### SYS
- Debug -> Trace Asynchronous SW
- Timebase Source -> SysTick

## Connectivity

### USART1 (iBus - Sensor)
- Mode -> Single Wire (Half-Dublex) (Transfer and Receive)
- BoudRate -> 115200 (8N1)
- DMA_RX -> Enabled
- DMA_TX -> Enabled
- NVIC -> USART1 Global Interrupt Enabled

### USART2 (iBus - Servo)
- Mode -> Single Wire (Half-Dublex) (Only Receive)
- BoudRate -> 115200 (8N1)
- DMA_RX -> Enabled

## Project Manager -> Advanced Settings
- Register CallBacks -> UART -> Enabled

# Step-by-step Implimentation

1. [OpenI6x Software for FlySky - i6x](https://github.com/OpenI6X/opentx)
2. Configure Microcontroller Peripherals according to Example Project
3. Copy 'Telemetry.c/.h' Files to Inc and Src Folders
4. Add '\*\*\*_UART_Telemetry_Init(&huart)' Functions in 'USER_CODE_2' Field
5. Add Global 'TelemetrySensorStruct SensorList\[NUM_SENSOR\]' Array According to [iBus Sensor Types](https://github.com/betaflight/betaflight/blob/master/src/main/telemetry/ibus_shared.h)
6. Edit 'NUM_SENSOR' @ 'Telemetry.h' File 
