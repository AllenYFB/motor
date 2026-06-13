# CubeMX 再生成后恢复入口

本工程约定：`Core/` 只放 CubeMX 生成代码，用户 C++ 代码放在 `UserApp/`、`Ctrl/`、`Port/`。

CubeMX 重新生成后，只需要检查 `Core/Src/freertos.c` 的两个 USER CODE 区。

## 1. Includes 区

在 `/* USER CODE BEGIN Includes */` 和 `/* USER CODE END Includes */` 之间保留：

```c
#include "common_inc.h"
```

## 2. StartDefaultTask 区

在 `StartDefaultTask()` 中，`MX_LWIP_Init();` 后面的 `/* USER CODE BEGIN StartDefaultTask */` 区保留：

```c
Main();
vTaskDelete(defaultTaskHandle);
```

完整形态如下：

```c
void StartDefaultTask(void *argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
  Main();
  vTaskDelete(defaultTaskHandle);
  /* USER CODE END StartDefaultTask */
}
```

## 3. CMake

根目录 `CMakeLists.txt` 是用户维护区，用来加入 C++ 源文件和 include 路径。CubeMX 子目录 `cmake/stm32cubemx/` 保持生成状态即可。

如果新增用户文件，只改根目录 `CMakeLists.txt`：

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    UserApp/main.cpp
    UserApp/Tasks/Src/userTaskInit.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    UserApp
    UserApp/Tasks/Inc
    Ctrl
    Port
)
```

## 4. 不要放进 Core 的内容

- 不要把电机控制 `.cpp/.hpp` 放进 `Core/`
- 不要在 `Core/Src/main.c` 里直接初始化电机库
- 不要在 `Core/Src/stm32f4xx_it.c` 或 HAL 回调里直接写算法逻辑

需要周期运行的逻辑放在 `UserApp/Tasks`，例如当前的 `UserMotorTask`。
