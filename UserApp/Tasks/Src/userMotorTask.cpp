#include "userMotorTask.hpp"

#include "Motor/motor_instance.hpp"
#include "cmsis_os.h"

namespace {

constexpr uint32_t kMotorTaskPeriodMs = 1U;

}  // namespace

extern "C" void UserMotorTask(void *argument)
{
  (void)argument;

  for (;;) {
    GetMotorController().Commutate();
    osDelay(kMotorTaskPeriodMs);
  }
}
