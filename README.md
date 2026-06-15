# ESPHome Omron D7S

ESPHome external component for the **Omron D7S seismic sensor** / **Grove D7S Vibration Sensor**.

This component exposes the D7S over I²C to Home Assistant using ESPHome binary sensors and numeric sensors.

> Status: experimental / community component. Do not use as a life-safety system.

## 1. Introduction

The Omron D7S is a seismic sensor intended to detect earthquake-related vibration and estimated collapse conditions. This ESPHome component reads the D7S directly over I²C and publishes useful states into Home Assistant.

Supported entities:

| Entity type | Entity | Description |
| --- | --- | --- |
| Binary sensor | `earthquake_occurring` | True while the D7S reports active earthquake motion |
| Binary sensor | `collapse_occurring` | True when the D7S reports a collapse event flag |
| Binary sensor | `shutoff_occurring` | True when the D7S reports a shutoff event flag |
| Sensor | `instantaneous_si` | Instantaneous SI value, published as `m/s` |
| Sensor | `instantaneous_pga` | Instantaneous PGA value, published as `m/s²` |

The implementation is based on the same register behavior exposed by Omron's Arduino library, but uses ESPHome's native I²C abstraction instead of directly depending on Arduino `Wire`.

## 2. Hardware

### Tested/target hardware

- ESP32 development board
- Omron D7S / Grove D7S Vibration Sensor
- I²C wiring

### Default ESP32 wiring

| D7S / Grove pin | ESP32 pin |
| --- | --- |
| VCC | 3.3V preferred |
| GND | GND |
| SDA | GPIO21 |
| SCL | GPIO22 |

The Grove D7S module is commonly documented as supporting 3.3V and 5V operation, but for ESP32 use 3.3V unless your breakout board explicitly provides I²C level shifting.

## 3. Installation

### Option B: Hosted Git repository

Add this to your ESPHome YAML:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/YOUR_USERNAME/esphome-d7s
      ref: main
    components: [d7s]
    refresh: 1d
```

### Option A: Local development

Clone or copy this repository next to your ESPHome YAML, then reference the local `components` folder:

```yaml
external_components:
  - source:
      type: local
      path: ../components
    components: [d7s]
```


## 4. Example configuration

See [`examples/d7s-example.yaml`](examples/d7s-example.yaml).

Minimal component configuration:

```yaml
i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true
  frequency: 100kHz

d7s:
  id: d7s_sensor
  address: 0x55
  update_interval: 1s
  initialize_on_boot: false
  reset_events_each_cycle: true
  axis_mode: switch_at_installation

binary_sensor:
  - platform: d7s
    d7s_id: d7s_sensor
    earthquake_occurring:
      name: "Earthquake Occurring"
    collapse_occurring:
      name: "Collapse Occurring"
    shutoff_occurring:
      name: "D7S Shutoff Event"

sensor:
  - platform: d7s
    d7s_id: d7s_sensor
    instantaneous_si:
      name: "Instantaneous SI"
    instantaneous_pga:
      name: "Instantaneous PGA"
```

## 5. Configuration reference

### `d7s:` hub

| Option | Required | Default | Description |
| --- | --- | --- | --- |
| `id` | yes | — | ESPHome ID used by the binary sensor and sensor platforms |
| `address` | no | `0x55` | D7S I²C address |
| `update_interval` | no | `1s` | Poll interval |
| `initialize_on_boot` | no | `false` | Runs D7S initial installation at boot |
| `reset_events_each_cycle` | no | `true` | Clears latched event flags once no earthquake is occurring |
| `axis_mode` | no | `switch_at_installation` | D7S axis setting |

### Axis modes

| YAML value | Meaning |
| --- | --- |
| `force_yz` | Force YZ plane |
| `force_xz` | Force XZ plane |
| `force_xy` | Force XY plane |
| `auto_switch` | Automatic axis switching |
| `switch_at_installation` | Select axis during initial installation |

## 6. First-time installation / calibration

For first-time mounting, temporarily enable:

```yaml
d7s:
  id: d7s_sensor
  initialize_on_boot: true
```

Flash the ESP32 and keep the D7S firmly mounted and motionless during boot. Once initialization completes, set it back to:

```yaml
initialize_on_boot: false
```

Then reflash. This avoids re-running installation every time the device restarts.

## 7. Home Assistant behavior

The numeric sensors are published as `0` when no active earthquake is occurring. During an active event, `instantaneous_si` and `instantaneous_pga` are read from the D7S.

`collapse_occurring` and `shutoff_occurring` are latched internally so brief D7S event flags are less likely to be missed by Home Assistant. With `reset_events_each_cycle: true`, those latched flags are cleared after the earthquake state returns to false.

## 8. Repository layout

```text
components/
  d7s/
    __init__.py
    binary_sensor.py
    sensor.py
    d7s.h
    d7s.cpp
examples/
  d7s-example.yaml
README.md
LICENSE
.gitignore
```

## 9. Known limitations

- It does not currently expose historical SI/PGA values.
- It does not currently expose a manual reset action/switch.
- It assumes the Omron/Grove D7S default I²C register behavior.

## 10. Credits

Inspired by Omron's D7S Grove Arduino library and ESPHome's external component architecture.

## 11. License

MIT. See [`LICENSE`](LICENSE).
