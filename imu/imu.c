#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "oledsupport.h"
#include "font.h"

// I2C setup for both OLED and IMU
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define MPU_ADDR 0x68

// MPU6050 Registers
#define PWR_MGMT_1     0x6B
#define ACCEL_CONFIG   0x1C
#define GYRO_CONFIG    0x1B
#define WHO_AM_I       0x75
#define ACCEL_XOUT_H   0x3B

void mpu6050_init() {
    uint8_t buf[] = {PWR_MGMT_1, 0x00};
    i2c_write_blocking(I2C_PORT, MPU_ADDR, buf, 2, false);

    uint8_t accel_cfg[] = {ACCEL_CONFIG, 0x00}; // ±2g
    i2c_write_blocking(I2C_PORT, MPU_ADDR, accel_cfg, 2, false);

    uint8_t gyro_cfg[] = {GYRO_CONFIG, 0x18}; // ±2000dps
    i2c_write_blocking(I2C_PORT, MPU_ADDR, gyro_cfg, 2, false);
}

bool mpu6050_check_whoami() {
    uint8_t reg = WHO_AM_I;
    uint8_t data;
    i2c_write_blocking(I2C_PORT, MPU_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU_ADDR, &data, 1, false);
    return (data == 0x68 || data == 0x98);
}

void mpu6050_read_accel(int16_t* ax, int16_t* ay, int16_t* az) {
    uint8_t reg = ACCEL_XOUT_H;
    uint8_t data[6];
    i2c_write_blocking(I2C_PORT, MPU_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU_ADDR, data, 6, false);
    *ax = (data[0] << 8) | data[1];
    *ay = (data[2] << 8) | data[3];
    *az = (data[4] << 8) | data[5];
}

// Bresenham's line drawing algorithm for OLED
void drawLine(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        if (x0 >= 0 && x0 < 128 && y0 >= 0 && y0 < 32) {
            ssd1306_drawPixel(x0, y0, 1);
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

int main() {
    stdio_init_all();

    // I2C init
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    sleep_ms(500);

    // OLED init
    ssd1306_setup();

    // MPU6050 check/init
    if (!mpu6050_check_whoami()) {
        ssd1306_clear();
        ssd1306_update();
        while (1) sleep_ms(1000);
    }
    mpu6050_init();

    // Loop
    int16_t ax, ay, az;
    const int centerX = 64, centerY = 16;
    const float scale = 25.0f;

    while (1) {
        mpu6050_read_accel(&ax, &ay, &az);

        float ax_g = ax * 0.000061f;
        float ay_g = ay * 0.000061f;

        int x_end = centerX + (int)(ay_g * scale);  // Horizontal
        int y_end = centerY + (int)(ax_g * scale);  // Vertical

        ssd1306_clear();
        drawLine(centerX, centerY, x_end, y_end);
        ssd1306_update();

        sleep_ms(10); // ~100Hz
    }

    return 0;
}
