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

#include "mgos.h"
#include "mgos_htu21df_internal.h"
#include "mgos_i2c.h"

// Datasheet:
// https://cdn-shop.adafruit.com/datasheets/1899_HTU21D.pdf

// Private functions follow
static bool mgos_htu21df_cmd(struct mgos_htu21df *sensor, uint8_t cmd) {
  if (!sensor || !sensor->i2c) {
    return false;
  }

  if (!mgos_i2c_write(sensor->i2c, sensor->i2caddr, &cmd, 1, true)) {
    LOG(LL_ERROR, ("I2C=0x%02x cmd=%u (0x%02x) write error", sensor->i2caddr, cmd, cmd));
    return false;
  }
  LOG(LL_DEBUG, ("I2C=0x%02x cmd=%u (0x%02x) write success", sensor->i2caddr, cmd, cmd));
  return true;
}

static uint8_t crc8(const uint8_t *data, int len) {
  const uint8_t poly = 0x31;
  uint8_t       crc  = 0x00;

  for (int j = len; j; --j) {
    crc ^= *data++;
    for (int i = 8; i; --i) {
      crc = (crc & 0x80) ? (crc << 1) ^ poly : (crc << 1);
    }
  }
  return crc;
}

// Private functions end

// Public functions follow
struct mgos_htu21df *mgos_htu21df_create(struct mgos_i2c *i2c, uint8_t i2caddr) {
  struct mgos_htu21df *sensor;
  uint8_t version;

  if (!i2c) {
    return NULL;
  }

  sensor = calloc(1, sizeof(struct mgos_htu21df));
  if (!sensor) {
    return NULL;
  }

  memset(sensor, 0, sizeof(struct mgos_htu21df));
  sensor->i2caddr = i2caddr;
  sensor->i2c     = i2c;

  mgos_htu21df_cmd(sensor, MGOS_HTU21DF_RESET);
  mgos_usleep(25000);

  mgos_htu21df_cmd(sensor, MGOS_HTU21DF_READREG);
  if (!mgos_i2c_read(sensor->i2c, sensor->i2caddr, &version, 1, true)) {
    LOG(LL_ERROR, ("Could not read command"));
    free(sensor);
    return NULL;
  }
  if (version == 0x02) {
    LOG(LL_INFO, ("HTU21DF created at I2C 0x%02x", i2caddr));
    return sensor;
  }

  LOG(LL_ERROR, ("Failed to create HTU21DF at I2C 0x%02x", i2caddr));
  free(sensor);
  return NULL;
}

void mgos_htu21df_destroy(struct mgos_htu21df **sensor) {
  if (!*sensor) {
    return;
  }

  free(*sensor);
  *sensor = NULL;
  return;
}

bool mgos_htu21df_read(struct mgos_htu21df *sensor) {
  double start = mg_time();

  if (!sensor || !sensor->i2c) {
    return false;
  }

  sensor->stats.read++;

  if (start - sensor->stats.last_read_time < MGOS_HTU21DF_READ_DELAY) {
    sensor->stats.read_success_cached++;
    return true;
  }

  // Read out sensor data here
  //
  uint8_t data[3];

  mgos_htu21df_cmd(sensor, MGOS_HTU21DF_READTEMP);
  mgos_usleep(50000);
  if (!mgos_i2c_read(sensor->i2c, sensor->i2caddr, data, 3, true)) {
    LOG(LL_ERROR, ("Could not read command"));
    return false;
  }
  if (data[2] != crc8(data, 2)) {
    LOG(LL_ERROR, ("CRC error on temperature data"));
    return false;
  }

  uint16_t temp        = (data[0] << 8) + data[1];
  float    temperature = temp;
  temperature        *= 175.72;
  temperature        /= 65536;
  temperature        -= 46.85;
  sensor->temperature = temperature;

  mgos_htu21df_cmd(sensor, MGOS_HTU21DF_READHUM);
  mgos_usleep(50000);
  if (!mgos_i2c_read(sensor->i2c, sensor->i2caddr, data, 3, true)) {
    LOG(LL_ERROR, ("Could not read command"));
    return false;
  }
  if (data[2] != crc8(data, 2)) {
    LOG(LL_ERROR, ("CRC error on temperature data"));
    return false;
  }

  uint16_t hum      = (data[0] << 8) + data[1];
  float    humidity = hum;
  humidity        *= 125;
  humidity        /= 65536;
  humidity        -= 6;
  sensor->humidity = humidity;

  LOG(LL_DEBUG, ("temperature=%.2fC humidity=%.1f%%", sensor->temperature, sensor->humidity));
  sensor->stats.read_success++;
  sensor->stats.read_success_usecs += 1000000 * (mg_time() - start);
  sensor->stats.last_read_time      = start;
  return true;
}

float mgos_htu21df_getTemperature(struct mgos_htu21df *sensor) {
  if (!mgos_htu21df_read(sensor)) {
    return NAN;
  }

  return sensor->temperature;
}

float mgos_htu21df_getHumidity(struct mgos_htu21df *sensor) {
  if (!mgos_htu21df_read(sensor)) {
    return NAN;
  }

  return sensor->humidity;
}

bool mgos_htu21df_getStats(struct mgos_htu21df *sensor, struct mgos_htu21df_stats *stats) {
  if (!sensor || !stats) {
    return false;
  }

  memcpy((void *)stats, (const void *)&sensor->stats, sizeof(struct mgos_htu21df_stats));
  return true;
}

bool mgos_htu21df_i2c_init(void) {
  return true;
}

// Public functions end
