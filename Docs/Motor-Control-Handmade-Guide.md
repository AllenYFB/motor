# 电机库手撕与持续迭代指南

这份文档记录当前电机库的写法、为什么这样拆层、以后继续往 ODrive 风格优化时应该从哪里下手。

当前目标不是一次性写成完整 FOC，而是先把工程做成可以持续演进的结构：上层控制逻辑不依赖 STM32 细节，底层端口代码负责定时器、GPIO、ADC、驱动芯片这些硬件差异。

## 当前分层

```text
UserApp/Tasks
  用户任务、按键、周期调用

Ctrl/Motor
  电机控制算法和状态机

Ctrl/Driver
  电机功率输出抽象接口

Ctrl/Sensor
  Hall、编码器等传感器抽象接口

Port
  STM32F407 具体硬件适配

Core/Drivers/Middlewares
  CubeMX 和 HAL 生成代码
```

写代码时优先遵守这个边界：

```text
算法层不要直接 include tim.h / gpio.h / main.h
硬件寄存器和 HAL 调用尽量只放在 Port 层
CubeMX 生成代码少改，必须改时写清楚原因
```

这样以后换芯片、换驱动器、从六步换成 FOC 时，不会把整个工程搅在一起。

## 当前六步控制方式

当前控制器是：

```text
Hall 传感器 -> 六步换相 -> TIM1 互补 PWM -> 三相桥
```

TIM1 使用：

```text
CH1  -> U 高边
CH1N -> U 低边
CH2  -> V 高边
CH2N -> V 低边
CH3  -> W 高边
CH3N -> W 低边
```

现在不是低边 GPIO 了，而是 TIM1 的互补输出。好处是：

```text
死区由硬件插入
高低边时序更稳定
换相时只改 CCR 和 CCER
后续容易接 ADC 同步采样和 FOC
```

## PWM 驱动接口怎么写

抽象接口在：

```text
Ctrl/Driver/motor_pwm_driver.hpp
```

它不暴露 STM32 的 `TIM_CHANNEL_1`、`CCER`、`htim1`，只暴露电机控制需要的能力：

```cpp
class MotorPwmDriver {
 public:
  virtual ~MotorPwmDriver() = default;

  virtual void InitOutputs() = 0;
  virtual MotorStatus StartPwmChannels() = 0;
  virtual void EnableMainOutput() = 0;
  virtual void DisableMainOutput() = 0;
  virtual void WriteGate(bool enable) = 0;
  virtual void SetPhaseOutputs(bool uHigh, bool vHigh, bool wHigh,
                               bool uLow, bool vLow, bool wLow) = 0;
  virtual void SetCompareRaw(uint32_t compareU, uint32_t compareV, uint32_t compareW) = 0;
  virtual uint32_t ReadPeriod() = 0;
};
```

手撕时要记住：

```text
SetCompareRaw() 负责占空比
SetPhaseOutputs() 负责哪几个桥臂允许输出
EnableMainOutput()/DisableMainOutput() 负责 TIM1 的 MOE 总开关
WriteGate() 负责驱动芯片使能脚
```

## STM32 端口层怎么写

STM32 具体实现放在：

```text
Port/stm32_motor_pwm.cpp
```

启动互补 PWM 要同时启动主输出和 N 输出：

```cpp
HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
```

三个通道都要启动：

```cpp
StartPwmPair(TIM_CHANNEL_1);
StartPwmPair(TIM_CHANNEL_2);
StartPwmPair(TIM_CHANNEL_3);
```

通道使能通过 `CCER`：

```cpp
TIM_CCER_CC1E   // U 高边 CH1
TIM_CCER_CC1NE  // U 低边 CH1N
TIM_CCER_CC2E
TIM_CCER_CC2NE
TIM_CCER_CC3E
TIM_CCER_CC3NE
```

不要在运行时把 `PB13/PB14/PB15` 再初始化成普通 GPIO。它们已经由 CubeMX 配成 TIM1 复用输出。

## 六步换相怎么写

六步控制器在：

```text
Ctrl/Motor/six_step_bldc_controller.cpp
```

当前采用普通六步 PWM，不使用同步整流：

```text
一个高边 PWM
一个低边常导通
第三相悬空
```

例如 `UhVl()`：

```cpp
void SixStepBldcController::UhVl()
{
  SetCompare(duty_, GetPeriod(), 0U);
  driver_.SetPhaseOutputs(true, false, false, false, true, false);
}
```

含义：

```text
U 高边 PWM，占空比 duty_
V 低边常导通，compare 设置为 period
W 相悬空，高低边都关闭
```

六个状态：

```text
UhVl: U 高边 PWM，V 低边导通
UhWl: U 高边 PWM，W 低边导通
VhWl: V 高边 PWM，W 低边导通
VhUl: V 高边 PWM，U 低边导通
WhUl: W 高边 PWM，U 低边导通
WhVl: W 高边 PWM，V 低边导通
```

低边为什么用 `GetPeriod()` 而不是 `duty_`：

```text
低边不是调压对象，它是当前扇区的回流开关。
高边用 duty_ 控制平均电压。
低边保持稳定导通，电流回路更明确。
```

## CubeMX 里必须确认的配置

TIM1：

```text
Counter Mode: Center Aligned
CH1/CH2/CH3: PWM Generation
CH1N/CH2N/CH3N: enabled
DeadTime: non-zero
MOE: software enable
```

当前为了避免 BKIN 浮空导致不输出，Break 暂时关闭：

```c
sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
```

后续如果要恢复硬件保护，必须先确认：

```text
PB12 / TIM1_BKIN 电平不会误触发
Break 极性和驱动故障输出极性一致
示波器确认触发后所有 PWM 关闭
```

## 每次修改后的验证顺序

不要一改完就大电流上电。建议按这个顺序：

1. 编译

```powershell
cmake --build --preset Debug
```

2. 烧录

```powershell
STM32_Programmer_CLI -c port=SWD -w build\Debug\stm32F407IGT6.elf -v -rst
```

3. 不接电机或低压限流，用示波器看 PWM

```text
UH/UL 不应同时导通
VH/VL 不应同时导通
WH/WL 不应同时导通
死区宽度要符合 MOS 和驱动芯片需求
```

4. 低占空比带电机测试

```text
先用 kMotorPwmStartDuty
观察电流、温升、是否抖动
再逐步提高 duty
```

5. 出现不转时先查这几个点

```text
PM1_CTRL_SD 是否使能
TIM1 MOE 是否被清掉
BKIN 是否误触发
Hall 状态是否为 1~6
CCER 是否打开了目标 CHx/CHxN
CCR 是否符合当前换相状态
```

## 后续持续迭代路线

建议按这个顺序推进，不要同时改太多层。

### 0. 已完成：控制循环迁移到 TIM1 更新中断

当前六步换相已经不再由 `UserMotorTask` 的 `osDelay(1)` 轮询触发，而是由 TIM1 更新中断触发：

```text
TIM1_UP_TIM10_IRQHandler
-> HAL_TIM_IRQHandler(&htim1)
-> MotorControlOnPwmTimerTick()
-> GetMotorController().Commutate()
```

相关文件：

```text
Core/Src/stm32f4xx_it.c
UserApp/Tasks/Inc/userMotorTask.hpp
UserApp/Tasks/Src/userMotorTask.cpp
```

`UserMotorTask` 暂时保留为空闲低频任务，后续可以放：

```text
状态监控
错误上报
低频通信
参数保存
调试统计
```

不要再在 `UserMotorTask` 里周期性调用 `Commutate()`，否则会出现任务和中断同时更新 PWM 的竞争。

### 1. 完善错误和状态

增加：

```text
MotorError bitmask
Idle / Run / Error 状态
GetError()
ClearErrors()
```

目标是任何停机都能知道原因，而不是只看到“不转”。

### 2. 把 Hall 解码独立出来

现在 Hall 逻辑还在六步控制器里。后续可以拆成：

```text
HallSensorBase 只读原始 Hall
HallDecoder 负责 001/011/010/110/100/101 解码
MotorController 只关心电角度扇区
```

### 3. 控制周期从 osDelay 迁到 TIM/ADC

当前 `UserMotorTask` 是 FreeRTOS 任务轮询。要提高实时性，应逐步变成：

```text
TIM1 更新事件
-> ADC 采样
-> 控制循环
-> 更新 CCR
```

这是 ODrive 的关键思路。

### 4. 增加电流采样

先做：

```text
ADC DMA 采样
母线电压
相电流零点校准
过流保护
```

再做：

```text
电流环 PI
Clarke/Park
SVPWM
FOC
```

### 5. 引入 Axis / Motor / Controller 分层

ODrive 的核心不是某个函数，而是分层清晰：

```text
Axis: 状态机和任务链
Motor: 功率级、电流环、PWM 输出
Controller: 位置/速度/力矩命令
Encoder/Hall: 位置估计
```

你当前工程可以逐步演进为：

```text
MotorAxis
  owns Controller
  owns MotorPowerStage
  owns Sensor
```

不要一次性搬 ODrive，先把接口边界学过来。

## 修改代码的基本原则

每次只改一个目标：

```text
一次只改 PWM 输出
一次只改 Hall 解码
一次只改状态机
一次只改 ADC 采样
```

每次改完都要有：

```text
能编译
能低压验证
知道怎么回退
文档记录为什么这么改
```

建议提交粒度：

```text
Use complementary PWM for BLDC phases
Add motor error state tracking
Move commutation update to timer callback
Add ADC current sampling skeleton
```

这样的提交以后排查问题会轻松很多。
