#include "stm32_motor_pwm.hpp"

#include "tim.h"

void Stm32MotorPwmDriver::InitHallGpio()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
}

void Stm32MotorPwmDriver::InitLowSideGpio()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = PM1_PWM_UL_Pin | PM1_PWM_VL_Pin | PM1_PWM_WL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  WriteLowSide(false, false, false);
}

MotorStatus Stm32MotorPwmDriver::StartPwmChannels()
{
  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK) {
    return MotorStatus::Error;
  }

  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2) != HAL_OK) {
    return MotorStatus::Error;
  }

  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3) != HAL_OK) {
    return MotorStatus::Error;
  }

  return MotorStatus::Ok;
}

void Stm32MotorPwmDriver::EnableMainOutput()
{
  __HAL_TIM_MOE_ENABLE(&htim1);
}

void Stm32MotorPwmDriver::WriteGate(bool enable)
{
  HAL_GPIO_WritePin(PM1_CTRL_SD_GPIO_Port, PM1_CTRL_SD_Pin,
                    enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Stm32MotorPwmDriver::WriteLowSide(bool u_low, bool v_low, bool w_low)
{
  HAL_GPIO_WritePin(PM1_PWM_UL_GPIO_Port, PM1_PWM_UL_Pin,
                    u_low ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PM1_PWM_VL_GPIO_Port, PM1_PWM_VL_Pin,
                    v_low ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PM1_PWM_WL_GPIO_Port, PM1_PWM_WL_Pin,
                    w_low ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Stm32MotorPwmDriver::SetCompareRaw(uint32_t compare_u, uint32_t compare_v, uint32_t compare_w)
{
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, compare_u);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, compare_v);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, compare_w);
}

uint32_t Stm32MotorPwmDriver::ReadHallGpio()
{
  uint32_t state = 0U;

  if (HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_10) != GPIO_PIN_RESET) {
    state |= 0x01U;
  }

  if (HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_11) != GPIO_PIN_RESET) {
    state |= 0x02U;
  }

  if (HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_12) != GPIO_PIN_RESET) {
    state |= 0x04U;
  }

  return state;
}

uint32_t Stm32MotorPwmDriver::ReadPeriod()
{
  return __HAL_TIM_GET_AUTORELOAD(&htim1);
}

Stm32MotorPwmDriver &GetStm32MotorPwmDriver()
{
  static Stm32MotorPwmDriver motorPwmDriver;
  return motorPwmDriver;
}
