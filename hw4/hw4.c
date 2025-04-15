#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include <math.h>

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}

void writeDAC(int channel, float voltage);

int main(){
    stdio_init_all();

    spi_init(SPI_PORT,12000); // set the SPI 0 at 12 kHz and connect to GPIOs
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI); 
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_init(PIN_CS); 

    // initialize CS to GP17, make it an output and set it high
    gpio_set_dir(PIN_CS, GPIO_OUT); // GP17 = output for CS
    gpio_put(PIN_CS,1); // GP17 to high

    float t=0;
    while (1) {  
        for (int i=0;i<200;i++){
            float v = 1.65 * sin(2*t*2.0*3.14) + 1.65; // 2 Hz 1.65 V amplitude sine wave centered around 1.65V
            // float v = fabs(3.3*(fmod(2*2*t,2)-1)); // 1 Hz triangle wave
            t = t + 0.002; // resolution of the waves
            writeDAC(0,v);
            sleep_ms(2);
        }
        printf("Write!\r\n");
    }
}

void writeDAC(int channel, float voltage){
    uint8_t data[2]; // initialize data variable
    int len = 2; // initialize the length of the data
    
    // voltage adjustment if over the limits
    if (voltage < 0){
        voltage = 0;
    } else if (voltage > 3.3){
        voltage = 3.3;
    }

    uint16_t binary_voltage = (uint16_t)((voltage / 3.3) * 1023.0);
   
    // initialize the data array
    data[0] = 0;
    data[1] = 0;

    data[0] = data[0] | (channel << 7); 
    data[0] = data[0] | (0b111<<4); // 0b01110000
    data[0] = data[0] | (binary_voltage >> 6);
    data[1] = data[1] | (binary_voltage << 2);

    // data[0] = 0b10101010; // for testing
    // data[1] = 255;

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, len); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS);
}