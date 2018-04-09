/*
 * Copyright 2018 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "mgos.h"
#include "mgos_i2c.h"

#define MGOS_HTU21DF_READ_DELAY    (2)

#ifdef __cplusplus
extern "C" {
#endif

struct mgos_htu21df;
struct mgos_htu21df_stats {
  double   last_read_time;       // value of mg_time() upon last call to _read()
  uint32_t read;                 // calls to _read()
  uint32_t read_success;         // successful _read()
  uint32_t read_success_cached;  // calls to _read() which were cached
  // Note: read_errors := read - read_success - read_success_cached
  double   read_success_usecs;   // time spent in successful uncached _read()
};


/*
 * Initialize a HTU21DF on the I2C bus `i2c` at address specified in `i2caddr`
 * parameter (default HTU21DF is on address 0x40). The sensor will be polled for
 * validity, upon success a new `struct mgos_htu21df` is allocated and
 * returned. If the device could not be found, NULL is returned.
 */
struct mgos_htu21df *mgos_htu21df_create(struct mgos_i2c *i2c, uint8_t i2caddr);

/*
 * Destroy the data structure associated with a HTU21DF device. The reference
 * to the pointer of the `struct mgos_htu21df` has to be provided, and upon
 * successful destruction, its associated memory will be freed and the pointer
 * set to NULL.
 */
void mgos_htu21df_destroy(struct mgos_htu21df **sensor);

/*
 * The sensor will be polled for its temperature and humidity data. If the poll
 * has occured in the last `MGOS_HTU21DF_READ_DELAY` seconds, the cached data is
 * used (so as not to repeatedly poll the bus upon subsequent calls).
 */
bool mgos_htu21df_read(struct mgos_htu21df *sensor);

/*
 * The sensor will be polled for its temperature and humidity data. If the poll
 * has occured in the last `MGOS_HTU21DF_READ_DELAY` seconds, the cached data is
 * used (so as not to repeatedly poll the bus upon subsequent calls).
 *
 * The return value is the temperature of the sensor in Celsius, or NAN if no
 * data was found.
 */
float mgos_htu21df_getTemperature(struct mgos_htu21df *sensor);

/*
 * The sensor will be polled for its temperature and humidity data. If the poll
 * has occured in the last `MGOS_HTU21DF_READ_DELAY` seconds, the cached data is
 * used (so as not to repeatedly poll the bus upon subsequent calls).
 *
 * The return value is the humidity of the sensor in percent relative humidity,
 * or NAN if no data was found.
 */
float mgos_htu21df_getHumidity(struct mgos_htu21df *sensor);

/*
 * Returns the running statistics on the sensor interaction, the user provides
 * a pointer to a `struct mgos_htu21df_stats` object, which is filled in by
 * this call.
 *
 * Upon success, true is returned. Otherwise, false is returned, in which case
 * the contents of `stats` is undetermined.
 */
bool mgos_htu21df_getStats(struct mgos_htu21df *sensor, struct mgos_htu21df_stats *stats);

/*
 * Initialization function for MGOS -- currently a noop.
 */
bool mgos_htu21df_i2c_init(void);

#ifdef __cplusplus
}
#endif
