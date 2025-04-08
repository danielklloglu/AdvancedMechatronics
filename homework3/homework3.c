#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define PIN_NUM 16

#define GPIO_WATCH_PIN 2

static char event_str[128];
static volatile int ctr = 0;

// Perform initialisation
int pico_led_init(void) {
    #if defined(PICO_DEFAULT_LED_PIN)
        // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
        // so we can use normal GPIO functionality to turn the led on and off
        gpio_init(PICO_DEFAULT_LED_PIN);
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
        return PICO_OK;
    #elif defined(CYW43_WL_GPIO_LED_PIN)
        // For Pico W devices we need to initialise the driver etc
        return cyw43_arch_init();
    #endif
    }
    
    // Turn the led on or off
    void pico_set_led(bool led_on) {
    #if defined(PICO_DEFAULT_LED_PIN)
        // Just set the GPIO on or off
        gpio_put(PICO_DEFAULT_LED_PIN, led_on);
    #elif defined(CYW43_WL_GPIO_LED_PIN)
        // Ask the wifi "driver" to set the GPIO on or off
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
    #endif
}

void gpio_event_string(char *buf, uint32_t events);

void gpio_callback(uint gpio, uint32_t events) {
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    gpio_event_string(event_str, events);
    //printf("GPIO %d %s\n", gpio, event_str);
    if (events & GPIO_IRQ_EDGE_FALL){
        if (ctr % 2 == 0){
            pico_set_led(false);
            ctr++;
        }else{
            pico_set_led(true);
            ctr++;
        }

    }
    // if (events & GPIO_IRQ_EDGE_FALL){
    //     pico_set_led(false);
    //     printf("Button Edge Fall:");
    // }
}

static const char *gpio_irq_str[] = {
    "LEVEL_LOW",  // 0x1
    "LEVEL_HIGH", // 0x2
    "EDGE_FALL",  // 0x4
    "EDGE_RISE"   // 0x8
};

void gpio_event_string(char *buf, uint32_t events) {
    for (uint i = 0; i < 4; i++) {
        uint mask = (1 << i);
        if (events & mask) {
            // Copy this event string into the user string
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0') {
                *buf++ = *event_str++;
            }
            events &= ~mask;

            // If more events add ", "
            if (events) {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}

int main() {
    stdio_init_all();
    // waits until the USB port has been opened
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");
    
    int rc = pico_led_init();
    hard_assert(rc == PICO_OK);
    pico_set_led(true);
    
    printf("Hello GPIO IRQ\n");
    gpio_init(GPIO_WATCH_PIN);
    gpio_set_irq_enabled_with_callback(GPIO_WATCH_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_init(PIN_NUM); // PIN_NUM without the GP
    gpio_set_dir(16, GPIO_IN);
    gpio_get(PIN_NUM);

    adc_init(); // init the adc module
    adc_gpio_init(26); // set ADC0 pin to be adc input instead of GPIO
    adc_select_input(0); // select to read from ADC0

    while (1) {
        char message[100];
        uint16_t result = adc_read();
        // scanf("%s", message);
        // printf("message: %d\r\n",result);
        sleep_ms(50);
    }
}