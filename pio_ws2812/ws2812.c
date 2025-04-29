#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

// ==== Configuration ==== //
#define WS2812_PIN 2        // <-- GPIO you wired to the DIN of first LED
#define NUM_PIXELS 4        // <-- How many LEDs you have chained
#define IS_RGBW false       // <-- If your LEDs have separate White channel (usually false)

// ==== Helper Functions ==== //

// Modified for Sparkfun WS2812B
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
        ((uint32_t)(g) << 8) |   // Green is shifted 8 bits
        ((uint32_t)(r) << 16) |  // Red is shifted 16 bits
        (uint32_t)(b);           // Blue stays here
}

// Sends a color to the LEDs
static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

// ==== Main Program ==== //

int main() {
    stdio_init_all();

    // Initialize PIO for WS2812
    PIO pio;
    uint sm;
    uint offset;

    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(
        &ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    sleep_ms(100); // wait for LEDs to power up

    // === Turn all LEDs red === //
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(pio, sm, urgb_u32(255, 0, 0)); // Full red
    }

    sleep_ms(1); // reset time

    while (true) {
        tight_loop_contents(); // Do nothing
    }
}
