/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "motor_pwm.h"

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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static uint8_t MotorTask_KeyScan(void);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
  int32_t duty = 0;
  uint8_t key;

  /* Infinite loop */
  for(;;)
  {
    key = MotorTask_KeyScan();

    if (key == 1U)
    {
      duty += 100;
      if (duty > (int32_t)(MOTOR_PWM_DUTY_MAX / 2U))
      {
        duty = (int32_t)(MOTOR_PWM_DUTY_MAX / 2U);
      }

      if (duty >= 0)
      {
        MotorPwm_SetControl(MOTOR_PWM_DIR_CW, (uint32_t)duty);
      }
      else
      {
        MotorPwm_SetControl(MOTOR_PWM_DIR_CCW, (uint32_t)(-duty));
      }
    }
    else if (key == 2U)
    {
      duty -= 100;
      if (duty < -(int32_t)(MOTOR_PWM_DUTY_MAX / 2U))
      {
        duty = -(int32_t)(MOTOR_PWM_DUTY_MAX / 2U);
      }

      if (duty >= 0)
      {
        MotorPwm_SetControl(MOTOR_PWM_DIR_CW, (uint32_t)duty);
      }
      else
      {
        MotorPwm_SetControl(MOTOR_PWM_DIR_CCW, (uint32_t)(-duty));
      }
    }
    else if (key == 3U)
    {
      duty = 0;
      MotorPwm_SetControl(MOTOR_PWM_DIR_CCW, 0U);
    }

    osDelay(10);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

static uint8_t MotorTask_KeyScan(void)
{
  static GPIO_PinState last_key0 = GPIO_PIN_SET;
  static GPIO_PinState last_key1 = GPIO_PIN_SET;
  static GPIO_PinState last_key2 = GPIO_PIN_SET;
  GPIO_PinState key0 = HAL_GPIO_ReadPin(Key_0_GPIO_Port, Key_0_Pin);
  GPIO_PinState key1 = HAL_GPIO_ReadPin(Key_1_GPIO_Port, Key_1_Pin);
  GPIO_PinState key2 = HAL_GPIO_ReadPin(Key_2_GPIO_Port, Key_2_Pin);
  uint8_t key = 0U;

  if ((last_key0 == GPIO_PIN_SET) && (key0 == GPIO_PIN_RESET))
  {
    key = 1U;
  }
  else if ((last_key1 == GPIO_PIN_SET) && (key1 == GPIO_PIN_RESET))
  {
    key = 2U;
  }
  else if ((last_key2 == GPIO_PIN_SET) && (key2 == GPIO_PIN_RESET))
  {
    key = 3U;
  }

  last_key0 = key0;
  last_key1 = key1;
  last_key2 = key2;

  return key;
}

/* USER CODE END Application */

