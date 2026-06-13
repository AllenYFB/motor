#ifndef HALL_SENSOR_BASE_HPP
#define HALL_SENSOR_BASE_HPP

#include <cstdint>

class HallSensorBase {
 public:
  virtual ~HallSensorBase() = default;

  virtual void Init() = 0;
  virtual uint32_t ReadState() = 0;
};

#endif /* HALL_SENSOR_BASE_HPP */
