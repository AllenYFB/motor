#ifndef MOTOR_TYPES_HPP
#define MOTOR_TYPES_HPP

#include <cstdint>

constexpr uint32_t kMotorPwmDutyMin = 0U;
constexpr uint32_t kMotorPwmDutyMax = static_cast<uint32_t>((4200U - 1U) * 96U / 100U);
constexpr uint32_t kMotorPwmStartDuty = 500U;

enum class MotorStatus {
  Ok = 0,
  Error,
};

enum class MotorRunState {
  Stop = 0,
  Run,
};

enum class MotorDirection {
  Ccw = 1,
  Cw = 2,
};

#endif /* MOTOR_TYPES_HPP */
