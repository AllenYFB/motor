#ifndef BUTTON_BASE_H
#define BUTTON_BASE_H

#include <cstdint>

class ButtonBase {
 public:
  enum Event {
    UP,
    DOWN,
    LONG_PRESS,
    CLICK,
  };

  explicit ButtonBase(uint8_t id);
  ButtonBase(uint8_t id, uint32_t longPressTimeMs, uint32_t debounceTimeMs = 25U);

  void Tick(uint32_t elapsedMs);
  void SetOnEventListener(void (*callback)(uint8_t id, Event event));

 protected:
  virtual bool ReadButtonPinIO(uint8_t id) = 0;

  uint8_t id_;
  bool lastPinIO_;
  bool stablePinIO_;
  uint32_t timerMs_;
  uint32_t pressTimeMs_;
  uint32_t changedTimeMs_;
  uint32_t longPressTimeMs_;
  uint32_t debounceTimeMs_;
  void (*onEvent_)(uint8_t id, Event event);
};

#endif /* BUTTON_BASE_H */
