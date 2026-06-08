/*
 * motor_pwm.c
 *
 * Adapted from ALIENTEK experiment 9-1 BLDC basic drive.
 * TIM1 CH1/2/3 generate high-side PWM, PB13/14/15 are used as low-side GPIOs.
 */

#include "motor_pwm.h"
#include "tim.h"

typedef void (*MotorPwm_CommutationFunc)(void);

typedef struct
{
  volatile MotorPwm_RunStateTypeDef run_state;
  volatile MotorPwm_DirectionTypeDef direction;
  volatile uint32_t duty;
  volatile uint32_t hall_state;
} MotorPwm_HandleTypeDef;

static MotorPwm_HandleTypeDef motor1 = {
  MOTOR_PWM_STOP,
  MOTOR_PWM_DIR_CCW,
  0U,
  0U
};

volatile uint32_t g_motor_pwm_debug_hall_raw = 0U;
volatile uint32_t g_motor_pwm_debug_hall_step = 0U;
volatile uint32_t g_motor_pwm_debug_run_state = 0U;
volatile uint32_t g_motor_pwm_debug_duty = 0U;
volatile uint32_t g_motor_pwm_debug_ccr1 = 0U;
volatile uint32_t g_motor_pwm_debug_ccr2 = 0U;
volatile uint32_t g_motor_pwm_debug_ccr3 = 0U;
volatile uint32_t g_motor_pwm_debug_sd = 0U;

static void MotorPwm_HallGpioInit(void);
static void MotorPwm_LowSideGpioInit(void);
static uint32_t MotorPwm_ClampCompare(uint32_t compare);
static void MotorPwm_UpdateDebug(void);
static void MotorPwm_AllBridgeOff(void);
static void MotorPwm_UhVl(void);
static void MotorPwm_UhWl(void);
static void MotorPwm_VhWl(void);
static void MotorPwm_VhUl(void);
static void MotorPwm_WhUl(void);
static void MotorPwm_WhVl(void);

static const MotorPwm_CommutationFunc commutation_table[6] = {
  MotorPwm_UhWl,
  MotorPwm_VhUl,
  MotorPwm_VhWl,
  MotorPwm_WhVl,
  MotorPwm_UhVl,
  MotorPwm_WhUl
};

static void MotorPwm_HallGpioInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
}

static void MotorPwm_LowSideGpioInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = PM1_PWM_UL_Pin | PM1_PWM_VL_Pin | PM1_PWM_WL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOB, PM1_PWM_UL_Pin | PM1_PWM_VL_Pin | PM1_PWM_WL_Pin, GPIO_PIN_RESET);
}


static void MotorPwm_UpdateDebug(void)
{
  g_motor_pwm_debug_hall_raw = MotorPwm_GetHallState();
  g_motor_pwm_debug_hall_step = motor1.hall_state;
  g_motor_pwm_debug_run_state = (uint32_t)motor1.run_state;
  g_motor_pwm_debug_duty = motor1.duty;
  g_motor_pwm_debug_ccr1 = htim1.Instance->CCR1;
  g_motor_pwm_debug_ccr2 = htim1.Instance->CCR2;
  g_motor_pwm_debug_ccr3 = htim1.Instance->CCR3;
  g_motor_pwm_debug_sd = (HAL_GPIO_ReadPin(PM1_CTRL_SD_GPIO_Port, PM1_CTRL_SD_Pin) == GPIO_PIN_SET) ? 1U : 0U;
}

MotorPwm_StatusTypeDef MotorPwm_Init(void)
{
  MotorPwm_HallGpioInit();
  MotorPwm_LowSideGpioInit();
  MotorPwm_DisableGate();
  MotorPwm_AllBridgeOff();
  MotorPwm_SetControl(MOTOR_PWM_DIR_CCW, 0U);

  return MotorPwm_Start();
}

MotorPwm_StatusTypeDef MotorPwm_Start(void)
{
  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK)
  {
    return MOTOR_PWM_ERROR;
  }

  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2) != HAL_OK)
  {
    return MOTOR_PWM_ERROR;
  }

  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3) != HAL_OK)
  {
    return MOTOR_PWM_ERROR;
  }

  __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE);

  return MOTOR_PWM_OK;
}

void MotorPwm_Stop(void)
{
  motor1.run_state = MOTOR_PWM_STOP;
  MotorPwm_DisableGate();
  MotorPwm_AllBridgeOff();
}

void MotorPwm_EnableGate(void)
{
  HAL_GPIO_WritePin(PM1_CTRL_SD_GPIO_Port, PM1_CTRL_SD_Pin, GPIO_PIN_SET);
  __HAL_TIM_MOE_ENABLE(&htim1);
  MotorPwm_UpdateDebug();
}

void MotorPwm_DisableGate(void)
{
  HAL_GPIO_WritePin(PM1_CTRL_SD_GPIO_Port, PM1_CTRL_SD_Pin, GPIO_PIN_RESET);
  MotorPwm_UpdateDebug();
}

void MotorPwm_SetControl(MotorPwm_DirectionTypeDef direction, uint32_t duty)
{
  if (direction != MOTOR_PWM_DIR_CW)
  {
    direction = MOTOR_PWM_DIR_CCW;
  }

  if (duty > MOTOR_PWM_DUTY_MAX)
  {
    duty = MOTOR_PWM_DUTY_MAX;
  }

  motor1.direction = direction;
  motor1.duty = duty;
  MotorPwm_UpdateDebug();

  if (duty == 0U)
  {
    motor1.run_state = MOTOR_PWM_STOP;
    MotorPwm_DisableGate();
    MotorPwm_AllBridgeOff();
  }
  else
  {
    motor1.run_state = MOTOR_PWM_RUN;
    MotorPwm_EnableGate();
    MotorPwm_Commutate();
  }
}

void MotorPwm_Commutate(void)
{
  uint32_t hall_state;

  if (motor1.run_state != MOTOR_PWM_RUN)
  {
    return;
  }

  hall_state = MotorPwm_GetHallState();
  g_motor_pwm_debug_hall_raw = hall_state;
  if (motor1.direction == MOTOR_PWM_DIR_CW)
  {
    hall_state = 7U - hall_state;
  }

  motor1.hall_state = hall_state;
  g_motor_pwm_debug_hall_step = hall_state;

  if ((hall_state >= 1U) && (hall_state <= 6U))
  {
    commutation_table[hall_state - 1U]();
  }
  else
  {
    MotorPwm_Stop();
  }

  MotorPwm_UpdateDebug();
}

uint32_t MotorPwm_GetHallState(void)
{
  uint32_t state = 0U;

  if (HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_10) != GPIO_PIN_RESET)
  {
    state |= 0x01U;
  }

  if (HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_11) != GPIO_PIN_RESET)
  {
    state |= 0x02U;
  }

  if (HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_12) != GPIO_PIN_RESET)
  {
    state |= 0x04U;
  }

  return state;
}

MotorPwm_RunStateTypeDef MotorPwm_GetRunState(void)
{
  return motor1.run_state;
}

uint32_t MotorPwm_GetPeriod(void)
{
  return __HAL_TIM_GET_AUTORELOAD(&htim1);
}

static void MotorPwm_AllBridgeOff(void)
{
  MotorPwm_SetCompare(0U, 0U, 0U);
  HAL_GPIO_WritePin(PM1_PWM_UL_GPIO_Port, PM1_PWM_UL_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PM1_PWM_VL_GPIO_Port, PM1_PWM_VL_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PM1_PWM_WL_GPIO_Port, PM1_PWM_WL_Pin, GPIO_PIN_RESET);
}

static void MotorPwm_UhVl(void)
{
  MotorPwm_SetCompare(motor1.duty, 0U, 0U);
  HAL_GPIO_WritePin(PM1_PWM_UL_GPIO_Port, PM1_PWM_UL_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PM1_PWM_VL_GPIO_Port, PM1_PWM_VL_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(PM1_PWM_WL_GPIO_Port, PM1_PWM_WL_Pin, GPIO_PIN_RESET);
}

static void MotorPwm_UhWl(void)
{
  MotorPwm_SetCompare(motor1.duty, 0U, 0U);
  HAL_GPIO_WritePin(PM1_PWM_UL_GPIO_Port, PM1_PWM_UL_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PM1_PWM_VL_GPIO_Port, PM1_PWM_VL_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PM1_PWM_WL_GPIO_Port, PM1_PWM_WL_Pin, GPIO_PIN_SET);
}

static void MotorPwm_VhWl(void)
{
  MotorPwm_SetCompare(0U, motor1.duty, 0U);
  HAL_GPIO_WritePin(PM1_PWM_UL_GPIO_Port, PM1_PWM_UL_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PM1_PWM_VL_GPIO_Port, PM1_PWM_VL_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PM1_PWM_WL_GPIO_Port, PM1_PWM_WL_Pin, GPIO_PIN_SET);
}

static void MotorPwm_VhUl(void)
{
  MotorPwm_SetCompare(0U, motor1.duty, 0U);
  HAL_GPIO_WritePin(PM1_PWM_UL_GPIO_Port, PM1_PWM_UL_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(PM1_PWM_VL_GPIO_Port, PM1_PWM_VL_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PM1_PWM_WL_GPIO_Port, PM1_PWM_WL_Pin, GPIO_PIN_RESET);
}

static void MotorPwm_WhUl(void)
{
  MotorPwm_SetCompare(0U, 0U, motor1.duty);
  HAL_GPIO_WritePin(PM1_PWM_UL_GPIO_Port, PM1_PWM_UL_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(PM1_PWM_VL_GPIO_Port, PM1_PWM_VL_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PM1_PWM_WL_GPIO_Port, PM1_PWM_WL_Pin, GPIO_PIN_RESET);
}

static void MotorPwm_WhVl(void)
{
  MotorPwm_SetCompare(0U, 0U, motor1.duty);
  HAL_GPIO_WritePin(PM1_PWM_UL_GPIO_Port, PM1_PWM_UL_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PM1_PWM_VL_GPIO_Port, PM1_PWM_VL_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(PM1_PWM_WL_GPIO_Port, PM1_PWM_WL_Pin, GPIO_PIN_RESET);
}

void MotorPwm_SetCompare(uint32_t compare_u, uint32_t compare_v, uint32_t compare_w)
{
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, MotorPwm_ClampCompare(compare_u));
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, MotorPwm_ClampCompare(compare_v));
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, MotorPwm_ClampCompare(compare_w));
}

static uint32_t MotorPwm_ClampCompare(uint32_t compare)
{
  uint32_t period = MotorPwm_GetPeriod();

  if (compare > period)
  {
    compare = period;
  }

  return compare;
}
