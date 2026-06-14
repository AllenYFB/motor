#ifndef USER_MOTOR_TASK_HPP
#define USER_MOTOR_TASK_HPP

#ifdef __cplusplus
extern "C" {
#endif

void UserMotorTask(void *argument);
void MotorControlOnPwmTimerTick(void);

#ifdef __cplusplus
}
#endif

#endif /* USER_MOTOR_TASK_HPP */
