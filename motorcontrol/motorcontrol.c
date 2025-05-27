#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include <stdlib.h>

// Motor A
#define AENBL 0     // PWM pin for Motor A speed
#define APHASE 1    // DIR pin for Motor A

// Motor B
#define BENBL 2     // PWM pin for Motor B speed
#define BPHASE 3    // DIR pin for Motor B

int duty_a = 0; // Range: -100 to +100
int duty_b = 0; // Range: -100 to +100

void set_motor(int duty_percent, uint enbl, uint phase) {
    // Clamp to -100 to +100
    if (duty_percent > 100) {
        duty_percent = 100;
    }
    if (duty_percent < -100) {
        duty_percent = -100;
    }

    // Set direction
    if (duty_percent >= 0) {
        gpio_put(phase, 1);  // Forward
    } else {
        gpio_put(phase, 0);  // Reverse
    }

    // Set PWM duty cycle
    gpio_set_function(enbl, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(enbl);
    pwm_set_wrap(slice, 100);  // 100 = 1% resolution
    pwm_set_chan_level(slice, pwm_gpio_to_channel(enbl), abs(duty_percent));
    pwm_set_enabled(slice, true);
}

int main() {
    stdio_init_all();

    // Set up direction pins
    gpio_init(APHASE);
    gpio_set_dir(APHASE, GPIO_OUT);

    gpio_init(BPHASE);
    gpio_set_dir(BPHASE, GPIO_OUT);

    // Wait for serial connection (optional)
    while (!stdio_usb_connected()) {
        tight_loop_contents();
    }

    printf("Dual motor ready.\n");
    printf("Use '+/-' for Motor A, '[' and ']' for Motor B.\n");

    while (true) {
        int c = getchar_timeout_us(0);
        
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '+') {
                if (duty_a < 100) {
                    duty_a += 1;
                }
            } else if (c == '-') {
                if (duty_a > -100) {
                    duty_a -= 1;
                }
            } else if (c == '[') {
                if (duty_b < 100) {
                    duty_b += 1;
                }
            } else if (c == ']') {
                if (duty_b > -100) {
                    duty_b -= 1;
                }
            }

            set_motor(duty_a, AENBL, APHASE);
            set_motor(duty_b, BENBL, BPHASE);
            printf("Motor A: %d%% | Motor B: %d%%\n", duty_a, duty_b);
        }

        sleep_ms(50);
    }
}
