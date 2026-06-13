#ifndef STM32_HALL_SENSOR_HPP
#define STM32_HALL_SENSOR_HPP

#include "Sensor/hall_sensor_base.hpp"

class Stm32HallSensor : public HallSensorBase {
 public:
  void Init() override;
  uint32_t ReadState() override;
};

Stm32HallSensor &GetStm32HallSensor();

#endif /* STM32_HALL_SENSOR_HPP */
