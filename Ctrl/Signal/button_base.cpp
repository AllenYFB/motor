#include "Signal/button_base.h"

ButtonBase::ButtonBase(uint8_t id)
    : ButtonBase(id, 2000U, 25U)
{
}

ButtonBase::ButtonBase(uint8_t id, uint32_t longPressTimeMs, uint32_t debounceTimeMs)
    : id_(id),
      lastPinIO_(false),
      stablePinIO_(false),
      timerMs_(0U),
      pressTimeMs_(0U),
      changedTimeMs_(0U),
      longPressTimeMs_(longPressTimeMs),
      debounceTimeMs_(debounceTimeMs),
      onEvent_(nullptr)
{
}

void ButtonBase::Tick(uint32_t elapsedMs)
{
  timerMs_ += elapsedMs;

  const bool pinIO = ReadButtonPinIO(id_);
  if (pinIO != lastPinIO_) {
    lastPinIO_ = pinIO;
    changedTimeMs_ = timerMs_;
  }

  if ((timerMs_ - changedTimeMs_) < debounceTimeMs_) {
    return;
  }

  if (pinIO == stablePinIO_) {
    return;
  }

  stablePinIO_ = pinIO;
  if (onEvent_ == nullptr) {
    return;
  }

  if (pinIO) {
    onEvent_(id_, UP);
    if ((timerMs_ - pressTimeMs_) > longPressTimeMs_) {
      onEvent_(id_, LONG_PRESS);
    } else {
      onEvent_(id_, CLICK);
    }
  } else {
    onEvent_(id_, DOWN);
    pressTimeMs_ = timerMs_;
  }
}

void ButtonBase::SetOnEventListener(void (*callback)(uint8_t id, Event event))
{
  lastPinIO_ = ReadButtonPinIO(id_);
  stablePinIO_ = lastPinIO_;
  onEvent_ = callback;
}
