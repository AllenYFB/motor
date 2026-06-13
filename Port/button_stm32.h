#ifndef BUTTON_STM32_H
#define BUTTON_STM32_H

#include "Signal/button_base.h"

class Button : public ButtonBase {
 public:
  explicit Button(uint8_t id);
  Button(uint8_t id, uint32_t longPressTimeMs, uint32_t debounceTimeMs = 25U);

  bool IsPressed();

 private:
  bool ReadButtonPinIO(uint8_t id) override;
};

#endif /* BUTTON_STM32_H */
