#include "d7s.h"

namespace esphome {
namespace d7s {

static const char *const TAG = "d7s";

static constexpr uint8_t REG_STATE_H = 0x10;
static constexpr uint8_t REG_STATE_L = 0x00;
static constexpr uint8_t REG_EVENT_H = 0x10;
static constexpr uint8_t REG_EVENT_L = 0x02;
static constexpr uint8_t REG_COMMAND_H = 0x10;
static constexpr uint8_t REG_COMMAND_L = 0x03;
static constexpr uint8_t REG_CTRL_H = 0x10;
static constexpr uint8_t REG_CTRL_L = 0x04;
static constexpr uint8_t REG_SI_H = 0x20;
static constexpr uint8_t REG_SI_L = 0x00;
static constexpr uint8_t REG_PGA_H = 0x20;
static constexpr uint8_t REG_PGA_L = 0x02;

static constexpr uint8_t NORMAL_MODE = 0x00;
static constexpr uint8_t NORMAL_MODE_NOT_IN_STANDBY = 0x01;
static constexpr uint8_t CMD_INITIAL_INSTALLATION = 0x02;

void D7SComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Omron D7S...");

  uint8_t state = 0;
  if (!this->get_state_(&state)) {
    ESP_LOGE(TAG, "Unable to communicate with D7S at I2C address 0x%02X", this->address_);
    this->mark_failed();
    return;
  }

  ESP_LOGI(TAG, "D7S initial state: 0x%02X", state);

  if (!this->set_axis_())
    ESP_LOGW(TAG, "Failed to set D7S axis mode");

  if (this->initialize_on_boot_) {
    ESP_LOGW(TAG, "Starting D7S initial installation. Keep the sensor firmly mounted and still.");
    if (!this->start_initial_installation_()) {
      ESP_LOGE(TAG, "Failed to start D7S initial installation");
      this->mark_failed();
      return;
    }

    const uint32_t start = millis();
    while (!this->is_ready_()) {
      if (millis() - start > 30000) {
        ESP_LOGE(TAG, "Timed out waiting for D7S initial installation to complete");
        this->mark_failed();
        return;
      }
     delay(250);
     yield();
    }

    ESP_LOGI(TAG, "D7S initial installation complete");
  }

  this->reset_events_();
}

void D7SComponent::update() {
  if (this->is_failed())
    return;

  const bool earthquake = this->is_earthquake_occurring_();
  this->read_events_();

  const bool shutoff = (this->latched_events_ & 0x01) != 0;
  const bool collapse = (this->latched_events_ & 0x02) != 0;

  this->publish_safe_binary_(this->earthquake_occurring_binary_sensor_, earthquake);
  this->publish_safe_binary_(this->shutoff_occurring_binary_sensor_, shutoff);
  this->publish_safe_binary_(this->collapse_occurring_binary_sensor_, collapse);

  if (earthquake) {
    this->publish_safe_sensor_(this->instantaneous_si_sensor_, this->read_instantaneous_si_());
    this->publish_safe_sensor_(this->instantaneous_pga_sensor_, this->read_instantaneous_pga_());
  } else {
    this->publish_safe_sensor_(this->instantaneous_si_sensor_, 0.0f);
    this->publish_safe_sensor_(this->instantaneous_pga_sensor_, 0.0f);

    if (this->reset_events_each_cycle_ && this->latched_events_ != 0)
      this->reset_events_();
  }
}

void D7SComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Omron D7S:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "  Initialize on boot: %s", YESNO(this->initialize_on_boot_));
  ESP_LOGCONFIG(TAG, "  Reset events each non-earthquake cycle: %s", YESNO(this->reset_events_each_cycle_));
  ESP_LOGCONFIG(TAG, "  Axis mode: 0x%02X", this->axis_mode_);
  LOG_BINARY_SENSOR("  ", "Earthquake Occurring", this->earthquake_occurring_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Collapse Occurring", this->collapse_occurring_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Shutoff Occurring", this->shutoff_occurring_binary_sensor_);
  LOG_SENSOR("  ", "Instantaneous SI", this->instantaneous_si_sensor_);
  LOG_SENSOR("  ", "Instantaneous PGA", this->instantaneous_pga_sensor_);
}

bool D7SComponent::read_u8_(uint8_t reg_h, uint8_t reg_l, uint8_t *value) {
  uint8_t reg[2] = {reg_h, reg_l};
  return this->write_read(reg, 2, value, 1);
}

bool D7SComponent::read_u16_(uint8_t reg_h, uint8_t reg_l, uint16_t *value) {
  uint8_t reg[2] = {reg_h, reg_l};
  uint8_t data[2] = {0, 0};
  if (!this->write_read(reg, 2, data, 2))
    return false;
  *value = (static_cast<uint16_t>(data[0]) << 8) | data[1];
  return true;
}

bool D7SComponent::write_u8_(uint8_t reg_h, uint8_t reg_l, uint8_t value) {
  uint8_t data[3] = {reg_h, reg_l, value};
  return this->write(data, 3);
}

bool D7SComponent::get_state_(uint8_t *state) {
  uint8_t raw = 0;
  if (!this->read_u8_(REG_STATE_H, REG_STATE_L, &raw))
    return false;
  *state = raw & 0x07;
  return true;
}

bool D7SComponent::is_ready_() {
  uint8_t state = 0;
  if (!this->get_state_(&state))
    return false;
  return state == NORMAL_MODE;
}

bool D7SComponent::is_earthquake_occurring_() {
  uint8_t state = 0;
  if (!this->get_state_(&state))
    return false;
  return state == NORMAL_MODE_NOT_IN_STANDBY;
}

bool D7SComponent::read_events_() {
  uint8_t events = 0;
  if (!this->read_u8_(REG_EVENT_H, REG_EVENT_L, &events))
    return false;
  this->latched_events_ |= (events & 0x03);
  return true;
}

bool D7SComponent::reset_events_() {
  uint8_t dummy = 0;
  if (!this->read_u8_(REG_EVENT_H, REG_EVENT_L, &dummy))
    return false;
  this->latched_events_ = 0;
  return true;
}

bool D7SComponent::set_axis_() {
  uint8_t reg = 0;
  if (!this->read_u8_(REG_CTRL_H, REG_CTRL_L, &reg))
    return false;
  reg = (this->axis_mode_ << 4) | (reg & 0x0F);
  return this->write_u8_(REG_CTRL_H, REG_CTRL_L, reg);
}

bool D7SComponent::start_initial_installation_() {
  return this->write_u8_(REG_COMMAND_H, REG_COMMAND_L, CMD_INITIAL_INSTALLATION);
}

float D7SComponent::read_instantaneous_si_() {
  uint16_t raw = 0;
  if (!this->read_u16_(REG_SI_H, REG_SI_L, &raw))
    return NAN;
  return static_cast<float>(raw) / 1000.0f;
}

float D7SComponent::read_instantaneous_pga_() {
  uint16_t raw = 0;
  if (!this->read_u16_(REG_PGA_H, REG_PGA_L, &raw))
    return NAN;
  return static_cast<float>(raw) / 1000.0f;
}

void D7SComponent::publish_safe_binary_(binary_sensor::BinarySensor *sensor, bool value) {
  if (sensor != nullptr)
    sensor->publish_state(value);
}

void D7SComponent::publish_safe_sensor_(sensor::Sensor *sensor, float value) {
  if (sensor != nullptr)
    sensor->publish_state(value);
}

}  // namespace d7s
}  // namespace esphome
