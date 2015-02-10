#include "imu.h"
#include "mpu6050.h"
#include "MedianFilter.h"

static axis_float_t gyro_rates;
static axis_float_t gyro_angles;
static axis_float_t accel_raws;
static axis_float_t accel_filtered;
static axis_float_t accel_angles;
static axis_float_t rates;
static axis_float_t angles;
static uint32_t gyro_update_timer = micros();
static uint32_t accel_update_timer = millis();
static uint32_t combination_update_timer = micros();
static MedianFilter accel_x_filter(11, 0.0);
static MedianFilter accel_y_filter(11, 0.0);
static MedianFilter accel_z_filter(11, 0.0);

void imu_init() {
  Wire.begin();
  TWBR = 10;  // setup i2c to run at 444 kHz
  mpu6050_init();
}

void update_gyro() {
  mpu6050_read_gyro(&gyro_rates);

  rates.x = gyro_rates.x + GYRO_X_OFFSET;
  rates.y = gyro_rates.y + GYRO_Y_OFFSET;
  rates.z = gyro_rates.z + GYRO_Z_OFFSET;

  // Integration of gyro rates to get the angles
  // for debugging only
  gyro_angles.x += rates.x * (float)(micros() - gyro_update_timer) / 1000000;
  gyro_angles.y += rates.y * (float)(micros() - gyro_update_timer) / 1000000;
}

void update_accel() {
  mpu6050_read_accel(&accel_raws);

  accel_filtered.x = accel_x_filter.in(accel_raws.x + ACCEL_X_OFFSET);
  accel_filtered.y = accel_y_filter.in(accel_raws.y + ACCEL_Y_OFFSET);
  accel_filtered.z = accel_z_filter.in(accel_raws.z + ACCEL_Z_OFFSET);
}

void combine() {
  //Angle calculation through Complementary filter

  float dt = (float)(micros() - combination_update_timer) / 1000000.0;

  accel_angles.x = atan2(accel_filtered.y, accel_filtered.z) * RAD_TO_DEG;
  accel_angles.y = atan2(-1 * accel_filtered.x,
    sqrt(accel_filtered.y * accel_filtered.y + accel_filtered.z * accel_filtered.z)
  ) * RAD_TO_DEG;

  angles.x = GYRO_PART * (angles.x + (rates.x * dt)) + ACC_PART * accel_angles.x;
  angles.y = GYRO_PART * (angles.y + (rates.y * dt)) + ACC_PART * accel_angles.y;

  combination_update_timer = micros();
}

bool imu_read() {
  bool updated = false;

  if ((micros() - gyro_update_timer) > 2000) {   // ~500 Hz
    update_gyro();
    gyro_update_timer = micros();

    update_accel();
    accel_update_timer = millis();
    updated = true;
  }

  //if ((millis() - accel_update_timer) > 2000) {    // ~100 hz
    //updated = true;
  //}

  if (updated == true) {
    combine();
  }

  return updated;
}

axis_float_t imu_rates() { return rates; }
axis_float_t imu_angles() { return angles; }
axis_float_t imu_gyro_angles() { return gyro_angles; }
axis_float_t imu_gyro_rates() { return gyro_rates; }
axis_float_t imu_accel_raws() { return accel_raws; }
axis_float_t imu_accel_filtered() { return accel_filtered; }
axis_float_t imu_accel_angles() { return accel_angles; }