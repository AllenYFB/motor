/*
 * motor_pwm.h
 *
 * Six-step BLDC driver for motor 1.
 */

#ifndef __MOTOR_PWM_H__
#define __MOTOR_PWM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define MOTOR_PWM_DUTY_MIN        (0U)
#define MOTOR_PWM_DUTY_MAX        ((uint32_t)((4200U - 1U) * 96U / 100U))
#define MOTOR_PWM_START_DUTY      (500U)

typedef enum
{
  MOTOR_PWM_OK = 0,
  MOTOR_PWM_ERROR
} MotorPwm_StatusTypeDef;

typedef enum
{
  MOTOR_PWM_STOP = 0,
  MOTOR_PWM_RUN
} MotorPwm_RunStateTypeDef;

typedef enum
{
  MOTOR_PWM_DIR_CCW = 1,
  MOTOR_PWM_DIR_CW = 2
} MotorPwm_DirectionTypeDef;

extern volatile uint32_t g_motor_pwm_debug_hall_raw;
extern volatile uint32_t g_motor_pwm_debug_hall_step;
extern volatile uint32_t g_motor_pwm_debug_run_state;
extern volatile uint32_t g_motor_pwm_debug_duty;
extern volatile uint32_t g_motor_pwm_debug_ccr1;
extern volatile uint32_t g_motor_pwm_debug_ccr2;
extern volatile uint32_t g_motor_pwm_debug_ccr3;
extern volatile uint32_t g_motor_pwm_debug_sd;

MotorPwm_StatusTypeDef MotorPwm_Init(void);
MotorPwm_StatusTypeDef MotorPwm_Start(void);
void MotorPwm_Stop(void);
void MotorPwm_EnableGate(void);
void MotorPwm_DisableGate(void);
void MotorPwm_SetControl(MotorPwm_DirectionTypeDef direction, uint32_t duty);
void MotorPwm_SetDuty(float duty_u, float duty_v, float duty_w);
void MotorPwm_SetCompare(uint32_t compare_u, uint32_t compare_v, uint32_t compare_w);
void MotorPwm_Commutate(void);
uint32_t MotorPwm_GetHallState(void);
MotorPwm_RunStateTypeDef MotorPwm_GetRunState(void);
uint32_t MotorPwm_GetPeriod(void);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_PWM_H__ */
