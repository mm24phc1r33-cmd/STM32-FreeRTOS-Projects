/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#include "task_handler.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
TaskHandle_t handle_cmd_task;
TaskHandle_t handle_menu_task;
TaskHandle_t handle_print_task;
TaskHandle_t handle_led_task;
TaskHandle_t handle_rtc_task;

BaseType_t status;

QueueHandle_t q_data;
QueueHandle_t q_print;

// Software timer handles
TimerHandle_t handle_led_timer[4];

volatile uint8_t user_data;

// State variable
state_t curr_state = sMainMenu;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
void menu_task(void *parameters);
void cmd_handler_task(void *parameters);
void print_task(void *parameters);
void led_task(void *parameters);
void rtc_task(void *parameters);

void led_effect_callback(TimerHandle_t xTimer);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */

  // Hardware LED Blink Test on Boot
  HAL_GPIO_WritePin(GPIOB, LED_GREEN_Pin|LED_ORANGE_Pin|LED_RED_Pin|LED_YELLOW_Pin, GPIO_PIN_SET);
  HAL_Delay(500);
  HAL_GPIO_WritePin(GPIOB, LED_GREEN_Pin|LED_ORANGE_Pin|LED_RED_Pin|LED_YELLOW_Pin, GPIO_PIN_RESET);

  // Stack sizes set to 500 words to prevent HardFault / Stack Overflow
  status = xTaskCreate(menu_task, "menu_task", 500, NULL, 2, &handle_menu_task);
  configASSERT(status == pdPASS);

  status = xTaskCreate(cmd_handler_task, "cmd_task", 500, NULL, 2, &handle_cmd_task);
  configASSERT(status == pdPASS);

  status = xTaskCreate(print_task, "print_task", 500, NULL, 2, &handle_print_task);
  configASSERT(status == pdPASS);

  status = xTaskCreate(led_task, "led_task", 500, NULL, 2, &handle_led_task);
  configASSERT(status == pdPASS);

  status = xTaskCreate(rtc_task, "rtc_task", 500, NULL, 2, &handle_rtc_task);
  configASSERT(status == pdPASS);

  // Queue to store incoming char data from UART Interrupt
  q_data = xQueueCreate(10, sizeof(char));
  configASSERT(q_data != NULL);

  // Queue stores char pointers (char*), which are 4 bytes each
  q_print = xQueueCreate(10, sizeof(size_t));
  configASSERT(q_print != NULL);

  // Create software timers for LED effects
  for(int i = 0; i < 4; i++)
  {
      handle_led_timer[i] = xTimerCreate(
          "LED_TIMER",
          pdMS_TO_TICKS(500),
          pdTRUE,
          (void *)(uintptr_t)(i + 1),
          led_effect_callback
      );

      configASSERT(handle_led_timer[i] != NULL);
  }

  // Start UART reception in Interrupt mode
  HAL_UART_Receive_IT(&huart2, (uint8_t *)&user_data, 1);

  char test[] = "UART TEST\r\n";

  HAL_UART_Transmit(&huart2,
                    (uint8_t *)test,
                    strlen(test),
                    HAL_MAX_DELAY);

  // Start Native FreeRTOS Scheduler
  vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Infinite loop */
  while (1)
  {
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  */
static void MX_RTC_Init(void)
{
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOB, LED_GREEN_Pin|LED_ORANGE_Pin|LED_RED_Pin|LED_YELLOW_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = LED_GREEN_Pin|LED_ORANGE_Pin|LED_RED_Pin|LED_YELLOW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

/* Function executes from UART interrupt handler */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t dummy;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(!xQueueIsQueueFullFromISR(q_data))
    {
        /* Enqueue data byte */
        xQueueSendFromISR(q_data, (void *)&user_data, &xHigherPriorityTaskWoken);
    }
    else
    {
        if(user_data == '\n')
        {
            /* Make sure that last data byte of the queue is '\n' */
            xQueueReceiveFromISR(q_data, (void *)&dummy, &xHigherPriorityTaskWoken);
            xQueueSendFromISR(q_data, (void *)&user_data, &xHigherPriorityTaskWoken);
        }
    }

    /* Send notification to command handling task if user_data == '\n' */
    if(user_data == '\n')
    {
        xTaskNotifyFromISR(handle_cmd_task, 0, eNoAction, &xHigherPriorityTaskWoken);
    }

    /* Re-enable UART RX in IT mode */
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&user_data, 1);

    /* Context switch if a higher priority task was woken */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* USER CODE END 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM5)
  {
    HAL_IncTick();
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
#endif /* USE_FULL_ASSERT */
