/* Copyright 2014, Mariano Cerdeiro
 * Copyright 2014, Pablo Ridolfi
 * Copyright 2014, Juan Cecconi
 * Copyright 2014, Gustavo Muro
 *
 * This file is part of CIAA Firmware.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/** \brief rtos example source file
 **
 ** This is a mini example of the CIAA Firmware which only uses the rtos.
 **
 **/

/** \addtogroup CIAA_Firmware CIAA Firmware
 ** @{ */
/** \addtogroup Examples CIAA Firmware Examples
 ** @{ */
/** \addtogroup RTOS rtos example source file
 ** @{ */

/*
 * Initials     Name
 * ---------------------------
 * MaCe         Mariano Cerdeiro
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20141221 v0.0.1 initials initial version
 */

/*==================[inclusions]=============================================*/
#include "ciaaIO.h"
#include "os.h"               /* <= operating system header */
#include "blinky_uart_waitevent.h"     /* <= own header */
#include "ciaaUART.h"

#include "ciaaPlatforms.h" // para saber qué board estamos usando



// better not toggle, in order to know if the bug before or after WaitEvent
#define	TOGGLE	1

#define	PAN	0
#define QUESO   1

 
/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

bool task3val = PAN;
bool task4val = PAN;


/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
/** \brief Main function
 *
 * This is the main entry point of the software.
 *
 * \returns 0
 *
 * \remarks This function never returns. Return value is only to avoid compiler
 *          warnings or errors.
 */
int main(void)
{
   /* Starts the operating system in the Application Mode 1 */
   /* This example has only one Application Mode */
   StartOS(AppMode1);

   /* StartOs shall never returns, but to avoid compiler warnings or errors
    * 0 is returned */
   return 0;
}

/** \brief Error Hook function
 *
 * This fucntion is called from the os if an os interface (API) returns an
 * error. Is for debugging proposes. If called this function triggers a
 * ShutdownOs which ends in a while(1).
 *
 * The values:
 *    OSErrorGetServiceId
 *    OSErrorGetParam1
 *    OSErrorGetParam2
 *    OSErrorGetParam3
 *    OSErrorGetRet
 *
 * will provide you the interface, the input parameters and the returned value.
 * For more details see the OSEK specification:
 * http://portal.osek-vdx.org/files/pdf/specs/os223.pdf
 *
 */
void ErrorHook(void)
{
   ShutdownOS(0);
}

/** \brief Initial task
 *
 * This task is started automatically in the application mode 1.
 */
TASK(InitTask)
{
   /* activate periodic task:
    *  - for the first time after 350 ticks (350 ms)
    *  - and then every 250 ticks (250 ms)
    */

   ciaaIOInit();
   ciaaUARTInit();
   SetRelAlarm(ActivateBlinkTask1, 350, 250);
   SetRelAlarm(ActivateBlinkTask2, 250, 50);
   ActivateTask(WaitTask3);
   ActivateTask(WaitTask4);

   /* terminate task */
   TerminateTask();
}

/** \brief Blink Task
 *
 * This task is started automatically every time that the alarm
 * ActivateBlinkTask1 expires.
 *
 */
TASK(BlinkTask1)
{
   /* add your cycle code here */

#if (edu_ciaa_nxp == BOARD)
   ciaaToggleOutput(LEDG);
#elif (ciaa_nxp == BOARD)
   ciaaToggleOutput(LED1);
#endif

   /* terminate task */
   TerminateTask();
}

TASK(BlinkTask2)
{
   /* add your cycle code here */
   ciaaToggleOutput(LEDB);
   /* terminate task */
   TerminateTask();
}

TASK(WaitTask3)
{
//   ciaaWriteOutput(LED2, 1);

   while(1) 
   {
      /* add your cycle code here */

#if TOGGLE
      WaitEvent(TESTW);
      ClearEvent(TESTW);
      task3val = PAN;
      ciaaToggleOutput(LED2);
#else
      ciaaWriteOutput(LED2, 1);
      WaitEvent(TESTW);
      ClearEvent(TESTW);
      task3val = PAN;
      ciaaWriteOutput(LED2, 0);
#endif
   }
   TerminateTask();
}


TASK(WaitTask4)
{

   while(1)
   {
      /* add your cycle code here */
#if TOGGLE
      WaitEvent(TESTW);
      ClearEvent(TESTW);
      task4val = PAN;
#else
      WaitEvent(TESTW);
      ClearEvent(TESTW);
      task4val = PAN;
#endif
   }
   TerminateTask();
}


// FTDI
ISR(UART2_IRQHandler)
{
   uint8_t status = Chip_UART_ReadLineStatus(LPC_USART2);
   ciaaWriteOutput(LED3, 1);


   if(status & UART_LSR_RDR)
   {
      do
      {
         Chip_UART_ReadByte(LPC_USART2);
      }while( Chip_UART_ReadLineStatus(LPC_USART2) & UART_LSR_RDR );

      /*
      ** ciaaDriverUart_rxIndication(&ciaaDriverUart_device1, uartControl[1].rxcnt);
      ** Siguendo el stack de rxIndication en esas condicioens hay un SetEvent
      ** para la tarea que pidio read y quedo detenida en WaitEvent()
      */
      if(task3val == PAN)
      {
         task3val = QUESO;
         SetEvent(WaitTask3, TESTW);
      }
   }
   ciaaWriteOutput(LED3, 0);
}

#if 1

// RS232 when available
ISR(UART3_IRQHandler)
{
   uint8_t status = Chip_UART_ReadLineStatus(LPC_USART3);


   if(status & UART_LSR_RDR)
   {
      do
      {
         Chip_UART_ReadByte(LPC_USART3);
      }while( Chip_UART_ReadLineStatus(LPC_USART3) & UART_LSR_RDR );

      /*
      ** ciaaDriverUart_rxIndication(&ciaaDriverUart_device1, uartControl[1].rxcnt);
      ** Siguendo el stack de rxIndication en esas condicioens hay un SetEvent
      ** para la tarea que pidio read y quedo detenida en WaitEvent()
      */
      if(task4val == PAN)
      {
         task4val = QUESO;
         SetEvent(WaitTask4, TESTW);
      }
   }
}
#endif


/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

