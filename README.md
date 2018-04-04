# HTU21D(F) I2C Driver

A Mongoose library for Measurement Specialities HTU21D(F) integrated circuit.

## Sensor details

The HTU21D(F) is a new digital humidity sensor with temperature output by MEAS.
Setting new standards in terms of size and intelligence, it is embedded in a
reflow solderable Dual Flat No leads (DFN) package with a small 3 x 3 x 0.9 mm
footprint. This sensor provides calibrated, linearized signals in digital, I2C
format.

HTU21D(F) digital humidity sensors are dedicated humidity and temperature plug
and play transducers for OEM applications where reliable and accurate
measurements are needed. Direct interface with a micro-controller is made
possible with the module for humidity and temperature digital outputs. These
low power sensors are designed for high volume and cost sensitive applications
with tight space constraints.

Every sensor is individually calibrated and tested. Lot identification is
printed on the sensor and an electronic identification code is stored on the
chip – which can be read out by command. Low battery can be detected and a
checksum improves communication reliability. The resolution of these digital
humidity sensors can be changed by command (8/12bit up to 12/14bit for RH/T).

With MEAS’ improvements and miniaturization of this sensor, the
performance-to-price ratio has been improved – and eventually, any device
should benefit from its cutting edge energy saving operation mode. Optional
PTFE filter/membrane (F) protects HTU21D digital humidity sensors against
dust and water immersion, as well as against contamination by particles.
PTFE filter/membranes preserve a high response time. The white PTFE
filter/membrane is directly stuck on the sensor housing.

See [datasheet](https://cdn-shop.adafruit.com/datasheets/1899_HTU21D.pdf)
for implementation details.

A great place to buy a ready made and tested unit is at [Adafruit](https://learn.adafruit.com/adafruit-htu21d-f-temperature-humidity-sensor/overview).

## Example application

An example program using a timer to read data from the sensor every 5 seconds:

```
#include "mgos.h"
#include "mgos_i2c.h"
#include "mgos_htu21df.h"

static struct mgos_htu21df *s_htu21df;

static void timer_cb(void *user_data) {
  float temperature, humidity;

  temperature=mgos_htu21df_getTemperature(s_htu21df);
  humidity=mgos_htu21df_getHumidity(s_htu21df);

  LOG(LL_INFO, ("htu21df temperature=%.2f humidity=%.2f", temperature, humidity));

  (void) user_data;
}

enum mgos_app_init_result mgos_app_init(void) {
  struct mgos_i2c *i2c;

  i2c=mgos_i2c_get_global();
  if (!i2c) {
    LOG(LL_ERROR, ("I2C bus missing, set i2c.enable=true in mos.yml"));
  } else {
    s_htu21df=mgos_htu21df_create(i2c, 0x40); // Default I2C address
    if (s_htu21df) {
      mgos_set_timer(5000, true, timer_cb, NULL);
    } else {
      LOG(LL_ERROR, ("Could not initialize sensor"));
    }
  }
  return MGOS_APP_INIT_SUCCESS;
}
```

# Disclaimer

This project is not an official Google project. It is not supported by Google
and Google specifically disclaims all warranties as to its quality,
merchantability, or fitness for a particular purpose.
