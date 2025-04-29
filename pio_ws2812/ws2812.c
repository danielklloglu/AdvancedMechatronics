#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "ws2812.pio.h"

#define NUM_PIXELS 4
#define SERVO_PIN 15
#define LED_PIN 2
#define IS_RGBW false
#define PIO_FREQ 800000

// Globals for servo
uint slice_num;
uint16_t wrap;

// HSV to RGB conversion structure
typedef struct {
    uint8_t r, g, b;
} wsColor;

wsColor HSBtoRGB(float hue, float sat, float brightness) {
    float r = 0, g = 0, b = 0;
    if (sat == 0.0f) {
        r = g = b = brightness;
    } else {
        if (hue >= 360.0f) hue -= 360.0f;
        int slice = hue / 60.0f;
        float frac = (hue / 60.0f) - slice;
        float p = brightness * (1.0f - sat);
        float q = brightness * (1.0f - sat * frac);
        float t = brightness * (1.0f - sat * (1.0f - frac));
        switch (slice) {
            case 0: r = brightness; g = t; b = p; break;
            case 1: r = q; g = brightness; b = p; break;
            case 2: r = p; g = brightness; b = t; break;
            case 3: r = p; g = q; b = brightness; break;
            case 4: r = t; g = p; b = brightness; break;
            case 5: r = brightness; g = p; b = q; break;
        }
    }
    return (wsColor){.r = r * 255, .g = g * 255, .b = b * 255};
}

uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(g) << 16) | ((uint32_t)(r) << 8) | b;
}

void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

void servo_pwm_setup() {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    pwm_set_clkdiv(slice_num, 120.f);
    pwm_set_wrap(slice_num, 24999);
    pwm_set_enabled(slice_num, true);
    wrap = 24999;
}

void setServoAngle(float angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    float min_pulse = 0.5, max_pulse = 2.5, period = 20.0;
    float pulse = min_pulse + (angle / 180.0f) * (max_pulse - min_pulse);
    uint16_t level = (uint16_t)((pulse / period) * wrap);
    pwm_set_gpio_level(SERVO_PIN, level);
}

int main() {
    stdio_init_all();
    sleep_ms(500);

    // Setup
    servo_pwm_setup();
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_PIN, PIO_FREQ, IS_RGBW);

    absolute_time_t start = get_absolute_time();

    while (1) {
        float t = absolute_time_diff_us(start, get_absolute_time()) / 1e6f;
        float progress = fmodf(t, 5.0f) / 5.0f;
        float angle = (progress <= 0.5f) ? progress * 2 * 180.0f : (1.0f - (progress - 0.5f) * 2) * 180.0f;
        setServoAngle(angle);

        float base_hue = progress * 360.0f;
        for (int i = 0; i < NUM_PIXELS; i++) {
            float hue = fmodf(base_hue + (i * (360.0f / NUM_PIXELS)), 360.0f);
            wsColor c = HSBtoRGB(hue, 1.0f, 0.3f);
            put_pixel(pio, sm, urgb_u32(c.r, c.g, c.b));
        }
        sleep_ms(20);
    }
}
