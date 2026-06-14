#include "userMotorTask.hpp"

#include "Motor/motor_instance.hpp"
#include "cmsis_os.h"

namespace {

constexpr uint32_t kMotorTaskPeriodMs = 10U;

}  // namespace

extern "C" void MotorControlOnPwmTimerTick(void)
{
  GetMotorController().Commutate();
}

extern "C" void UserMotorTask(void *argument)
{
  (void)argument;

  for (;;) {
    osDelay(kMotorTaskPeriodMs);
  }
}
