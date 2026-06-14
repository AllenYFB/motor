#include "Motor/six_step_bldc_controller.hpp"

SixStepBldcController::SixStepBldcController(MotorPwmDriver &driver, HallSensorBase &hallSensor)
    : driver_(driver),
      hallSensor_(hallSensor),
      runState_(MotorRunState::Stop),
      direction_(MotorDirection::Ccw),
      duty_(0U),
      hallState_(0U)
{
}

MotorStatus SixStepBldcController::Init()
{
  hallSensor_.Init();
  driver_.InitOutputs();
  DisableGate();
  AllBridgeOff();
  SetControl(MotorDirection::Ccw, 0U);

  return Start();
}

MotorStatus SixStepBldcController::Start()
{
  const MotorStatus status = driver_.StartPwmChannels();
  if (status != MotorStatus::Ok) {
    return status;
  }

  return MotorStatus::Ok;
}

void SixStepBldcController::Stop()
{
  runState_ = MotorRunState::Stop;
  DisableGate();
  AllBridgeOff();
}

void SixStepBldcController::EnableGate()
{
  driver_.WriteGate(true);
  driver_.EnableMainOutput();
}

void SixStepBldcController::DisableGate()
{
  driver_.DisableMainOutput();
  driver_.WriteGate(false);
}

void SixStepBldcController::SetControl(MotorDirection direction, uint32_t duty)
{
  if (direction != MotorDirection::Cw) {
    direction = MotorDirection::Ccw;
  }

  if (duty > kMotorPwmDutyMax) {
    duty = kMotorPwmDutyMax;
  }

  direction_ = direction;
  duty_ = duty;

  if (duty == 0U) {
    runState_ = MotorRunState::Stop;
    DisableGate();
    AllBridgeOff();
  } else {
    runState_ = MotorRunState::Run;
    EnableGate();
    Commutate();
  }
}

void SixStepBldcController::SetDuty(float dutyU, float dutyV, float dutyW)
{
  const uint32_t period = GetPeriod();

  if (dutyU < 0.0f) {
    dutyU = 0.0f;
  } else if (dutyU > 1.0f) {
    dutyU = 1.0f;
  }

  if (dutyV < 0.0f) {
    dutyV = 0.0f;
  } else if (dutyV > 1.0f) {
    dutyV = 1.0f;
  }

  if (dutyW < 0.0f) {
    dutyW = 0.0f;
  } else if (dutyW > 1.0f) {
    dutyW = 1.0f;
  }

  SetCompare(static_cast<uint32_t>(dutyU * static_cast<float>(period)),
             static_cast<uint32_t>(dutyV * static_cast<float>(period)),
             static_cast<uint32_t>(dutyW * static_cast<float>(period)));
}

void SixStepBldcController::SetCompare(uint32_t compareU, uint32_t compareV, uint32_t compareW)
{
  driver_.SetCompareRaw(ClampCompare(compareU), ClampCompare(compareV), ClampCompare(compareW));
}

void SixStepBldcController::Commutate()
{
  static constexpr CommutationFunc commutationTable[6] = {
      &SixStepBldcController::UhWl,
      &SixStepBldcController::VhUl,
      &SixStepBldcController::VhWl,
      &SixStepBldcController::WhVl,
      &SixStepBldcController::UhVl,
      &SixStepBldcController::WhUl,
  };

  if (runState_ != MotorRunState::Run) {
    return;
  }

  uint32_t hallState = GetHallState();
  if (direction_ == MotorDirection::Cw) {
    hallState = 7U - hallState;
  }

  hallState_ = hallState;

  if ((hallState >= 1U) && (hallState <= 6U)) {
    (this->*commutationTable[hallState - 1U])();
  } else {
    Stop();
  }

}

uint32_t SixStepBldcController::GetHallState()
{
  return hallSensor_.ReadState();
}

MotorRunState SixStepBldcController::GetRunState() const
{
  return runState_;
}

uint32_t SixStepBldcController::GetPeriod()
{
  return driver_.ReadPeriod();
}

uint32_t SixStepBldcController::ClampCompare(uint32_t compare)
{
  const uint32_t period = GetPeriod();

  if (compare > period) {
    compare = period;
  }

  return compare;
}

void SixStepBldcController::AllBridgeOff()
{
  SetCompare(0U, 0U, 0U);
  driver_.SetPhaseOutputs(false, false, false, false, false, false);
}

void SixStepBldcController::UhVl()
{
  SetCompare(duty_, GetPeriod(), 0U);
  driver_.SetPhaseOutputs(true, false, false, false, true, false);
}

void SixStepBldcController::UhWl()
{
  SetCompare(duty_, 0U, GetPeriod());
  driver_.SetPhaseOutputs(true, false, false, false, false, true);
}

void SixStepBldcController::VhWl()
{
  SetCompare(0U, duty_, GetPeriod());
  driver_.SetPhaseOutputs(false, true, false, false, false, true);
}

void SixStepBldcController::VhUl()
{
  SetCompare(GetPeriod(), duty_, 0U);
  driver_.SetPhaseOutputs(false, true, false, true, false, false);
}

void SixStepBldcController::WhUl()
{
  SetCompare(GetPeriod(), 0U, duty_);
  driver_.SetPhaseOutputs(false, false, true, true, false, false);
}

void SixStepBldcController::WhVl()
{
  SetCompare(0U, GetPeriod(), duty_);
  driver_.SetPhaseOutputs(false, false, true, false, true, false);
}
