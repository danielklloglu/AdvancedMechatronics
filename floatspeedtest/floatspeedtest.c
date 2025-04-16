#include <stdio.h>
#include "pico/stdlib.h"
#include <math.h>

#define N 1000
#define CLOCK_FREQ_MHZ 150

int main() {
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");

    volatile float f1, f2;
    printf("Enter two floats to use: ");
    scanf("%f %f", &f1, &f2);

    volatile float result = 0.0;
    absolute_time_t start, end;
    uint64_t elapsed_us;
    float cycles;

    // Addition
    start = get_absolute_time();
    for (int i = 0; i < N; i++) {
        result = f1 + f2;
    }
    end = get_absolute_time();
    elapsed_us = to_us_since_boot(end) - to_us_since_boot(start);
    cycles = (elapsed_us * CLOCK_FREQ_MHZ) / (float)N;
    printf("\nAddition took %.2f clock cycles\n", cycles);

    // Subtraction
    start = get_absolute_time();
    for (int i = 0; i < N; i++) {
        result = f1 - f2;
    }
    end = get_absolute_time();
    elapsed_us = to_us_since_boot(end) - to_us_since_boot(start);
    cycles = (elapsed_us * CLOCK_FREQ_MHZ) / (float)N;
    printf("Subtraction took %.2f clock cycles\n", cycles);

    // Multiplication
    start = get_absolute_time();
    for (int i = 0; i < N; i++) {
        result = f1 * f2;
    }
    end = get_absolute_time();
    elapsed_us = to_us_since_boot(end) - to_us_since_boot(start);
    cycles = (elapsed_us * CLOCK_FREQ_MHZ) / (float)N;
    printf("Multiplication took %.2f clock cycles\n", cycles);

    // Division
    start = get_absolute_time();
    for (int i = 0; i < N; i++) {
        result = f1 / f2;
    }
    end = get_absolute_time();
    elapsed_us = to_us_since_boot(end) - to_us_since_boot(start);
    cycles = (elapsed_us * CLOCK_FREQ_MHZ) / (float)N;
    printf("Division took %.2f clock cycles\n", cycles);

    return 0;
}

