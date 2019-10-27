/*
 * UART_DMA.c
 *
 *  Created on: 27.10.2019
 *     Author: Mateusz Salamon
 *		 www.msalamon.pl
 *
 *      Website:
 *      GitHub:
 *      Contact: mateusz@msalamon.pl
 */

#include "UART_DMA.h"
#include "string.h"

#define DMA_RX_BUFFER_SIZE          64
extern uint8_t DMA_RX_Buffer[DMA_RX_BUFFER_SIZE];

#define UART_BUFFER_SIZE            256
extern uint8_t UART_Buffer[UART_BUFFER_SIZE];

void UARTDMA_UartIrqHandler(UART_HandleTypeDef *huart)
{
	if(huart->Instance->SR & UART_FLAG_IDLE)       // Check if Idle flag is set
	{
		volatile uint32_t tmp;
		tmp = huart->Instance->SR;                      // Read status register
		tmp = huart->Instance->DR;                      // Read data register
		huart->hdmarx->Instance->CR &= ~DMA_SxCR_EN; // Disable DMA - it will force Transfer Complete interrupt if it's enabled
		tmp = tmp; // For unused warning
	}
}

void UARTDMA_DmaIrqHandler(DMA_HandleTypeDef *hdma)
{
	uint8_t* UartBufferPointer;
	static uint32_t Write;
	uint32_t Length, BytesToCopy;
	typedef struct
	{
		__IO uint32_t ISR;   // DMA interrupt status register
		__IO uint32_t Reserved0;
		__IO uint32_t IFCR;  // DMA interrupt flag clear register
	} DMA_Base_Registers;

	DMA_Base_Registers *DmaRegisters = (DMA_Base_Registers *) hdma->StreamBaseAddress; // Take registers base address

	if (__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_TC) != RESET) // Check if interrupt source is Transfer Complete
	{
		DmaRegisters->IFCR = DMA_FLAG_TCIF1_5 << hdma->StreamIndex;	// Clear Transfer Complete flag

		Length = DMA_RX_BUFFER_SIZE - hdma->Instance->NDTR; // Get the Lengthgth of transfered data

		BytesToCopy = UART_BUFFER_SIZE - Write; // Get number of bytes we can copy to the end of buffer

		if(BytesToCopy > Length) // Check how many copy
		{
			BytesToCopy = Length;
		}

		// Write received data for UART main buffer for manipulation later
		UartBufferPointer = DMA_RX_Buffer;
		memcpy(&UART_Buffer[Write], UartBufferPointer, BytesToCopy); // Copy first part of message

		// Remaining data
		Write += BytesToCopy;
		Length -= BytesToCopy;
		UartBufferPointer += BytesToCopy;

		// If still data to write for beginning of buffer - circular feature
		if(Length)
		{
			memcpy(&UART_Buffer[0], UartBufferPointer, Length); // Don't care if we override Read pointer now
			Write = Length;
		}

		// Prepare DMA for next transfer
		// Important! DMA stream won't start if all flags are not cleared first

		DmaRegisters->IFCR = 0x3FU << hdma->StreamIndex; 		// Clear all interrupts
		hdma->Instance->M0AR = (uint32_t) DMA_RX_Buffer; // Set memory address for DMA again
		hdma->Instance->NDTR = DMA_RX_BUFFER_SIZE; // Set number of bytes to receive
		hdma->Instance->CR |= DMA_SxCR_EN;            	// Start DMA transfer
	}
}

void UARTDMA_Init(UART_HandleTypeDef *huart)
{
	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);   	// UART Idle Line interrupt
	__HAL_DMA_ENABLE_IT(huart->hdmarx, DMA_IT_TC); // UART DMA Transfer Complete interrupt

	HAL_UART_Receive_DMA(huart, DMA_RX_Buffer, DMA_RX_BUFFER_SIZE); // Run DMA for whole DMA buffer

	huart->hdmarx->Instance->CR &= ~DMA_SxCR_HTIE; // Disable DMA Half Complete interrupt
}
