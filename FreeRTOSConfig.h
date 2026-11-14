/* FreeRTOSConfig.h - native FreeRTOS for STM32F103C8 (Cortex-M3, 72 MHz) */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>
extern uint32_t SystemCoreClock;

/* ---- Scheduler ---- */
#define configUSE_PREEMPTION 1
#define configCPU_CLOCK_HZ ( SystemCoreClock )
#define configTICK_RATE_HZ ( ( TickType_t ) 1000 ) /* tick = 1 ms */
#define configMAX_PRIORITIES ( 5 )
#define configMINIMAL_STACK_SIZE ( ( uint16_t ) 128 ) /* in words */
#define configTOTAL_HEAP_SIZE ( ( size_t ) ( 8 * 1024 ) )
#define configMAX_TASK_NAME_LEN ( 16 )
#define configUSE_16_BIT_TICKS 0
#define configIDLE_SHOULD_YIELD 1
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configSUPPORT_STATIC_ALLOCATION 0

/* ---- Synchronization (mutex, queues) ---- */
#define configUSE_MUTEXES 1
#define configUSE_COUNTING_SEMAPHORES 0

/* ---- Software timers (needed by xEventGroupSetBitsFromISR) ---- */
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY ( 2 )
#define configTIMER_QUEUE_LENGTH 5
#define configTIMER_TASK_STACK_DEPTH ( configMINIMAL_STACK_SIZE * 2 )

/* ---- Optional API functions ---- */
#define INCLUDE_vTaskDelay 1
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_xTimerPendFunctionCall 1 /* required by xEventGroupSetBitsFromISR */

/* ---- Interrupt priorities (STM32F1 has 4 priority bits) ---- */
#define configPRIO_BITS 4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )

/* ---- Assert ---- */
#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/* ---- Map FreeRTOS handlers to the CMSIS vector table names ---- */
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
