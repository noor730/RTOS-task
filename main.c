/* USER CODE BEGIN Header */
/**
  * @file   main.c
  * @brief  Native FreeRTOS task - STM32F103C8 (Blue Pill)
  * Switch: PA0 (EXTI falling) | UART: USART1 115200 (PA9 TX, PA10 RX)
  */
/* USER CODE END Header */

#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PTD */
typedef enum { SRC_COUNTER = 0, SRC_SHARED } DataSource_t;

typedef struct
{
  DataSource_t source;   /* who sent it: counter or shared */
  uint32_t     value;    /* the value to print */
} UARTData_t;
/* USER CODE END PTD */

/* USER CODE BEGIN PD */
#define EVT_SWITCH_PRESSED   ( 1UL << 0 )
#define DEBOUNCE_MS          200
#define TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )   /* same for all tasks */
/* USER CODE END PD */

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
volatile uint32_t counter = 0;
volatile uint32_t shared  = 100;

QueueHandle_t      UARTData;       /* queue of UARTData_t */
SemaphoreHandle_t  sharedMutex;    /* protects "shared" */
EventGroupHandle_t switchEvent;    /* set from the ISR */
/* USER CODE END PV */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */
void CalcSwitch1(void *pvParameters);
void IncreaseVar(void *pvParameters);
void DecreaseVar(void *pvParameters);
void UARTTask(void *pvParameters);
/* USER CODE END PFP */

int main(void)
{
  HAL_Init();                      // HAL #__
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */

  /* FreeRTOS: Kernel > API Reference > Queues > xQueueCreate */
  UARTData = xQueueCreate(20, sizeof(UARTData_t));

  /* FreeRTOS: Kernel > API Reference > Semaphores & Mutexes > xSemaphoreCreateMutex */
  sharedMutex = xSemaphoreCreateMutex();

  /* FreeRTOS: Kernel > API Reference > Event Groups > xEventGroupCreate */
  switchEvent = xEventGroupCreate();

  if ((UARTData == NULL) || (sharedMutex == NULL) || (switchEvent == NULL))
  {
    Error_Handler();
  }

  /* FreeRTOS: Kernel > API Reference > Task Creation > xTaskCreate */
  xTaskCreate(CalcSwitch1, "CalcSwitch1", 128, NULL, TASK_PRIORITY, NULL);
  xTaskCreate(IncreaseVar, "IncreaseVar", 128, NULL, TASK_PRIORITY, NULL);
  xTaskCreate(DecreaseVar, "DecreaseVar", 128, NULL, TASK_PRIORITY, NULL);
  xTaskCreate(UARTTask,    "UARTTask",    256, NULL, TASK_PRIORITY, NULL);

  /* FreeRTOS: Kernel > API Reference > Task Control > vTaskStartScheduler */
  vTaskStartScheduler();
  /* USER CODE END 2 */

  while (1)
  {
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)   // HAL #__
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)   // HAL #__
  {
    Error_Handler();
  }
}

static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)   // HAL #__
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);   // HAL #__

  /* Priority 5 (not 0): FromISR APIs need priority >= configMAX_SYSCALL_INTERRUPT_PRIORITY */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);   // HAL #__
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);           // HAL #__
}

/* USER CODE BEGIN 4 */

/* ISR callback: counter++ (reset after 9) and set the event bit */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)   // HAL #__
{
  static uint32_t lastPress = 0;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (GPIO_Pin == GPIO_PIN_0)
  {
    uint32_t now = HAL_GetTick();   // HAL #__
    if ((now - lastPress) < DEBOUNCE_MS)
    {
      return;
    }
    lastPress = now;

    counter++;
    if (counter > 9)
    {
      counter = 0;
    }

    /* FreeRTOS: Kernel > API Reference > Event Groups > xEventGroupSetBitsFromISR */
    xEventGroupSetBitsFromISR(switchEvent, EVT_SWITCH_PRESSED, &xHigherPriorityTaskWoken);

    /* FreeRTOS: Kernel > API Reference > Task Control > portYIELD_FROM_ISR */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

/* Task1: wait max 2 ticks for the event, then send counter*10 */
void CalcSwitch1(void *pvParameters)
{
  UARTData_t msg;
  EventBits_t bits;

  for (;;)
  {
    /* FreeRTOS: Kernel > API Reference > Event Groups > xEventGroupWaitBits */
    bits = xEventGroupWaitBits(switchEvent, EVT_SWITCH_PRESSED, pdTRUE, pdFALSE, 2);

    if (bits & EVT_SWITCH_PRESSED)
    {
      msg.source = SRC_COUNTER;
      msg.value  = counter * 10;   /* original counter unchanged */

      /* FreeRTOS: Kernel > API Reference > Queues > xQueueSend */
      xQueueSend(UARTData, &msg, 0);
    }
  }
}

/* Task2: every 10 ticks, shared++ (above 200 -> 100) */
void IncreaseVar(void *pvParameters)
{
  UARTData_t msg;

  for (;;)
  {
    /* FreeRTOS: Kernel > API Reference > Task Control > vTaskDelay */
    vTaskDelay(10);

    /* FreeRTOS: Kernel > API Reference > Semaphores & Mutexes > xSemaphoreTake */
    xSemaphoreTake(sharedMutex, portMAX_DELAY);
    shared++;
    if (shared > 200)
    {
      shared = 100;
    }
    msg.source = SRC_SHARED;
    msg.value  = shared;
    /* FreeRTOS: Kernel > API Reference > Semaphores & Mutexes > xSemaphoreGive */
    xSemaphoreGive(sharedMutex);

    xQueueSend(UARTData, &msg, 0);
  }
}

/* Task3: every 12 ticks, shared-- (below 100 -> 200) */
void DecreaseVar(void *pvParameters)
{
  UARTData_t msg;

  for (;;)
  {
    vTaskDelay(12);

    xSemaphoreTake(sharedMutex, portMAX_DELAY);
    shared--;
    if (shared < 100)
    {
      shared = 200;
    }
    msg.source = SRC_SHARED;
    msg.value  = shared;
    xSemaphoreGive(sharedMutex);

    xQueueSend(UARTData, &msg, 0);
  }
}

/* Task4: every 5 ticks, empty the queue and print on PuTTY */
void UARTTask(void *pvParameters)
{
  UARTData_t msg;
  char text[40];
  int len;

  for (;;)
  {
    vTaskDelay(5);

    /* FreeRTOS: Kernel > API Reference > Queues > xQueueReceive */
    while (xQueueReceive(UARTData, &msg, 0) == pdPASS)
    {
      if (msg.source == SRC_COUNTER)
      {
        len = snprintf(text, sizeof(text), "counter: %lu\r\n", (unsigned long)msg.value);
      }
      else
      {
        len = snprintf(text, sizeof(text), "shared: %lu\r\n", (unsigned long)msg.value);
      }
      HAL_UART_Transmit(&huart1, (uint8_t *)text, (uint16_t)len, 100);   // HAL #__
    }
  }
}

/* USER CODE END 4 */

/* TIM1 is the HAL timebase (SysTick belongs to FreeRTOS) */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)   // HAL #__
{
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();   // HAL #__
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
