#ifndef USERAPP_COMMON_INC_H
#define USERAPP_COMMON_INC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "freertos_inc.h"
#include "usart.h"

void Main(void);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include <string.h>
#endif

#endif /* USERAPP_COMMON_INC_H */
