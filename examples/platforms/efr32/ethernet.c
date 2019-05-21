/**
 * @file
 *      This file implements the platform specific abstraction for ethernet
 * 
 */
#include "ksz8851snl.h"
#include "ksz8851snl_spi.h"


void otSysEthernetInit(void){
    KSZ8851SNL_Init();
}

void otSysEthernetDirectWrite(int numBytes, const uint8_t * data){
    KSZ8851SNL_SPI_Transmit(numBytes, data);
}
