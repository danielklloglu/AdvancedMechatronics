#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define LED_PIN 15
#define ADC_PIN 26 // GPIO26 = ADC0

// Command flags from Core 0 to Core 1
#define CMD_READ_ADC 0x01
#define CMD_LED_ON   0x02
#define CMD_LED_OFF  0x03

// Response flag from Core 1 to Core 0
#define FLAG_ADC_DONE 0x10

volatile float shared_adc_result = 0.0f;

void core1_entry() {
    // Initialize hardware on Core 1
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    adc_init();
    adc_gpio_init(ADC_PIN); // ADC0 is on GPIO26
    adc_select_input(0); // Select ADC0

    while (1) {
        uint32_t command = multicore_fifo_pop_blocking();

        if (command == CMD_READ_ADC) {
            uint16_t raw = adc_read();
            shared_adc_result = (3.3f * raw) / 4095.0f;
            multicore_fifo_push_blocking(FLAG_ADC_DONE);
        }
        else if (command == CMD_LED_ON) {
            gpio_put(LED_PIN, 1);
        }
        else if (command == CMD_LED_OFF) {
            gpio_put(LED_PIN, 0);
        }
    }
}

int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("Multicore control started. Commands:\n");
    printf("0 = Read A0 voltage\n1 = Turn ON LED\n2 = Turn OFF LED\n");

    multicore_launch_core1(core1_entry);

    while (1) {
        char c;
        scanf(" %c", &c); // Read user input (skip whitespace)

        if (c == '0') {
            multicore_fifo_push_blocking(CMD_READ_ADC);
            multicore_fifo_pop_blocking(); // Wait for ADC result
            printf("Voltage on A0: %.2f V\n", shared_adc_result);
        }
        else if (c == '1') {
            multicore_fifo_push_blocking(CMD_LED_ON);
            printf("LED turned ON\n");
        }
        else if (c == '2') {
            multicore_fifo_push_blocking(CMD_LED_OFF);
            printf("LED turned OFF\n");
        }
        else {
            printf("Unknown command: %c\n", c);
        }
    }
}
