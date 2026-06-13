#include "stm32_hall_sensor.hpp"

#include "main.h"

void Stm32HallSensor::Init()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
}

uint32_t Stm32HallSensor::ReadState()
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

Stm32HallSensor &GetStm32HallSensor()
{
  static Stm32HallSensor hallSensor;
  return hallSensor;
}
