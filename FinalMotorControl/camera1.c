#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdlib.h>

// Motor A
#define AENBL 19
#define APHASE 18

// Motor B
#define BENBL 17
#define BPHASE 16

void setup_motor(uint enbl, uint phase) {
    gpio_set_function(enbl, GPIO_FUNC_PWM);
    gpio_init(phase);
    gpio_set_dir(phase, GPIO_OUT);

    uint slice = pwm_gpio_to_slice_num(enbl);
    pwm_set_wrap(slice, 100);
    pwm_set_enabled(slice, true);
}

void set_motor(int duty_percent, uint enbl, uint phase) {
    if (duty_percent > 100) duty_percent = 100;
    if (duty_percent < -100) duty_percent = -100;

    gpio_put(phase, duty_percent >= 0);

    uint slice = pwm_gpio_to_slice_num(enbl);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(enbl), abs(duty_percent));
}

int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) sleep_ms(100);
    printf("Motor test starting...\n");

    setup_motor(AENBL, APHASE);
    setup_motor(BENBL, BPHASE);

    set_motor(80, AENBL, APHASE);  // Run Motor A at 80%
    set_motor(80, BENBL, BPHASE);  // Run Motor B at 80%

    while (true) {
        // Keep motors running
        sleep_ms(100);
    }
}

// // HW17 - Full Motor Control Based on Camera Line Tracking
// // Final Version: Controls two motors using DRV8835 via PWM + PHASE
// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/pwm.h"
// #include "cam.h"
// #include <stdlib.h>

// // Motor A (left)
// #define AENBL 19     // PWM pin for Motor A
// #define APHASE 18    // DIR pin for Motor A

// // Motor B (right)
// #define BENBL 17     // PWM pin for Motor B
// #define BPHASE 16    // DIR pin for Motor B

// // Control parameters
// const int MAX_DUTY = 100;       // Max PWM duty cycle
// const int DEADBAND = 3;         // Deadband range for center
// const float GAIN = 2.5f;        // Proportional control gain
// const int CENTER = 30;          // Center of the image (0–60)

// // Initialize motor control pins
// void setup_motor(uint enbl, uint phase) {
//     gpio_set_function(enbl, GPIO_FUNC_PWM);
//     gpio_init(phase);
//     gpio_set_dir(phase, GPIO_OUT);

//     uint slice = pwm_gpio_to_slice_num(enbl);
//     pwm_set_wrap(slice, 100);
//     pwm_set_enabled(slice, true);
// }

// // Set motor speed and direction
// void set_motor(int duty_percent, uint enbl, uint phase) {
//     if (duty_percent > MAX_DUTY) duty_percent = MAX_DUTY;
//     if (duty_percent < -MAX_DUTY) duty_percent = -MAX_DUTY;

//     gpio_put(phase, duty_percent >= 0);

//     uint slice = pwm_gpio_to_slice_num(enbl);
//     pwm_set_chan_level(slice, pwm_gpio_to_channel(enbl), abs(duty_percent));
// }

// int main() {
//     stdio_init_all();
//     while (!stdio_usb_connected()) sleep_ms(100);
//     printf("HW17: Starting camera-guided motor control...\n");

//     // Setup motors
//     setup_motor(AENBL, APHASE);
//     setup_motor(BENBL, BPHASE);

//     // Setup camera
//     init_camera_pins();

//     while (true) {
//         setSaveImage(1);

//         // Timeout-safe wait for image capture
//         int timeout = 0;
//         while (getSaveImage() == 1 && timeout++ < 100) sleep_ms(10);
//         if (timeout >= 100) {
//             printf("Camera timeout!\n");
//             continue;
//         }

//         convertImage();
//         int com = findLine(IMAGESIZEY / 2);
//         int error = com - CENTER;

//         // Deadband to prevent jitter
//         if (abs(error) < DEADBAND) error = 0;

//         // Compute adjustment
//         int adjust = (int)(GAIN * error);
//         int base = MAX_DUTY / 2;

//         int left_duty = base - adjust;
//         int right_duty = base + adjust;

//         set_motor(left_duty, AENBL, APHASE);
//         set_motor(right_duty, BENBL, BPHASE);

//         printf("COM: %d | Error: %d | L: %d | R: %d\n", com, error, left_duty, right_duty);
//         sleep_ms(100);
//     }
// }
