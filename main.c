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
 *
 * This program is based on the demo application for the dsPIC33F packaged
 * with FreeRTOS 6.1.0. The port has been modified to work with the dsPIC30F
 * series Digital Signal Controller.
 *
 */

/* Standard includes. */
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "semphr.h"

/* Demo application includes. */
#include "BlockQ.h"
#include "crflash.h"
#include "blocktim.h"
#include "integer.h"
#include "comtest2.h"
#include "partest.h"
#include "lcd.h"
#include "timertest.h"
#include "ad9835.h"

/* Demo task priorities. */
#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define mainCOM_TEST_PRIORITY				( 2 )

/* The check task may require a bit more stack as it calls sprintf(). */
#define mainCHECK_TASK_STACK_SIZE			( configMINIMAL_STACK_SIZE * 2 )

/* The execution period of the check task. */
#define mainCHECK_TASK_PERIOD				( ( portTickType ) 3000 / portTICK_RATE_MS )

/* The frequency at which the "fast interrupt test" interrupt will occur. */
#define mainTEST_INTERRUPT_FREQUENCY		( 20000 )

/* The number of processor clocks we expect to occur between each "fast
interrupt test" interrupt. */
#define mainEXPECTED_CLOCKS_BETWEEN_INTERRUPTS ( configCPU_CLOCK_HZ / mainTEST_INTERRUPT_FREQUENCY )

/* The number of nano seconds between each processor clock. */
#define mainNS_PER_CLOCK ( ( unsigned short ) ( ( 1.0 / ( double ) configCPU_CLOCK_HZ ) * 1000000000.0 ) )

/* Dimension the buffer used to hold the value of the maximum jitter time when
it is converted to a string. */
#define mainMAX_STRING_LENGTH				( 20 )

_FOSC( FRC_PLL16 & CSW_FSCM_OFF )
//_FOSC(CSW_FSCM_OFF & FRC & FRC_PLL4);
_FWDT( WDTPSB_16 & WDTPSA_512 & WDT_OFF )
_FBORPOR( PWRT_16 & BORV_42 & PBOR_OFF & MCLR_EN )
_FGS( CODE_PROT_OFF )

/*-----------------------------------------------------------*/

static void vBoringTask( void *pvParameters );

/*
 * Setup the processor ready for the demo.
 */
static void prvSetupHardware( void );

/*-----------------------------------------------------------*/

/* The queue used to send messages to the LCD task. */
static xQueueHandle xLCDQueue;

/*-----------------------------------------------------------*/

/*
 * Create the demo tasks then start the scheduler.
 */
int main( void )
{
	/* Configure any hardware required for this demo. */
	prvSetupHardware();

	ad9835_init();
	//xTaskCreate( vSPITask, ( signed char * ) "SPI", SPI_TASK_STACK_SIZE, NULL, SPI_TASK_PRIORITY, NULL ); 
	/* Create the standard demo tasks. */
	//vStartBlockingQueueTasks( mainBLOCK_Q_PRIORITY );	
	//vStartIntegerMathTasks( tskIDLE_PRIORITY );
	//vStartFlashCoRoutines( mainNUM_FLASH_COROUTINES );
	//vAltStartComTestTasks( mainCOM_TEST_PRIORITY, mainCOM_TEST_BAUD_RATE, mainCOM_TEST_LED );
	//vCreateBlockTimeTasks();

	/* Create the test tasks defined within this file. */
	xTaskCreate( vBoringTask, ( signed char * ) "Boring", mainCHECK_TASK_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL );

	/* Start the task that will control the LCD.  This returns the handle
	to the queue used to write text out to the task. */
//	xLCDQueue = xStartLCDTask();

	/* Finally start the scheduler. */
	vTaskStartScheduler();

	/* Will only reach here if there is insufficient heap available to start
	the scheduler. */
	return 0;
}
/*-----------------------------------------------------------*/


static void prvSetupHardware( void ){
	TRISB &= ~0x0001;
}
/*-----------------------------------------------------------*/


static void vBoringTask( void *pvParameters )
{
	/* Used to wake the task at the correct frequency. */
	portTickType xLastExecutionTime; 

	/* Buffer into which the maximum jitter time is written as a string. */
	static char cStringBuffer[ mainMAX_STRING_LENGTH ];

	/* The message that is sent on the queue to the LCD task.  The first
	parameter is the minimum time (in ticks) that the message should be
	left on the LCD without being overwritten.  The second parameter is a pointer
	to the message to display itself. */
	xLCDMessage xMessage = { 0, cStringBuffer };

	/* Initialise xLastExecutionTime so the first call to vTaskDelayUntil()
	works correctly. */
	xLastExecutionTime = xTaskGetTickCount();

	ad9835_set_state( AD_ON );

	for( ;; )
	{
		LATB ^= 0x0001;
		/* Wait until it is time for the next cycle. */
		vTaskDelay(( ( portTickType ) 10 / portTICK_RATE_MS ));
		//vTaskDelayUntil( &xLastExecutionTime, mainCHECK_TASK_PERIOD );
		/* Send the message to the LCD gatekeeper for display. */
		//xQueueSend( xLCDQueue, &xMessage, portMAX_DELAY );
	}
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* Schedule the co-routines from within the idle task hook. */
	vCoRoutineSchedule();
}
/*-----------------------------------------------------------*/

