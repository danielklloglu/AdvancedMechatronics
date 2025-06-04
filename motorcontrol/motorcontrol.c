#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include <stdlib.h>

// Motor A
#define AENBL 18     // PWM pin for Motor A speed
#define APHASE 19    // DIR pin for Motor A

// Motor B
#define BENBL 16     // PWM pin for Motor B speed
#define BPHASE 17    // DIR pin for Motor B

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

    while (true) {
        int c = getchar_timeout_us(0);
        
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == 'w') {
                duty_a = 100;
                duty_b = 100;
            } else if (c == 's') {
                duty_a = 0;
                duty_b = 0;
            } else if (c == 'd') {
                duty_a = -100;
                duty_b = 100;
            } else if (c == 'a') {
                duty_a = 100;
                duty_b = -100;
            } else if (c == 'x') {
                duty_a = -100;
                duty_b = -100;
            } else if (c == 'q') {
                printf("Square path movement starting...\n");
                // Drive in a square path: 3s straight, turn, repeat 4 times
                for (int i = 0; i < 4; i++) {
                    // Move straight
                    set_motor(80, AENBL, APHASE);  // Forward
                    set_motor(80, BENBL, BPHASE);  // Forward
                    sleep_ms(3000);

                    // Turn: one motor forward, one motor reverse
                    set_motor(100, AENBL, APHASE);   // Reverse
                    set_motor(-100, BENBL, BPHASE);  // Forward
                    sleep_ms(333);  // Adjust for 90° turn
                }

                    // Stop both motors
                    set_motor(0, AENBL, APHASE);
                    set_motor(0, BENBL, BPHASE);
            }
            set_motor(duty_a, AENBL, APHASE);
            set_motor(duty_b, BENBL, BPHASE);
            printf("Motor A: %d%% | Motor B: %d%%\n", duty_a, duty_b);
        }

        sleep_ms(50);
    }
}