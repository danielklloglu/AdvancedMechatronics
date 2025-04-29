#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO_PIN 15 // Change to the GPIO you use

void servo_pwm_init(uint gpio_pin) {
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio_pin);
    
    pwm_set_clkdiv(slice_num, 120.0f); // Divider
    pwm_set_wrap(slice_num, 24999);    // Wrap to make 50Hz
    pwm_set_enabled(slice_num, true);
}

void servo_set_angle(uint gpio_pin, float angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    uint slice_num = pwm_gpio_to_slice_num(gpio_pin);

    // Map angle (0-180) to pulse width (0.5ms to 2.5ms)
    float min_pulse = 0.5; // ms
    float max_pulse = 2.5; // ms
    float period_ms = 20.0; // 1/50Hz = 20ms

    float pulse_width_ms = min_pulse + (angle / 180.0f) * (max_pulse - min_pulse);
    float duty_cycle = pulse_width_ms / period_ms; // Percentage (0-1)

    uint16_t level = (uint16_t)(duty_cycle * 24999); // scale to wrap value

    pwm_set_gpio_level(gpio_pin, level);
}

int main() {
    stdio_init_all();
    
    servo_pwm_init(SERVO_PIN);

    while (true) {
        // Sweep from 0 to 180
        for (int angle = 0; angle <= 180; angle += 1) {
            servo_set_angle(SERVO_PIN, angle);
            sleep_ms(11); // ~2 seconds to go 0 -> 180
        }

        // Sweep from 180 to 0
        for (int angle = 180; angle >= 0; angle -= 1) {
            servo_set_angle(SERVO_PIN, angle);
            sleep_ms(11); // ~2 seconds to go 180 -> 0
        }
    }
}

