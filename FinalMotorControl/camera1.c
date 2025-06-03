#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "cam.h"  // assumes you have cam.h/cam.c properly configured

#define LED_LEFT  21
#define LED_RIGHT 20

const int MAX_BRIGHTNESS = 1000; // PWM resolution
const int CENTER = 30;           // Middle of 0–60 range
const float GAIN = 16.0f;        // How aggressively we respond

void setup_pwm(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice, MAX_BRIGHTNESS);
    pwm_set_enabled(slice, true);
}

void set_brightness(uint gpio, int level) {
    if (level < 0) level = 0;
    if (level > MAX_BRIGHTNESS) level = MAX_BRIGHTNESS;
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(gpio), level);
}

int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) sleep_ms(100);

    printf("Camera-controlled LED PWM starting...\n");

    // Setup PWM on both LEDs
    setup_pwm(LED_LEFT);
    setup_pwm(LED_RIGHT);

    // Initialize camera
    init_camera_pins();

    while (true) {
        setSaveImage(1);
        while (getSaveImage() == 1) {}
        convertImage();

        int com = findLine(IMAGESIZEY / 2); // Get center of white line
        int error = com - CENTER;           // How far off we are
        int adjust = (int)(GAIN * error);   // Proportional control

        int left = MAX_BRIGHTNESS / 2 - adjust;
        int right = MAX_BRIGHTNESS / 2 + adjust;

        set_brightness(LED_LEFT, left);
        set_brightness(LED_RIGHT, right);

        printf("COM: %d | Error: %d | Left: %d | Right: %d\n", com, error, left, right);
        sleep_ms(100);
    }
}


// // HW17 - Differential Drive LED Simulation Controller
// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/pwm.h"
// #include "cam.h"
// #include <stdlib.h>

// // LED Pins representing left and right motor PWM
// #define LED_LEFT  16
// #define LED_RIGHT 17

// // Control parameters
// const int MAX_DUTY = 100;       // Max PWM duty cycle
// const int DEADBAND = 3;         // No correction if error within this range
// const float SLOPE = 2.5f;       // Proportional control gain

// void setup_pwm(uint gpio) {
//     gpio_set_function(gpio, GPIO_FUNC_PWM);
//     uint slice = pwm_gpio_to_slice_num(gpio);
//     pwm_set_wrap(slice, 100);
//     pwm_set_enabled(slice, true);
// }

// void set_brightness(uint gpio, int level) {
//     if (level > MAX_DUTY) level = MAX_DUTY;
//     if (level < 0) level = 0;
//     uint slice = pwm_gpio_to_slice_num(gpio);
//     pwm_set_chan_level(slice, pwm_gpio_to_channel(gpio), level);
// }

// int main() {
//     stdio_init_all();
//     while (!stdio_usb_connected()) {
//         sleep_ms(100);
//     }
//     printf("HW17: LED PWM Differential Controller Starting...\n");

//     // Initialize camera and LEDs
//     init_camera_pins();
//     setup_pwm(LED_LEFT);
//     setup_pwm(LED_RIGHT);

//     while (true) {
//         setSaveImage(1);
//         while (getSaveImage() == 1) {}
//         convertImage();

//         int com = findLine(IMAGESIZEY / 2);  // Line center from camera
//         int error = com - 30;                // Center of 60-pixel range

//         // Deadband logic
//         if (abs(error) < DEADBAND) {
//             error = 0;
//         }

//         // Control signal scaled by slope
//         int adjust = (int)(SLOPE * error);

//         // Base speed
//         int base = MAX_DUTY / 2;

//         // Adjust left/right speeds based on direction
//         int left_duty = base - adjust;
//         int right_duty = base + adjust;

//         // Clamp and apply to LEDs
//         set_brightness(LED_LEFT, left_duty);
//         set_brightness(LED_RIGHT, right_duty);

//         printf("COM: %d | Error: %d | L: %d | R: %d\n", com, error, left_duty, right_duty);
//         sleep_ms(100);
//     }
// }
