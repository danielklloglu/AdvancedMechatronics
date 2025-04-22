#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C setup
#define I2C_PORT i2c0
#define SDA_PIN 8
#define SCL_PIN 9
#define MCP23008_ADDR 0x20

// MCP23008 register addresses
#define IODIR 0x00
#define OLAT  0x0A
#define GPIO  0x09

uint8_t output_state = 0x00;

// Set the state of a specific GP pin
void setPin(uint8_t pin, bool on) {
    if (on) {
        output_state |= (1 << pin);
    } else {
        output_state &= ~(1 << pin);
    }

    uint8_t buf[2] = {OLAT, output_state};
    i2c_write_blocking(I2C_PORT, MCP23008_ADDR, buf, 2, false);
}

// Read the state of a specific input pin (returns 0 or 1)
uint8_t readPin(uint8_t pin) {
    uint8_t data;
    uint8_t reg = GPIO;
    i2c_write_blocking(I2C_PORT, MCP23008_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MCP23008_ADDR, &data, 1, false);

    return (data >> pin) & 0x01;
}

int main() {
    stdio_init_all();

    // Initialize I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    // Onboard LED for heartbeat
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    // GP0 as input (1), GP7 as output (0), others as input (1)
    uint8_t iodir_buf[2] = {IODIR, 0b01111111}; // GP7 = 0 (output), rest = 1 (input)
    i2c_write_blocking(I2C_PORT, MCP23008_ADDR, iodir_buf, 2, false);

    while (true) {
        gpio_put(25, 1);  // Heartbeat ON

        uint8_t button = readPin(0); // Read GP0

        // Button logic: LOW = pressed
        if (button == 0) {
            setPin(7, true);  // Turn on GP7
        } else {
            setPin(7, false); // Turn off GP7
        }
        
        sleep_ms(100);
        gpio_put(25, 0);  // Heartbeat OFF
        sleep_ms(100);
    }
}
