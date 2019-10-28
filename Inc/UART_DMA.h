/*
 * UART_DMA.h
 *
 *  Created on: 27.10.2019
 *     Author: Mateusz Salamon
 *		 www.msalamon.pl
 *
 *      Website:
 *      GitHub:
 *      Contact: mateusz@msalamon.pl
*/
#include "stm32f4xx_hal.h"

#define DMA_RX_BUFFER_SIZE          64
#define UART_BUFFER_SIZE            256

typedef struct
{
	UART_HandleTypeDef* huart;					// UART handler

	uint8_t DMA_RX_Buffer[DMA_RX_BUFFER_SIZE];	// DMA direct buffer
	uint8_t UART_Buffer[UART_BUFFER_SIZE];		// UART working circular buffer

	uint32_t WritePointer;	// Next cell to write
}UARTDMA_HandleTypeDef;

void UARTDMA_UartIrqHandler(UARTDMA_HandleTypeDef *huartdma);
void UARTDMA_DmaIrqHandler(UARTDMA_HandleTypeDef *huartdma);

void UARTDMA_Init(UARTDMA_HandleTypeDef *huartdma, UART_HandleTypeDef *huart);
