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

void UARTDMA_UartIrqHandler(UART_HandleTypeDef *huart);
void UARTDMA_DmaIrqHandler(DMA_HandleTypeDef *hdma);

void UARTDMA_Init(UART_HandleTypeDef *huart);
