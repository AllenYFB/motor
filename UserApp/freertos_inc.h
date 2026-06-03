#ifndef USERAPP_FREERTOS_INC_H
#define USERAPP_FREERTOS_INC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis_os.h"

extern osThreadId_t tcpServerTaskHandle;
extern osThreadId_t tcpClientTaskHandle;

#ifdef __cplusplus
}
#endif

#endif /* USERAPP_FREERTOS_INC_H */
