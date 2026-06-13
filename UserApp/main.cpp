#include "common_inc.h"

/* C 代码在 freertos.c 中调用 Main()，这里使用 extern "C" 避免 C++ 名字改编。 */
extern "C" void Main(void)
{
  /* 用户应用入口只负责启动用户层任务，具体任务创建放在 userTaskInit.cpp。 */
  User_Tasks_Init();
}
