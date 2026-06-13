#include "Motor/motor_instance.hpp"

#include "stm32_hall_sensor.hpp"
#include "stm32_motor_pwm.hpp"

MotorController &GetMotorController()
{
  static MotorController controller(GetStm32MotorPwmDriver(), GetStm32HallSensor());
  return controller;
}
