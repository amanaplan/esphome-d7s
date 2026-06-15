#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace d7s {

class D7SComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_initialize_on_boot(bool value) { this->initialize_on_boot_ = value; }
  void set_reset_events_each_cycle(bool value) { this->reset_events_each_cycle_ = value; }
  void set_axis_mode(uint8_t value) { this->axis_mode_ = value; }

  void set_earthquake_occurring_binary_sensor(binary_sensor::BinarySensor *sensor) {
    this->earthquake_occurring_binary_sensor_ = sensor;
  }
  void set_collapse_occurring_binary_sensor(binary_sensor::BinarySensor *sensor) {
    this->collapse_occurring_binary_sensor_ = sensor;
  }
  void set_shutoff_occurring_binary_sensor(binary_sensor::BinarySensor *sensor) {
    this->shutoff_occurring_binary_sensor_ = sensor;
  }
  void set_instantaneous_si_sensor(sensor::Sensor *sensor) { this->instantaneous_si_sensor_ = sensor; }
  void set_instantaneous_pga_sensor(sensor::Sensor *sensor) { this->instantaneous_pga_sensor_ = sensor; }

 protected:
  bool initialize_on_boot_{false};
  bool reset_events_each_cycle_{true};
  uint8_t axis_mode_{0x04};
  uint8_t latched_events_{0};

  binary_sensor::BinarySensor *earthquake_occurring_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *collapse_occurring_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *shutoff_occurring_binary_sensor_{nullptr};
  sensor::Sensor *instantaneous_si_sensor_{nullptr};
  sensor::Sensor *instantaneous_pga_sensor_{nullptr};

  bool read_u8_(uint8_t reg_h, uint8_t reg_l, uint8_t *value);
  bool read_u16_(uint8_t reg_h, uint8_t reg_l, uint16_t *value);
  bool write_u8_(uint8_t reg_h, uint8_t reg_l, uint8_t value);

  bool get_state_(uint8_t *state);
  bool is_ready_();
  bool is_earthquake_occurring_();
  bool read_events_();
  bool reset_events_();
  bool set_axis_();
  bool start_initial_installation_();

  float read_instantaneous_si_();
  float read_instantaneous_pga_();

  void publish_safe_binary_(binary_sensor::BinarySensor *sensor, bool value);
  void publish_safe_sensor_(sensor::Sensor *sensor, float value);
};

}  // namespace d7s
}  // namespace esphome
