#include "userKeyTask.hpp"

#include "main.h"
#include "Motor/motor_instance.hpp"

namespace {

UserKeyTaskController userKeyTaskController;

void DispatchButtonEvent(uint8_t buttonId, ButtonBase::Event event)
{
  userKeyTaskController.OnButtonEvent(buttonId, event);
}

}  // namespace

UserKeyTaskController::UserKeyTaskController()
    : buttons_{
          Button(0U),
          Button(1U),
          Button(2U),
      },
      motorCommand_{
          false,
          MotorDirection::Ccw,
          kDutyMin,
      }
{
  for (uint32_t i = 0U; i < kButtonCount; ++i) {
    buttons_[i].SetOnEventListener(DispatchButtonEvent);
  }
}

void UserKeyTaskController::Run()
{
  ApplyMotorCommand();

  for (;;) {
    for (uint32_t i = 0U; i < kButtonCount; ++i) {
      buttons_[i].Tick(kScanPeriodMs);
    }

    osDelay(kScanPeriodMs);
  }
}

void UserKeyTaskController::OnButtonEvent(uint8_t buttonId, ButtonBase::Event event)
{
  if (event != ButtonBase::CLICK) {
    return;
  }

  OnKeyPressed(buttonId);
}

void UserKeyTaskController::SetStatusLed(bool motorEnabled)
{
  HAL_GPIO_WritePin(Red_LED0_GPIO_Port, Red_LED0_Pin,
                    motorEnabled ? GPIO_PIN_RESET : GPIO_PIN_SET);
  HAL_GPIO_WritePin(Green_LED1_GPIO_Port, Green_LED1_Pin,
                    motorEnabled ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void UserKeyTaskController::Beep(uint32_t durationMs)
{
  HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
  osDelay(durationMs);
  HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);
}

void UserKeyTaskController::ApplyMotorCommand()
{
  if (motorCommand_.enabled) {
    GetMotorController().SetControl(motorCommand_.direction, motorCommand_.duty);
  } else {
    GetMotorController().Stop();
  }

  SetStatusLed(motorCommand_.enabled);
}

void UserKeyTaskController::OnKeyPressed(uint32_t keyIndex)
{
  switch (keyIndex) {
    case 0U:
      motorCommand_.enabled = !motorCommand_.enabled;
      break;

    case 1U:
      if (motorCommand_.duty + kDutyStep <= kMotorPwmDutyMax) {
        motorCommand_.duty += kDutyStep;
      } else {
        motorCommand_.duty = kDutyMin;
      }
      motorCommand_.enabled = true;
      break;

    case 2U:
      motorCommand_.direction =
          (motorCommand_.direction == MotorDirection::Ccw) ? MotorDirection::Cw
                                                           : MotorDirection::Ccw;
      motorCommand_.enabled = true;
      break;

    default:
      break;
  }

  ApplyMotorCommand();
  Beep(20U);
}

extern "C" void UserKeyTask(void *argument)
{
  (void)argument;
  userKeyTaskController.Run();
}
