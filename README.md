# STM32F407 Motor Project

这是一个基于 STM32F407IGT6 的电机控制工程。工程由 STM32CubeMX 生成外设、FreeRTOS 和 LwIP 初始化代码，使用 CMake + Ninja + Arm GNU Toolchain 在 VS Code 中编译，使用 OpenOCD + CMSIS-DAP 进行烧录和调试。

当前分支目标是摆脱 Keil/MDK，建立一套以后更换芯片也能复用的开发环境。

## 当前环境

- MCU: STM32F407IGT6
- CubeMX: 6.16.0
- IDE: VS Code
- Build system: CMake preset
- Generator: Ninja
- Compiler: Arm GNU Toolchain 12.2.1
- Debug server: OpenOCD 0.12.0
- Debug probe: CMSIS-DAP, SWD
- VS Code debug plugin: Cortex-Debug

## 新电脑环境安装

### 1. 安装基础工具

建议使用 `winget` 安装：

```powershell
winget install Kitware.CMake
winget install Ninja-build.Ninja
winget install Arm.ArmGnuToolchain
```

也需要安装：

- STM32CubeMX
- VS Code
- VS Code 插件 `CMake Tools`
- VS Code 插件 `Cortex-Debug`
- OpenOCD，例如安装到 `D:\OpenOCD-20240916-0.12.0`

### 2. 检查命令

打开 VS Code 终端，执行：

```powershell
cmake --version
ninja --version
arm-none-eabi-gcc --version
```

推荐看到：

```text
arm-none-eabi-gcc 12.2.1
```

如果系统里同时存在旧版 GCC，要保证新版路径在旧版前面：

```text
C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\12.2 mpacbti-rel1\bin
```

本工程的 `.vscode/settings.json` 已经为 VS Code 当前工作区优先配置了这个路径。

## CubeMX 生成工程

打开 `stm32F407IGT6.ioc`，在 Project Manager 中选择：

```text
Toolchain / IDE: CMake
Default Compiler: GCC
```

然后点击 Generate Code。

CubeMX 会生成：

```text
CMakeLists.txt
CMakePresets.json
cmake/gcc-arm-none-eabi.cmake
cmake/stm32cubemx/CMakeLists.txt
startup_stm32f407xx.s
STM32F407XX_FLASH.ld
Core/Src/syscalls.c
Core/Src/sysmem.c
Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
```

注意：本工程自己的用户源文件，例如 `Core/Src/motor_pwm.c`，需要保留在顶层 `CMakeLists.txt` 的 user sources 区域：

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Src/motor_pwm.c
)
```

## 编译

第一次配置或重新生成 CubeMX 后：

```powershell
cmake --fresh --preset Debug
cmake --build --preset Debug
```

平时只编译：

```powershell
cmake --build --preset Debug
```

生成文件：

```text
build/Debug/stm32F407IGT6.elf
build/Debug/stm32F407IGT6.map
```

也可以在 VS Code 中运行任务：

```text
Tasks: Run Task -> Build Debug
```

## 烧录

本工程使用 OpenOCD + CMSIS-DAP + SWD。

命令行烧录：

```powershell
& "D:\OpenOCD-20240916-0.12.0\bin\openocd.exe" `
  -s "D:\OpenOCD-20240916-0.12.0\share\openocd\scripts" `
  -f interface/cmsis-dap.cfg `
  -c "transport select swd" `
  -f target/stm32f4x.cfg `
  -c "adapter speed 4000" `
  -c "program build/Debug/stm32F407IGT6.elf verify reset exit"
```

看到下面信息表示成功：

```text
Programming Finished
Verified OK
Resetting Target
```

VS Code 一键烧录：

```text
Tasks: Run Task -> Flash STM32F407
```

该任务会先执行 `Build Debug`，再烧录。

## 调试

安装 VS Code 插件：

```text
Cortex-Debug
```

调试配置位于：

```text
.vscode/launch.json
```

启动调试：

```text
Run and Debug -> Debug STM32F407
```

或者按 `F5`。

常用快捷键：

```text
F5          Continue
F10         Step Over
F11         Step Into
Shift+F11   Step Out
Shift+F5    Stop
```

变量查看：

- 鼠标悬停在变量上
- 左侧 VARIABLES 面板
- WATCH 中添加变量名

注意：STM32F407 的 Cortex-M4 只有 6 个 Flash 硬件断点。调试时不要一次设置太多断点，否则会出现：

```text
Can not find free FPB Comparator
can't add breakpoint: resource not available
```

建议先只保留 1 到 2 个断点。

## VS Code 文件说明

```text
.vscode/settings.json
```

配置当前工程的 PATH，让 VS Code 优先使用 Arm GNU Toolchain 12.2 和 Ninja。

```text
.vscode/tasks.json
```

包含：

- `Build Debug`
- `Flash STM32F407`

```text
.vscode/launch.json
```

包含 Cortex-Debug 调试配置。

## Git 提交建议

建议提交：

```text
CMakeLists.txt
CMakePresets.json
cmake/
startup_stm32f407xx.s
STM32F407XX_FLASH.ld
Core/
Drivers/
LWIP/
Middlewares/
.vscode/
.gitignore
README.md
stm32F407IGT6.ioc
```

不要提交：

```text
build/
```

Keil/MDK 生成产物也不建议继续提交。

## 更换其他芯片时的思路

1. 用 CubeMX 新建或修改 `.ioc`。
2. 选择 CMake + GCC。
3. 重新生成启动文件、链接脚本和 `cmake/stm32cubemx`。
4. 保留自己的业务代码目录，例如后续的 `Ctrl/`、`Port/`、`UserApp/`。
5. 针对新芯片重写 `Port/` 层适配，不动上层电机控制逻辑。

也就是说，CubeMX 负责芯片初始化和底层工程骨架，CMake/VS Code/OpenOCD 负责统一编译、烧录和调试。
