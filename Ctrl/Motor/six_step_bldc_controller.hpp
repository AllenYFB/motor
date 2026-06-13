#ifndef SIX_STEP_BLDC_CONTROLLER_HPP
#define SIX_STEP_BLDC_CONTROLLER_HPP

#include "Driver/motor_pwm_driver.hpp"

class SixStepBldcController {
 public:
  explicit SixStepBldcController(MotorPwmDriver &driver);

  MotorStatus Init();
  MotorStatus Start();
  void Stop();
  void EnableGate();
  void DisableGate();
  void SetControl(MotorDirection direction, uint32_t duty);
  void SetDuty(float dutyU, float dutyV, float dutyW);
  void SetCompare(uint32_t compareU, uint32_t compareV, uint32_t compareW);
  void Commutate();
  uint32_t GetHallState();
  MotorRunState GetRunState() const;
  uint32_t GetPeriod();

 private:
  using CommutationFunc = void (SixStepBldcController::*)();

  uint32_t ClampCompare(uint32_t compare);
  void AllBridgeOff();
  void UhVl();
  void UhWl();
  void VhWl();
  void VhUl();
  void WhUl();
  void WhVl();

  MotorPwmDriver &driver_;
  volatile MotorRunState runState_;
  volatile MotorDirection direction_;
  volatile uint32_t duty_;
  volatile uint32_t hallState_;
};

#endif /* SIX_STEP_BLDC_CONTROLLER_HPP */
