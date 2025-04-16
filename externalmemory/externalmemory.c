#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include <math.h>

// SPI Defines
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_CS_RAM 13
#define PIN_CS_DAC 17

union FloatInt {
    float f;
    uint32_t i;
};

void spi_ram_init();
void spi_dac_init();
void spi_ram_write(uint16_t addr, float v);
float spi_ram_read(uint16_t addr);
void writeDAC(int channel, float voltage);
void spi_ram_init();

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
    spi_dac_init();
    spi_ram_init();

    float t = 0.0;

    for (int i=0;i<1000;i++){
        float v = 1.65f * sinf(2.0f * M_PI * t) + 1.65f; // 0–3.3V sine wave
        spi_ram_write(i * 4, v); // 4 bytes per float
        t = t + 0.001; // resolution of the waves
    }

    uint16_t addr = 0;

    while (1) {
        float v = spi_ram_read(addr);
        writeDAC(0, v);
        sleep_ms(1);
        addr += 4;
        if (addr >= 4000) addr = 0;
    }
}


void spi_ram_init() {
    spi_init(SPI_PORT, 1000 * 1000); // 1 MHz
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);

    gpio_init(PIN_CS_RAM);
    gpio_set_dir(PIN_CS_RAM, GPIO_OUT);
    gpio_put(PIN_CS_RAM, 1);

    uint8_t buf[2] = {0x01, 0x40}; // write mode register: sequential mode
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, buf, 2);
    cs_deselect(PIN_CS_RAM);
}

void spi_dac_init() {
    spi_init(SPI_PORT, 1000 * 1000); // 1 MHz
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS_DAC);
    gpio_set_dir(PIN_CS_DAC, GPIO_OUT);
    gpio_put(PIN_CS_DAC, 1);
}

void spi_ram_write(uint16_t addr, float v){
    uint8_t buf[7];
    buf[0] = 0x02; // write command
    buf[1] = addr >> 8; // address high 8 bits
    buf[2] = addr & 0xFF;  // address low 8 bits

    union FloatInt num;
    num.f = v;

    buf[3] = (num.i>>24);  // float high 8 bits
    buf[4] = (num.i>>16);  // float mid high 8 bits
    buf[5] = (num.i>>8);  // float mid low bits
    buf[6] = (num.i);  // float low 8 bits

    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, buf, 7);
    cs_deselect(PIN_CS_RAM);

}

float spi_ram_read(uint16_t addr) {
    uint8_t tx[3] = {0x03, addr >> 8, addr & 0xFF}; // Step 1–3
    uint8_t rx[4] = {0};                            // Step 4 — will hold 4 data bytes (a float)

    cs_select(PIN_CS_RAM);                          // Step 1 — pull CS low

    spi_write_blocking(SPI_PORT, tx, 3);            // Step 2–3 — send read instruction + address
    spi_read_blocking(SPI_PORT, 0, rx, 4);          // Step 4 — read 4 bytes into rx[] (send dummy bytes 0x00)

    cs_deselect(PIN_CS_RAM);                        // Step 5 — pull CS high to end transaction

    union FloatInt num;                             // Step 6 — interpret the bytes as a float
    num.i = (rx[0] << 24) | (rx[1] << 16) | (rx[2] << 8) | rx[3];
    return num.f;
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

    cs_deselect(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, data, len); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS_DAC);
}