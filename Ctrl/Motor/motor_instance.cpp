#include "Motor/motor_instance.hpp"

#include "stm32_motor_pwm.hpp"

MotorController &GetMotorController()
{
  static MotorController controller(GetStm32MotorPwmDriver());
  return controller;
}
