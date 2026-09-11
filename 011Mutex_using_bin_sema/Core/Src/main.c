/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Demo of mutual exclusion using Mutex APIs
  *
  * Two instances of the same task (Print1, Print2) both write to the shared
  * UART. #define USE_MUTEX below to protect the critical section with a
  * FreeRTOS mutex (xSemaphoreCreateMutex). Comment it out to see the
  * unprotected/corrupted behaviour again for comparison.
  *
  * Unlike the earlier binary-semaphore demo, a real MUTEX is used here.
  * A mutex supports priority inheritance: if the low-priority task (Print1,
  * priority 1) is holding the mutex and the high-priority task (Print2,
  * priority 2) needs it, FreeRTOS temporarily raises Print1's priority so
  * it can finish and release the mutex quickly - avoiding priority
  * inversion. A plain binary semaphore does not do this.
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* MUTEX switch: comment this out to see the UNPROTECTED (corrupted) output. */
#define USE_MUTEX

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Dimensions the buffer into which messages destined for the UART are placed. */
#define mainMAX_MSG_LEN   ( 80 )
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* USER CODE BEGIN PV */
char usr_msg[250] = {0};

TaskHandle_t xTaskHandle1 = NULL;
TaskHandle_t xTaskHandle2 = NULL;

#ifdef USE_MUTEX
/* Declare a variable of type xSemaphoreHandle. This is used to reference the
mutex type semaphore that is used to ensure mutual exclusive access to UART. */
xSemaphoreHandle xMutex;
#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */
void printmsg(char *msg);
static void vprintmsg(char *msg);

/* The task to be created. Two instances of this task are created. */
static void prvPrintTask(void *pvParameters);

/* The function that uses a mutex to control access to the UART. */
static void prvNewPrintString(const char *pcString);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void printmsg(char *msg)
{
    vprintmsg(msg);
}

static void vprintmsg(char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/*-----------------------------------------------------------*/
static void prvNewPrintString(const char *pcString)
{
    static char cBuffer[mainMAX_MSG_LEN];

#ifdef USE_MUTEX
    /* The mutex is created before the scheduler is started so already
    exists by the time this task executes.

    Attempt to take the mutex, blocking indefinitely if it is not
    available immediately. The call to xSemaphoreTake() will only return
    once the mutex has been successfully obtained, so there is no need to
    check the return value here. */
    xSemaphoreTake(xMutex, portMAX_DELAY);
    {
#endif
        /* The following line will only execute once the mutex has been
        successfully obtained - so the UART can be accessed freely. */
        sprintf(cBuffer, "%s", pcString);
        printmsg(cBuffer);
#ifdef USE_MUTEX
    }
    xSemaphoreGive(xMutex);
#endif
}

/*-----------------------------------------------------------*/
static void prvPrintTask(void *pvParameters)
{
    char *pcStringToPrint;

    /* Two instances of this task are created so the string the task will
    send to prvNewPrintString() is passed in the task parameter. Cast this
    to the required type. */
    pcStringToPrint = (char *)pvParameters;

    for (;;)
    {
        /* Print out the string using the newly defined function. */
        prvNewPrintString(pcStringToPrint);

        /* Wait a pseudo random time. Note that rand() is not necessarily
        re-entrant, but in this case it does not really matter as the code
        does not care what value is returned. */
        vTaskDelay(rand() & 0XF);
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

    sprintf(usr_msg, "Demo of mutual exclusion using Mutex APIs\r\n");
    printmsg(usr_msg);

#ifdef USE_MUTEX
    /* Before a semaphore is used it must be explicitly created. In this
    example a mutex type semaphore is created. */
    xMutex = xSemaphoreCreateMutex();
#endif

    /* The tasks are going to use a pseudo random delay, seed the random
    number generator. */
    srand(567);

#ifdef USE_MUTEX
    /* Only create the tasks if the mutex was created successfully. */
    if (xMutex != NULL)
    {
#endif
        /* Create two instances of the task that attempt to write to the
        UART. The string they attempt to write is passed in as the task
        parameter. The tasks are created at DIFFERENT priorities so some
        pre-emption will occur. */
        xTaskCreate(prvPrintTask, "Print1", 240,
                    "Task 1 ******************************************\r\n",
                    1, &xTaskHandle1);

        xTaskCreate(prvPrintTask, "Print2", 240,
                    "Task 2 ------------------------------------------\r\n",
                    2, &xTaskHandle2);

        /* Start the scheduler so the created tasks start executing. */
        vTaskStartScheduler();
#ifdef USE_MUTEX
    }
    else
    {
        sprintf(usr_msg, "Mutex creation failed\r\n");
        printmsg(usr_msg);
    }
#endif

    /* If all is well we will never reach here as the scheduler will now be
    running the tasks. If we do reach here then it is likely that there was
    insufficient heap memory available for a resource to be created. */
    for (;;);

  /* USER CODE END 2 */

  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* USER CODE END RTOS_QUEUES */

  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* USER CODE END RTOS_EVENTS */

  osKernelStart();

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
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

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
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
  * @brief USART2 Initialization Function
  * @retval None
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
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

void StartDefaultTask(void *argument)
{
  for (;;)
  {
    osDelay(1);
  }
}

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
