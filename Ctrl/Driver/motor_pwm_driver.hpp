#ifndef MOTOR_PWM_DRIVER_HPP
#define MOTOR_PWM_DRIVER_HPP

#include "Motor/motor_types.hpp"

class MotorPwmDriver {
 public:
  virtual ~MotorPwmDriver() = default;

  virtual void InitOutputs() = 0;
  virtual MotorStatus StartPwmChannels() = 0;
  virtual void EnableMainOutput() = 0;
  virtual void DisableMainOutput() = 0;
  virtual void WriteGate(bool enable) = 0;
  virtual void SetPhaseOutputs(bool uHigh, bool vHigh, bool wHigh,
                               bool uLow, bool vLow, bool wLow) = 0;
  virtual void SetCompareRaw(uint32_t compareU, uint32_t compareV, uint32_t compareW) = 0;
  virtual uint32_t ReadPeriod() = 0;
};

#endif /* MOTOR_PWM_DRIVER_HPP */
