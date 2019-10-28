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



void UARTDMA_UartIrqHandler(UARTDMA_HandleTypeDef *huartdma)
{
	if(huartdma->huart->Instance->SR & UART_FLAG_IDLE)       // Check if Idle flag is set
	{
		volatile uint32_t tmp;
		tmp = huartdma->huart->Instance->SR;                      // Read status register
		tmp = huartdma->huart->Instance->DR;                      // Read data register
		huartdma->huart->hdmarx->Instance->CR &= ~DMA_SxCR_EN; // Disable DMA - it will force Transfer Complete interrupt if it's enabled
		tmp = tmp; // For unused warning
	}
}

void UARTDMA_DmaIrqHandler(UARTDMA_HandleTypeDef *huartdma)
{
	uint8_t* UartBufferPointer;
	uint32_t Length, BytesToCopy;
	typedef struct
	{
		__IO uint32_t ISR;   // DMA interrupt status register
		__IO uint32_t Reserved0;
		__IO uint32_t IFCR;  // DMA interrupt flag clear register
	} DMA_Base_Registers;

	DMA_Base_Registers *DmaRegisters = (DMA_Base_Registers *) huartdma->huart->hdmarx->StreamBaseAddress; // Take registers base address

	if (__HAL_DMA_GET_IT_SOURCE(huartdma->huart->hdmarx, DMA_IT_TC) != RESET) // Check if interrupt source is Transfer Complete
	{
		DmaRegisters->IFCR = DMA_FLAG_TCIF1_5 << huartdma->huart->hdmarx->StreamIndex;	// Clear Transfer Complete flag

		Length = DMA_RX_BUFFER_SIZE - huartdma->huart->hdmarx->Instance->NDTR; // Get the Lengthgth of transfered data

		BytesToCopy = UART_BUFFER_SIZE - huartdma->WritePointer; // Get number of bytes we can copy to the end of buffer

		if(BytesToCopy > Length) // Check how many copy
		{
			BytesToCopy = Length;
		}

		// Write received data for UART main buffer for manipulation later
		UartBufferPointer = huartdma->DMA_RX_Buffer;
		memcpy(huartdma->UART_Buffer + huartdma->WritePointer, UartBufferPointer, BytesToCopy); // Copy first part of message

		// Remaining data
		huartdma->WritePointer += BytesToCopy;
		Length -= BytesToCopy;
		UartBufferPointer += BytesToCopy;

		// If still data to write for beginning of buffer - circular feature
		if(Length)
		{
			memcpy(huartdma->UART_Buffer, UartBufferPointer, Length); // Don't care if we override Read pointer now
			huartdma->WritePointer = Length;
		}

		// Prepare DMA for next transfer
		// Important! DMA stream won't start if all flags are not cleared first

		DmaRegisters->IFCR = 0x3FU << huartdma->huart->hdmarx->StreamIndex; 		// Clear all interrupts
		huartdma->huart->hdmarx->Instance->M0AR = (uint32_t) huartdma->DMA_RX_Buffer; // Set memory address for DMA again
		huartdma->huart->hdmarx->Instance->NDTR = DMA_RX_BUFFER_SIZE; // Set number of bytes to receive
		huartdma->huart->hdmarx->Instance->CR |= DMA_SxCR_EN;            	// Start DMA transfer
	}
}

void UARTDMA_Init(UARTDMA_HandleTypeDef *huartdma, UART_HandleTypeDef *huart)
{
	huartdma->huart = huart;

	__HAL_UART_ENABLE_IT(huartdma->huart, UART_IT_IDLE);   	// UART Idle Line interrupt
	__HAL_DMA_ENABLE_IT(huartdma->huart->hdmarx, DMA_IT_TC); // UART DMA Transfer Complete interrupt

	HAL_UART_Receive_DMA(huartdma->huart, huartdma->DMA_RX_Buffer, DMA_RX_BUFFER_SIZE); // Run DMA for whole DMA buffer

	huartdma->huart->hdmarx->Instance->CR &= ~DMA_SxCR_HTIE; // Disable DMA Half Complete interrupt
}
