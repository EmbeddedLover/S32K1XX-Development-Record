/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
/* ###################################################################
**     Filename    : main.c
**     Project     : wdog_interrupt_s32k146
**     Processor   : S32K146_144
**     Version     : Driver 01.00
**     Compiler    : GNU C Compiler
**     Date/Time   : 2017-07-14, 14:08, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.00
** @brief
**         Main module.
**         This module contains user's application code.
*/
/*!
**  @addtogroup main_module main module documentation
**  @{
*/
/* MODULE main */


/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "clockMan1.h"
#include "pin_mux.h"
#include "watchdog1.h"
#include "pwrMan1.h"
#if CPU_INIT_CONFIG
  #include "Init_Config.h"
#endif

volatile int exit_code = 0;
/* User includes (#include below this line is not maintained by Processor Expert) */

#include <stdint.h>
#include <stdbool.h>

/* To use with other board than EVB please comment the following line */

#define EVB

#ifdef EVB
    #define LED0            15U /* pin PTD15 - LED RED of LED RGB on DEV-KIT 	*/
    #define LED1            16U /* pin PTD16 - LED GREEN of LED RGB on DEV-KIT 	*/
    #define LED_GPIO        PTD /* LED GPIO type */
#else
    #define LED0             0U /* pin PTC0 - LED0 on Motherboard */
    #define LED1             1U /* pin PTC1 - LED1 on Motherboard */
    #define LED_GPIO        PTC /* LED GPIO type */
#endif

/* Initialize SysTick counter */
void SysTick_Init(void)
{
	S32_SysTick->RVR = 0xFFFFFFul;
	S32_SysTick->CVR = 0ul;
	S32_SysTick->CSR = 0u;
}

/* Enable SysTick counter and interrupt */
void SysTick_Enable(void)
{
	S32_SysTick->CSR = S32_SysTick_CSR_TICKINT(1u) | S32_SysTick_CSR_ENABLE(1);
}

/* Disable SysTick */
void SysTick_Disable(void)
{
	S32_SysTick->CSR = 0ul;
}

/* SysTick IRQ handler */
void SysTick_Handler(void)
{
	/* Variable that stores the number of Watchdog triggers */
	static uint8_t s_refreshNumber = 0;
	/* After 8 watchdog refreshes, the interrupt will no longer reset the wdog counter */
	if(s_refreshNumber < 8)
	{
		/* Reset Watchdog counter */
		WDOG_DRV_Trigger(INST_WATCHDOG1);
		/* Toggle LED0 */
		PINS_DRV_TogglePins(LED_GPIO, (1 << LED0));
		/* Increment refresh number */
		s_refreshNumber++;
	}
}

/* WatchDog IRQ handler */
void WDOG_ISR(void)
{
	/* Turn off both LEDs */
    PINS_DRV_ClearPins(LED_GPIO, (1 << LED0) | (1 << LED1));
	/* Disable the SysTick timer */
	SysTick_Disable();
}

/*!
  \brief The main function for the project.
  \details The startup initialization sequence is the following:
 * - __start (startup asm routine)
 * - __init_hardware()
 * - main()
 *   - PE_low_level_init()
 *     - Common_Init()
 *     - Peripherals_Init()
*/
int main(void)
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  #ifdef PEX_RTOS_INIT
    PEX_RTOS_INIT();                 /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of Processor Expert internal initialization.                    ***/

    /* Initialize and configure clocks
     *  -   Setup system clocks
     *  -   Enable clock feed for Ports
     *  -   See Clock Manager component for more info
     */
    CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
                        g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);


    /* Initialize pins
     *  -   Setup output pins for LEDs
     *  -   See PinSettings component for more info
     */
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);

    /* Stop if the previous reset was caused by the Watchdog */
    if(POWER_SYS_GetResetSrcStatusCmd(RCM, RCM_WATCH_DOG) == true)
    {
    	/* Turn on both LEDs */
        PINS_DRV_WritePin(LED_GPIO, LED0, 1u);
        PINS_DRV_WritePin(LED_GPIO, LED1, 1u);
    }
    else
    {
    	/* Turn off both LEDs */
		PINS_DRV_WritePin(LED_GPIO, LED0, 0u);
		PINS_DRV_WritePin(LED_GPIO, LED1, 0u);

		/* Install IRQ handlers for WDOG and SysTick interrupts */
		INT_SYS_InstallHandler(WDOG_EWM_IRQn, WDOG_ISR, (isr_t *)0);
		INT_SYS_InstallHandler(SysTick_IRQn, SysTick_Handler, (isr_t *)0);

		/* Enable Watchdog IRQ */
		INT_SYS_EnableIRQ(WDOG_EWM_IRQn);

		/* Configure and enable SysTick */
		SysTick_Init();
		SysTick_Enable();

		/* Initialize WDOG */
		WDOG_DRV_Init(INST_WATCHDOG1, &watchdog1_Config0);
    }
    while(1)
    {
    	;
    }
  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;) {
    if(exit_code != 0) {
      break;
    }
  }
  return exit_code;
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.1 [05.21]
**     for the Freescale S32K series of microcontrollers.
**
** ###################################################################
*/

