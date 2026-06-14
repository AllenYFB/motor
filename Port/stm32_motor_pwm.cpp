#include "stm32_motor_pwm.hpp"

#include "tim.h"

void Stm32MotorPwmDriver::InitOutputs()
{
  SetCompareRaw(0U, 0U, 0U);
  SetPhaseOutputs(false, false, false, false, false, false);
}

MotorStatus Stm32MotorPwmDriver::StartPwmChannels()
{
  if (StartPwmPair(TIM_CHANNEL_1) != MotorStatus::Ok) {
    return MotorStatus::Error;
  }

  if (StartPwmPair(TIM_CHANNEL_2) != MotorStatus::Ok) {
    return MotorStatus::Error;
  }

  if (StartPwmPair(TIM_CHANNEL_3) != MotorStatus::Ok) {
    return MotorStatus::Error;
  }

  SetPhaseOutputs(false, false, false, false, false, false);
  return MotorStatus::Ok;
}

void Stm32MotorPwmDriver::EnableMainOutput()
{
  __HAL_TIM_MOE_ENABLE(&htim1);
}

void Stm32MotorPwmDriver::DisableMainOutput()
{
  __HAL_TIM_MOE_DISABLE_UNCONDITIONALLY(&htim1);
}

void Stm32MotorPwmDriver::WriteGate(bool enable)
{
  HAL_GPIO_WritePin(PM1_CTRL_SD_GPIO_Port, PM1_CTRL_SD_Pin,
                    enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Stm32MotorPwmDriver::SetPhaseOutputs(bool u_high, bool v_high, bool w_high,
                                          bool u_low, bool v_low, bool w_low)
{
  SetChannelEnabled(TIM_CHANNEL_1, u_high, u_low);
  SetChannelEnabled(TIM_CHANNEL_2, v_high, v_low);
  SetChannelEnabled(TIM_CHANNEL_3, w_high, w_low);
}

void Stm32MotorPwmDriver::SetCompareRaw(uint32_t compare_u, uint32_t compare_v, uint32_t compare_w)
{
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, compare_u);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, compare_v);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, compare_w);
}

uint32_t Stm32MotorPwmDriver::ReadPeriod()
{
  return __HAL_TIM_GET_AUTORELOAD(&htim1);
}

MotorStatus Stm32MotorPwmDriver::StartPwmPair(uint32_t channel)
{
  if (HAL_TIM_PWM_Start(&htim1, channel) != HAL_OK) {
    return MotorStatus::Error;
  }

  if (HAL_TIMEx_PWMN_Start(&htim1, channel) != HAL_OK) {
    return MotorStatus::Error;
  }

  return MotorStatus::Ok;
}

void Stm32MotorPwmDriver::SetChannelEnabled(uint32_t channel, bool high_enabled, bool low_enabled)
{
  uint32_t high_mask = 0U;
  uint32_t low_mask = 0U;

  switch (channel) {
    case TIM_CHANNEL_1:
      high_mask = TIM_CCER_CC1E;
      low_mask = TIM_CCER_CC1NE;
      break;

    case TIM_CHANNEL_2:
      high_mask = TIM_CCER_CC2E;
      low_mask = TIM_CCER_CC2NE;
      break;

    case TIM_CHANNEL_3:
      high_mask = TIM_CCER_CC3E;
      low_mask = TIM_CCER_CC3NE;
      break;

    default:
      return;
  }

  if (high_enabled) {
    htim1.Instance->CCER |= high_mask;
  } else {
    htim1.Instance->CCER &= ~high_mask;
  }

  if (low_enabled) {
    htim1.Instance->CCER |= low_mask;
  } else {
    htim1.Instance->CCER &= ~low_mask;
  }
}

Stm32MotorPwmDriver &GetStm32MotorPwmDriver()
{
  static Stm32MotorPwmDriver motorPwmDriver;
  return motorPwmDriver;
}
