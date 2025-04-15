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

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 12000); // set to a 1000 to see easier
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi
    
    float t=0;

    while (1) {  
        // for (int i=0;i<200;i++){
        //     float v = 1.65*sin(2*t*2.0*3.14) + 1.65;
        //     // float v = fabs(3.3*(fmod(2*2*t,2)-1));
        //     t=t+0.002;
        //     writeDAC(0,v);
        //     sleep_ms(2);
        // }
        // printf("Data written!\r\n");
        writeDAC(0,0);
    }
}

void writeDAC(int channel, float voltage){
    uint8_t data[2];
    int len = 2;
    // if (voltage < 0){
    //     voltage = 0;
    // }

    // if (voltage > 3.3){
    //     voltage = 3.3;
    // }

    // uint16_t v_bin = (uint16_t)((voltage / 3.3) * 1023.0);
    
    // data[0] = 0;
    // data[1] = 0;
    // data[0] = data[0] | (channel<<7);
    // data[0] = data[0] | (0b111<<4);
    // data[0] = data[0] | (v_bin>>6);
    // data[1] = data[1] | (v_bin << 2);
    data[0] = 0b10101010;
    data[1] = 255;
    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, len); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS);
}


// #include <stdio.h>
// #include <math.h>
// #include "pico/stdlib.h"
// #include "pico/binary_info.h"
// #include "hardware/spi.h"

// #define SPI_PORT spi0
// #define SCK_PIN_NUM 18
// #define SDI_PIN_NUM 19
// #define CS_PIN_NUM 17
// #define PI 3.142
// #define FREQ 2

// void spi_initialize(void);
// static inline void cs_select(uint cs_pin);
// static inline void cs_deselect(uint cs_pin);
// void writeDAC(int channel, float volts);

// int main()
// {
//     stdio_init_all();
//     spi_initialize();
//     float t=0;

//     while (1){
//         for (int i=0;i<200;i++){
//             float v = 1.65*sin(FREQ*t*2.0*PI) + 1.65;
//             // float v = fabs(3.3*(fmod(2*FREQ*t,2)-1));
//             t=t+0.002;
//             writeDAC(0,v);
//             sleep_ms(2);
//         }
//         printf("Data written!\r\n");
//         //sleep_ms(0.01);
//     }
// }

// void spi_initialize(void){
//     spi_init(SPI_PORT,12000); // set the SPI 0 at 12 kHz and connect to GPIOs
//     gpio_set_function(SCK_PIN_NUM, GPIO_FUNC_SPI); // set the SCK to GP6 (pin 9)
//     gpio_set_function(SDI_PIN_NUM, GPIO_FUNC_SPI); // set the SDI to GP7 (pin 10) 

//     gpio_init(CS_PIN_NUM); // set CS to GP9 (pin 12)
//     gpio_set_dir(CS_PIN_NUM, GPIO_OUT); // set GP9 to be output pin for chip select
//     gpio_put(CS_PIN_NUM,1); // initialize GP9 to high
// }

// static inline void cs_select(uint cs_pin) {
//     asm volatile("nop \n nop \n nop"); // FIXME
//     gpio_put(cs_pin, 0);
//     asm volatile("nop \n nop \n nop"); // FIXME
// }

// static inline void cs_deselect(uint cs_pin) {
//     asm volatile("nop \n nop \n nop"); // FIXME
//     gpio_put(cs_pin, 1);
//     asm volatile("nop \n nop \n nop"); // FIXME
// }

// void writeDAC(int channel, float volts){
//     uint8_t data[2];
//     int len = 2;

//     if (volts < 0){
//         volts = 0;
//     }

//     if (volts > 3.3){
//         volts = 3.3;
//     }

//     uint16_t v_bin = (uint16_t)((volts / 3.3) * 1023.0);
    
//     data[0] = 0;
//     data[1] = 0;
//     data[0] = data[0] | (channel<<7);
//     data[0] = data[0] | (0b111<<4);
//     data[0] = data[0] | (v_bin>>6);
//     data[1] = data[1] | (v_bin << 2);

//     //data[0] = 0b01111000;
//     //data[1] = 0b00000000;

//     cs_select(CS_PIN_NUM);
//     spi_write_blocking(SPI_PORT, data, len); // where data is a uint8_t array with length len
//     cs_deselect(CS_PIN_NUM);
// }