# STM32F407 Motor Project

这是一个基于 STM32F407IGT6 的电机控制工程。工程由 STM32CubeMX 生成外设、FreeRTOS 和 LwIP 初始化代码，使用 CMake + Ninja + Arm GNU Toolchain 在 VS Code 中编译，使用 OpenOCD + CMSIS-DAP 进行烧录和调试。

当前分支目标是摆脱 Keil/MDK，建立一套以后更换芯片也能复用的开发环境。

## 当前验证环境

这套环境已经在 Windows 下验证通过：

```text
Windows: Windows 10/11
STM32CubeMX: 6.16.0
VS Code: 1.x
CMake: 3.26.0
Ninja: 1.13.2
Arm GNU Toolchain: 12.2.1
OpenOCD: 0.12.0, 2024-09-16 build
Debug probe: CMSIS-DAP, SWD
VS Code plugin: CMake Tools
VS Code plugin: Cortex-Debug
```

工程目标芯片：

```text
STM32F407IGT6
```

## 新电脑从零安装

### 1. 是否需要管理员权限

普通情况下，`winget install` 可以在普通 PowerShell 中运行。但如果遇到以下情况，建议用管理员身份打开 PowerShell：

```text
安装器要求写入 Program Files
安装后 PATH 没有被写入
winget 提示权限不足
VS Code 新终端识别不到新工具
```

管理员 PowerShell 打开方式：

1. 开始菜单搜索 `PowerShell`
2. 右键
3. 选择 `以管理员身份运行`

如果只是本工程使用，也可以不改系统 PATH。本工程的 `.vscode/settings.json` 已经为 VS Code 当前工作区配置了工具路径。

### 2. 安装 STM32CubeMX

安装 STM32CubeMX，推荐版本：

```text
STM32CubeMX 6.16.0
```

下载地址：

```text
https://www.st.com/en/development-tools/stm32cubemx.html
```

安装完成后，能够打开本工程的：

```text
stm32F407IGT6.ioc
```

### 3. 安装 VS Code

安装 VS Code：

```text
https://code.visualstudio.com/
```

然后安装插件：

```text
CMake Tools
Cortex-Debug
C/C++
```

### 4. 安装 CMake

推荐版本：

```text
CMake 3.26.0 或更新版本
```

使用 winget：

```powershell
winget install Kitware.CMake
```

如果想看 winget 能安装的版本：

```powershell
winget show Kitware.CMake
```

如果 winget 不可用，手动下载安装：

```text
https://cmake.org/download/
```

安装时建议勾选：

```text
Add CMake to the system PATH
```

验证：

```powershell
cmake --version
```

### 5. 安装 Ninja

推荐版本：

```text
Ninja 1.13.2
```

使用 winget：

```powershell
winget install Ninja-build.Ninja
```

查看版本：

```powershell
winget show Ninja-build.Ninja
```

如果 winget 不可用，手动下载：

```text
https://github.com/ninja-build/ninja/releases
```

下载 `ninja-win.zip`，解压后把 `ninja.exe` 所在目录加入 PATH。

winget 安装后，Ninja 通常位于：

```text
C:\Users\<用户名>\AppData\Local\Microsoft\WinGet\Links\ninja.exe
```

验证：

```powershell
ninja --version
```

如果 VS Code 终端提示找不到 `ninja`，先关闭 VS Code，然后重新打开。

### 6. 安装 Arm GNU Toolchain

推荐版本：

```text
Arm GNU Toolchain 12.2.1
```

本工程使用 CubeMX 生成的 GCC 链接脚本，其中部分段使用了 `READONLY`。这个写法需要 GCC 11 或更新版本。不要使用旧的 `arm-none-eabi-gcc 10.3.1`，否则可能在链接时报错：

```text
non constant or forward reference address expression for section .ARM.extab
```

使用 winget：

```powershell
winget install Arm.ArmGnuToolchain
```

查看 winget 包信息：

```powershell
winget show Arm.ArmGnuToolchain
```

本工程验证过的安装路径：

```text
C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\12.2 mpacbti-rel1\bin
```

验证：

```powershell
arm-none-eabi-gcc --version
arm-none-eabi-gdb --version
```

推荐看到：

```text
arm-none-eabi-gcc 12.2.1
```

如果显示的是旧版本，例如：

```text
arm-none-eabi-gcc 10.3.1
```

说明旧工具链路径排在新工具链前面。需要把下面路径放到 PATH 前面：

```text
C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\12.2 mpacbti-rel1\bin
```

本工程的 `.vscode/settings.json` 已经在 VS Code 工作区内优先配置该路径。如果 VS Code 终端仍然显示旧版本，请关闭 VS Code 后重新打开。

### 7. 安装 OpenOCD

本工程验证过：

```text
OpenOCD 0.12.0, 2024-09-16
```

当前配置默认 OpenOCD 路径为：

```text
D:\OpenOCD-20240916-0.12.0
```

也就是说，需要存在：

```text
D:\OpenOCD-20240916-0.12.0\bin\openocd.exe
D:\OpenOCD-20240916-0.12.0\share\openocd\scripts
```

如果你的 OpenOCD 安装在其他目录，需要修改两个文件：

```text
.vscode/tasks.json
.vscode/launch.json
```

把里面的：

```text
D:\OpenOCD-20240916-0.12.0
```

替换成你的实际安装路径。

验证：

```powershell
& "D:\OpenOCD-20240916-0.12.0\bin\openocd.exe" --version
```

### 8. 检查完整环境

打开 VS Code，确认当前打开的是本工程根目录，也就是包含这些文件的目录：

```text
CMakeLists.txt
CMakePresets.json
stm32F407IGT6.ioc
Core/
Drivers/
Middlewares/
```

打开 VS Code 终端：

```text
Terminal -> New Terminal
```

执行：

```powershell
cmake --version
ninja --version
arm-none-eabi-gcc --version
```

推荐输出：

```text
cmake version 3.26.0 或更新
1.13.2
arm-none-eabi-gcc 12.2.1
```

如果 PowerShell 出现：

```text
无法加载 profile.ps1，因为在此系统上禁止运行脚本
```

这不是编译错误。工程的 `.vscode/settings.json` 已经配置了 `PowerShell -NoProfile`。关闭旧终端，重新打开 VS Code 终端即可。

## CubeMX 生成工程

打开：

```text
stm32F407IGT6.ioc
```

在 Project Manager 中选择：

```text
Toolchain / IDE: CMake
Default Compiler: GCC
```

然后点击：

```text
Generate Code
```

CubeMX 会生成或更新：

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
    # Add user sources here
    Core/Src/motor_pwm.c
)
```

CubeMX 重新生成后，要检查这行是否还在。

## 编译

第一次配置或 CubeMX 重新生成后：

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

如果需要生成 `.bin` 文件：

```powershell
arm-none-eabi-objcopy -O binary build/Debug/stm32F407IGT6.elf build/Debug/stm32F407IGT6.bin
```

生成后会得到：

```text
build/Debug/stm32F407IGT6.bin
```

也可以在 VS Code 中运行任务：

```text
Ctrl + Shift + P
Tasks: Run Task
Build Debug
```

## 烧录

本工程使用：

```text
OpenOCD + CMSIS-DAP + SWD
```

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
Ctrl + Shift + P
Tasks: Run Task
Flash STM32F407
```

`Flash STM32F407` 会先执行 `Build Debug`，再烧录。

如果 OpenOCD 连不上，可以把速度从 4000 降低到 1000：

```text
adapter speed 1000
```

需要修改：

```text
.vscode/tasks.json
.vscode/launch.json
```

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

或者按：

```text
F5
```

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

如果调试失败后有残留进程，可以在 VS Code 终端执行：

```powershell
Get-Process openocd,arm-none-eabi-gdb -ErrorAction SilentlyContinue
Get-Process openocd,arm-none-eabi-gdb -ErrorAction SilentlyContinue | Stop-Process -Force
```

## VS Code 文件说明

```text
.vscode/settings.json
```

配置当前工程的 PATH，让 VS Code 优先使用 Arm GNU Toolchain 12.2 和 Ninja。

```text
.vscode/tasks.json
```

包含：

```text
Build Debug
Flash STM32F407
```

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
