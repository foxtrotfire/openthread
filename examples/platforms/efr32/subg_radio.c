/**
 * @file
 *      This file implements the platform specific abstraction for ethernet
 *
 */
#include "AT86RF212.h"
#include "AT86RF212_spi.h"

void otSysSubgRadioInit(void)
{
    AT86RF212_Init();
}

void otSysSubgRadioDirectWrite(int numBytes, const uint8_t *data)
{
    AT86RF212_SPI_Transmit(numBytes, data);
}

uint8_t otSysSubgRadioDirectReadRegister(uint8_t reg)
{
    return AT86RF212_SPI_ReadRegister(reg);
}
void otSysSubgRadioFrameWrite(int numBytes, uint8_t *data)
{
    AT86RF212_SPI_WriteFrame(numBytes, data);
}

uint8_t *otSysSubgRadioDirectReadFrame()
{
    return AT86RF212_SPI_ReadFrame();
}

void otSysSubgRadioTest(void){

    transmissionTest();

}