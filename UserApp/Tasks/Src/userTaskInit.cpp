#include "userTaskInit.hpp"

#include "Motor/motor_instance.hpp"
#include "userKeyTask.hpp"
#include "userMotorTask.hpp"
#include "cmsis_os.h"

namespace {

const osThreadAttr_t KeyTask_attributes = {
    .name = "KeyTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

const osThreadAttr_t MotorTask_attributes = {
    .name = "MotorTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityAboveNormal,
};

}  // namespace

void User_Tasks_Init(void)
{
  GetMotorController().Init();

  osThreadNew(UserMotorTask, nullptr, &MotorTask_attributes);
  osThreadNew(UserKeyTask, nullptr, &KeyTask_attributes);
}
