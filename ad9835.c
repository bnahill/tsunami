/***************************************************************************
 *   Copyright (C) 2011 by Ben Nahill                                      *
 *   bnahill@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/*
 * This file provides basic functionality to interact with the AD9835
 * waveform generator using the dsPIC30F SPI module in conjunction with
 * FreeRTOS.
 */

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "ad9835.h"

// Static prototypes
static void vSPIInit( void );
static void vSPITask( void *pbParameters );

// Write the current value as well as a deferred value
#define AD_F_WRITE    (0b10 << 12)
// Write to the defer register
#define AD_F_DEFER    (0b11 << 12)
// MSB H,L; LSB H,L
#define AD_F_ADDR_MH  (0b11 << 8)
#define AD_F_ADDR_ML  (0b10 << 8)
#define AD_F_ADDR_LH  (0b01 << 8)
#define AD_F_ADDR_LL  (0b00 << 8)

// Constants
#define SPI_QUEUE_LEN         20
#define SPI_TASK_STACK_SIZE   ( configMINIMAL_STACK_SIZE + 128 )
#define SPI_TASK_PRIORITY     ( tskIDLE_PRIORITY + 3 )

// Static globals
static xQueueHandle xSPIQueue;
static xSemaphoreHandle xSPISemaphore;

/*
 * void ad9835_init (void)
 *
 * Initialize SPI, a queue for SPI words, and a semaphore
 * for the SPI resource. Start its task too.
 */
void ad9835_init( void ){
	xSPIQueue = xQueueCreate(SPI_QUEUE_LEN, sizeof(uint16_t));
	vSemaphoreCreateBinary( xSPISemaphore );

	vSPIInit();

	// Provide the initial resource
	xSemaphoreGive( xSPISemaphore );
	xTaskCreate( vSPITask, ( signed char * ) "SPI", SPI_TASK_STACK_SIZE, NULL, SPI_TASK_PRIORITY, NULL );
}

/*
 * void vSPIInit (void)
 *
 * Initialize the SPI module
 */
static void vSPIInit( void ){
	SPI1STAT = 0;
	//FRMEN = 1
	//MODE16 = 1
	//SMP = 0
	//CKE = 1
	//SSEN = 0 (not used)
	//CKP = 1 (idle high)
	//MASTEN = 1
	//SPRE = 110 (2)
	//PPRE = 10 (4)
	SPI1CON =  0b0100010101111010;
	IFS0bits.SPI1IF = 0;
}

/* 
 * SPI ISR:
 * Produces SPI semaphore
 * Consumes SPI queue (since it may pull from the queue more quickly than
 * the task) This could be a problem since it may prevent FSYNC from framing
 * a single word.
 *
 */
static void __attribute__((__interrupt__, auto_psv)) _SPI1Interrupt( void ){
	portBASE_TYPE taskWoken = pdFALSE;
	uint16_t buf;
	IFS0bits.SPI1IF = 0;
	// Is there more data to be transmitted?
	if(xQueueReceiveFromISR(xSPIQueue, &buf, &taskWoken) == pdTRUE){
		// Send it
		SPI1BUF = buf;

		// A task may be woken if the queue is full
	} else {
		// Otherwise make the SPI resource available
		xSemaphoreGiveFromISR(xSPISemaphore, &taskWoken);
	}

	// Switch tasks if needed
	if( taskWoken != pdFALSE ){
		taskYIELD();
	}
}


void ad9835_set_state( ad9835_state state ){
	uint16_t buf = (uint16_t) state | 0xC000;
	xQueueSend(xSPIQueue, &buf, portMAX_DELAY);
}

void vSPITask( void *pbParameters ){
	uint16_t buf;
	SPI1STATbits.SPIEN = 1;
	// Enable interrupt
	IEC0bits.SPI1IE = 1;

	while(1){
		// Block on SPI resource (only SPI consumer, can hold this resource)
		if(xSemaphoreTake(xSPISemaphore, portMAX_DELAY) == pdTRUE){
			// Block on SPI queue
			if(xQueueReceive(xSPIQueue, &buf, portMAX_DELAY) == pdTRUE){
				SPI1BUF = buf;
			}
		}
	}
}

/*
 * void ad9835_set_frequency ( freq )
 *
 * Set the FREQ0 register of the AD9835
 *
 * Each byte is sent in a two-stage process where the first byte is defered.
 * The second byte then triggers the actual device register to be loaded with
 * both bytes.
 */
void ad9835_set_frequency( frequency freq ){
	uint16_t word;

	word = AD_F_DEFER | AD_F_ADDR_MH | freq.bytes.msb_h;
	// Use blocking queue operations
	xQueueSend( xSPIQueue, &word, portMAX_DELAY );
	
	word = AD_F_WRITE | AD_F_ADDR_ML | freq.bytes.msb_l;
	xQueueSend( xSPIQueue, &word, portMAX_DELAY );
	
	word = AD_F_DEFER | AD_F_ADDR_LH | freq.bytes.lsb_h;
	xQueueSend( xSPIQueue, &word, portMAX_DELAY );
	
	word = AD_F_WRITE | AD_F_ADDR_LL | freq.bytes.lsb_l;
	xQueueSend( xSPIQueue, &word, portMAX_DELAY );
}
