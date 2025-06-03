// HW17 - Differential Drive Motor Control with Camera Input
// Based on tested motor control code using PWM + DIR with DRV8835 H-Bridge
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "cam.h"


// Motor A (tested configuration)
#define AENBL 18     // PWM pin for Motor A
#define APHASE 19    // DIR pin for Motor A

// Motor B (tested configuration)
#define BENBL 16     // PWM pin for Motor B
#define BPHASE 17    // DIR pin for Motor B

// Control parameters
const int MAX_DUTY = 100;       // Max PWM duty cycle
const int DEADBAND = 3;         // No correction if error within this range
const float SLOPE = 2.5f;       // Proportional control gain

// Initialize PWM and DIR pin for motor
void setup_motor(uint enbl, uint phase) {
    gpio_set_function(enbl, GPIO_FUNC_PWM);
    gpio_init(phase);
    gpio_set_dir(phase, GPIO_OUT);

    uint slice = pwm_gpio_to_slice_num(enbl);
    pwm_set_wrap(slice, 100);
    pwm_set_enabled(slice, true);
}

// Set motor duty and direction
void set_motor(int duty_percent, uint enbl, uint phase) {
    if (duty_percent > MAX_DUTY) duty_percent = MAX_DUTY;
    if (duty_percent < -MAX_DUTY) duty_percent = -MAX_DUTY;

    gpio_put(phase, duty_percent >= 0);

    uint slice = pwm_gpio_to_slice_num(enbl);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(enbl), abs(duty_percent));
}

int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("HW17: Motor PWM Differential Controller Starting...\n");

    // Initialize camera and motor control pins
    init_camera_pins();
    setup_motor(AENBL, APHASE);
    setup_motor(BENBL, BPHASE);

    while (true) {
        setSaveImage(1);
        while (getSaveImage() == 1) {}
        convertImage();

        int com = findLine(IMAGESIZEY / 2);  // Line center from camera
        int error = com - 30;                // Center of 60-pixel range

        // Deadband logic
        if (abs(error) < DEADBAND) {
            error = 0;
        }

        // Control signal scaled by slope
        int adjust = (int)(SLOPE * error);

        // Base speed
        int base = MAX_DUTY / 2;

        // Adjust left/right speeds based on direction
        int left_duty = base - adjust;
        int right_duty = base + adjust;

        // Apply motor speeds using verified motor control logic
        set_motor(left_duty, AENBL, APHASE);
        set_motor(right_duty, BENBL, BPHASE);

        printf("COM: %d | Error: %d | Left: %d | Right: %d\n", com, error, left_duty, right_duty);
        sleep_ms(100);
    }
}