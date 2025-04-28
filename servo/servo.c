#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define LEDPin 25 // Built-in LED pin

void setup_pwm() {
    gpio_set_function(LEDPin, GPIO_FUNC_PWM); // Set the pin function
    uint slice_num = pwm_gpio_to_slice_num(LEDPin); // Find which slice the pin uses

    float div = 3.0f; // Clock divider
    pwm_set_clkdiv(slice_num, div); // Set the divider
    
    uint16_t wrap = 50000; // Set the wrap value
    pwm_set_wrap(slice_num, wrap);

    pwm_set_gpio_level(LEDPin, wrap / 2); // 50% duty cycle
    pwm_set_enabled(slice_num, true); // Enable PWM output
}

int main()
{
    stdio_init_all();
    setup_pwm();

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
