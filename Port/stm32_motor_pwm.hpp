#ifndef STM32_MOTOR_PWM_HPP
#define STM32_MOTOR_PWM_HPP

#include "Driver/motor_pwm_driver.hpp"

class Stm32MotorPwmDriver : public MotorPwmDriver {
 public:
  void InitHallGpio() override;
  void InitLowSideGpio() override;
  MotorStatus StartPwmChannels() override;
  void EnableMainOutput() override;
  void WriteGate(bool enable) override;
  void WriteLowSide(bool u_low, bool v_low, bool w_low) override;
  void SetCompareRaw(uint32_t compare_u, uint32_t compare_v, uint32_t compare_w) override;
  uint32_t ReadHallGpio() override;
  uint32_t ReadPeriod() override;
};

Stm32MotorPwmDriver &GetStm32MotorPwmDriver();

#endif /* STM32_MOTOR_PWM_HPP */
