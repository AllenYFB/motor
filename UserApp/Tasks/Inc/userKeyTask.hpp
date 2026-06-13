#ifndef USER_KEY_TASK_HPP
#define USER_KEY_TASK_HPP

#include "cmsis_os.h"
#include "button_stm32.h"
#include "Motor/motor_types.hpp"

class UserKeyTaskController {
 public:
  UserKeyTaskController();

  void Run();
  void OnButtonEvent(uint8_t buttonId, ButtonBase::Event event);

 private:
  struct MotorCommand {
    bool enabled;
    MotorDirection direction;
    uint32_t duty;
  };

  static constexpr uint32_t kButtonCount = 3U;
  static constexpr uint32_t kScanPeriodMs = 10U;
  static constexpr uint32_t kDutyStep = 200U;
  static constexpr uint32_t kDutyMin = kMotorPwmStartDuty;

  void SetStatusLed(bool motorEnabled);
  void Beep(uint32_t durationMs);
  void ApplyMotorCommand();
  void OnKeyPressed(uint32_t keyIndex);

  Button buttons_[kButtonCount];
  MotorCommand motorCommand_;
};

#ifdef __cplusplus
extern "C" {
#endif

void UserKeyTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* USER_KEY_TASK_HPP */
