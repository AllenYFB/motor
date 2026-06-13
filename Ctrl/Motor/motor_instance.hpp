#ifndef MOTOR_INSTANCE_HPP
#define MOTOR_INSTANCE_HPP

#include "Motor/six_step_bldc_controller.hpp"

using MotorController = SixStepBldcController;

MotorController &GetMotorController();

#endif /* MOTOR_INSTANCE_HPP */
