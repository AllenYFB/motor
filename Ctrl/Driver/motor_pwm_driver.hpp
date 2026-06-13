#ifndef MOTOR_PWM_DRIVER_HPP
#define MOTOR_PWM_DRIVER_HPP

#include "Motor/motor_types.hpp"

class MotorPwmDriver {
 public:
  virtual ~MotorPwmDriver() = default;

  virtual void InitHallGpio() = 0;
  virtual void InitLowSideGpio() = 0;
  virtual MotorStatus StartPwmChannels() = 0;
  virtual void EnableMainOutput() = 0;
  virtual void WriteGate(bool enable) = 0;
  virtual void WriteLowSide(bool uLow, bool vLow, bool wLow) = 0;
  virtual void SetCompareRaw(uint32_t compareU, uint32_t compareV, uint32_t compareW) = 0;
  virtual uint32_t ReadHallGpio() = 0;
  virtual uint32_t ReadPeriod() = 0;
};

#endif /* MOTOR_PWM_DRIVER_HPP */
