#ifndef STM32_MOTOR_PWM_HPP
#define STM32_MOTOR_PWM_HPP

#include "Driver/motor_pwm_driver.hpp"

class Stm32MotorPwmDriver : public MotorPwmDriver {
 public:
  void InitOutputs() override;
  MotorStatus StartPwmChannels() override;
  void EnableMainOutput() override;
  void DisableMainOutput() override;
  void WriteGate(bool enable) override;
  void SetPhaseOutputs(bool u_high, bool v_high, bool w_high,
                       bool u_low, bool v_low, bool w_low) override;
  void SetCompareRaw(uint32_t compare_u, uint32_t compare_v, uint32_t compare_w) override;
  uint32_t ReadPeriod() override;

 private:
  MotorStatus StartPwmPair(uint32_t channel);
  void SetChannelEnabled(uint32_t channel, bool high_enabled, bool low_enabled);
};

Stm32MotorPwmDriver &GetStm32MotorPwmDriver();

#endif /* STM32_MOTOR_PWM_HPP */
