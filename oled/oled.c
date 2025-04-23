#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "oledsupport.h"
#include "font.h"
#include <stdio.h>

#define LED_PIN 25

// Draw a single ASCII character at (x, y)
void drawChar(int x, int y, char c) {
    if (c < 0x20 || c > 0x7F) return;
    int index = c - 0x20;
    for (int col = 0; col < 5; col++) {
        char columnBits = ASCII[index][col];
        for (int row = 0; row < 8; row++) {
            int pixel = (columnBits >> row) & 0x01;
            ssd1306_drawPixel(x + col, y + row, pixel);
        }
    }
    for (int row = 0; row < 8; row++) {
        ssd1306_drawPixel(x + 5, y + row, 0);
    }
}

// Draw a string of characters at (x, y)
void drawMessage(int x, int y, const char* msg) {
    while (*msg != '\0') {
        drawChar(x, y, *msg);
        x += 6;
        msg++;
    }
}

int main() {
    stdio_init_all();

    // I2C setup on GP8/9
    i2c_init(i2c0, 400 * 1000);
    gpio_set_function(8, GPIO_FUNC_I2C);
    gpio_set_function(9, GPIO_FUNC_I2C);
    gpio_pull_up(8);
    gpio_pull_up(9);

    // LED setup
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // OLED setup
    ssd1306_setup();

    // ADC setup (ADC0 = GPIO26)
    adc_init();
    adc_gpio_init(26);      // Enable ADC on GPIO26
    adc_select_input(0);    // Select ADC0

    // Loop vars
    bool led_on = false;
    bool pixel_on = false;
    char msg[50];
    char fps_msg[30];
    absolute_time_t prev_time = get_absolute_time();

    while (1) {
        // Blink LED
        gpio_put(LED_PIN, led_on);
        led_on = !led_on;

        // Read ADC0
        uint16_t raw = adc_read();             // 12-bit ADC: 0–4095
        float voltage = (raw * 3.3f) / 4095.0f; // Convert to volts

        // Measure time and FPS
        absolute_time_t now = get_absolute_time();
        uint32_t t_us = to_us_since_boot(now);
        uint32_t dt_us = absolute_time_diff_us(prev_time, now);
        prev_time = now;
        float fps = 1000000.0f / dt_us; // microseconds to seconds

        // Format messages
        sprintf(msg, "ADC0 = %.2f V", voltage);
        sprintf(fps_msg, "FPS = %.1f", fps);

        // Draw screen
        ssd1306_clear();
        drawMessage(0, 0, "Voltage Monitor");
        drawMessage(0, 10, msg);
        drawMessage(0, 24, fps_msg);  // Bottom line
        ssd1306_drawPixel(0, 31, pixel_on); // Heartbeat pixel
        pixel_on = !pixel_on;
        ssd1306_update();

        sleep_ms(voltage*20); // small delay
    }

    return 0;
}
