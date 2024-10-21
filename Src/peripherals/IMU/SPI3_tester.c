#include <I2C/i2c.h>
#include <SPI/spi.h>
#include <print_scan.h>
#include <globals.h>

void testFunction_SPI3(){
//    spi_config(SPI3);


        uint8_t instruction = (uint8_t) 0x2C | 0x80;
        uint8_t datal, datah;

    while(1){
        spi_startCommunication(SPI3_CS);
        spi_transmitReceive(SPI3, &instruction, NULL, 1, false);
//        nop(10);
        spi_transmitReceive(SPI3, NULL, &datal, 1, false);
        spi_transmitReceive(SPI3, NULL, &datah, 1, false);
        spi_stopCommunication(SPI3_CS);
        nop(10);
        printMsg("bruh\r\n");

    }
}
