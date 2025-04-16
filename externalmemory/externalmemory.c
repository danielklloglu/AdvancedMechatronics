// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/spi.h"
// #include "pico/binary_info.h"
// #include <math.h>

// // SPI Defines
// // We are going to use SPI 0, and allocate it to the following GPIO pins
// // Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
// #define SPI_PORT spi0
// #define spi_default PICO_DEFAULT_SPI_INSTANCE()
// #define PIN_MISO 16
// #define PIN_CS   17
// #define PIN_SCK  18
// #define PIN_MOSI 19
// #define PIN_CS   13

// union FloatInt {
//     float f;
//     uint32_t i;
// };

// void spi_ram_init();

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

// void writeDAC(int channel, float voltage);

// int main(){
//     stdio_init_all();
//     spi_dac_init();
//     spi_ram_init();

//     for (int i=0;i<1000;i++){
//         float v = 1.65 * sin(2*t*2.0*3.14) + 1.65; // 2 Hz 1.65 V amplitude sine wave centered around 1.65V
//         t = t + 0.002; // resolution of the waves
//     }

//     float t=0;

//     while (1) {  
//         float v = spi_ram_read(something);
//         writeDAC(0,v);
//         sleep_ms(1);
//     }
// }

// void spi_ram_init(){
//     uint8_t buf[2];
//     buf[0] = 0b1;
//     buf[1] = 0b01000000; // sequential mode

//     spi_init(SPI_PORT,12000); // set the SPI 0 at 12 kHz and connect to GPIOs
//     gpio_set_function(PIN_SCK, GPIO_FUNC_SPI); 
//     gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
//     gpio_init(PIN_CS); 

//     // initialize CS to GP17, make it an output and set it high
//     gpio_set_dir(PIN_CS, GPIO_OUT); // GP17 = output for CS
//     gpio_put(PIN_CS,1); // GP17 to high
// }

// void spi_dac_init(){
//     spi_init(SPI_PORT,12000); // set the SPI 0 at 12 kHz and connect to GPIOs
//     gpio_set_function(PIN_SCK, GPIO_FUNC_SPI); 
//     gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
//     gpio_init(PIN_CS); 

//     // initialize CS to GP17, make it an output and set it high
//     gpio_set_dir(PIN_CS, GPIO_OUT); // GP17 = output for CS
//     gpio_put(PIN_CS,1); // GP17 to high
// }

// void spi_ram_write(uint16_t addr, float v){
//     uint8_t buf[7];
//     buf[0] = 0b00000010; // write command
//     buf[1] = addr >> 8; // address high 8 bits
//     buf[2] = addr&0xFF;  // address low 8 bits

//     union FloatInt num;
//     num.f = v;

//     buf[3] = num.i>>24;  // float high 8 bits
//     buf[4] = num.i>>16;  // float mid high 8 bits
//     buf[5] = num.i>>8;  // float mid low bits
//     buf[6] = num.i;  // float low 8 bits
//     spi_write_blocking(spi_default, buf, 7)
// }

// float spi_ram_read(uint16_t address){
//     uint8_t write[7], read[7];
//     write[0] = ; //_instruction
//     write[1] = ;// addr high byte
//     write[2] = ;// addr low byte
//     spi_write_read_blocking(spi_default, write, read, 7);

//     read[0] = ; // garbage
//     read[1] = ; // garbage
//     read[2] = ; // garbage
//     read[3] = ; // garbage
//     read[4] = ; 
//     read[5] = ; 
//     read[6] = ; 

//     union FloatInt num;
//     num.i = num.i | read[3]<< 24;
    
//     return num.f;
// }

// void writeDAC(int channel, float voltage){
//     uint8_t data[2]; // initialize data variable
//     int len = 2; // initialize the length of the data
    
//     // voltage adjustment if over the limits
//     if (voltage < 0){
//         voltage = 0;
//     } else if (voltage > 3.3){
//         voltage = 3.3;
//     }

//     uint16_t binary_voltage = (uint16_t)((voltage / 3.3) * 1023.0);
   
//     // initialize the data array
//     data[0] = 0;
//     data[1] = 0;

//     data[0] = data[0] | (channel << 7); 
//     data[0] = data[0] | (0b111<<4); // 0b01110000
//     data[0] = data[0] | (binary_voltage >> 6);
//     data[1] = data[1] | (binary_voltage << 2);

//     // data[0] = 0b10101010; // for testing
//     // data[1] = 255;

//     cs_select(PIN_CS);
//     spi_write_blocking(SPI_PORT, data, len); // where data is a uint8_t array with length len
//     cs_deselect(PIN_CS);
// }