#include "button_stm32.h"

#include "gpio.h"

Button::Button(uint8_t id)
    : ButtonBase(id)
{
}

Button::Button(uint8_t id, uint32_t longPressTimeMs, uint32_t debounceTimeMs)
    : ButtonBase(id, longPressTimeMs, debounceTimeMs)
{
}

bool Button::ReadButtonPinIO(uint8_t id)
{
  switch (id) {
    case 0U:
      return HAL_GPIO_ReadPin(Key_0_GPIO_Port, Key_0_Pin) == GPIO_PIN_SET;

    case 1U:
      return HAL_GPIO_ReadPin(Key_1_GPIO_Port, Key_1_Pin) == GPIO_PIN_SET;

    case 2U:
      return HAL_GPIO_ReadPin(Key_2_GPIO_Port, Key_2_Pin) == GPIO_PIN_SET;

    default:
      return true;
  }
}

bool Button::IsPressed()
{
  return !ReadButtonPinIO(id_);
}
