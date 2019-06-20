/*
 * AT86RF212_spi.h
 *
 *  Created on: 14 mei 2019
 *      Author: Koen
 */

#ifndef EMDRV_SPIDRV_868MHZ_PHY_INC_AT86RF212_SPI_H_
#define EMDRV_SPIDRV_868MHZ_PHY_INC_AT86RF212_SPI_H_

#include <stdio.h>
#include <stdint.h>
#include "AT86RF212_def.h"

void AT86RF212_SPI_Transmit(int numBytes, const uint8_t * data);
void AT86RF212_SPI_Receive(int numBytes, uint8_t * buffer);
void AT86RF212_SPI_WriteByte(uint8_t value);
uint16_t AT86RF212_SPI_ReadRegister(uint8_t reg);
void AT86RF212_SPI_WriteRegister(uint8_t reg, uint16_t value);
void AT86RF212_SPI_Init();
void AT86RF212_SPI_WriteFrame(uint8_t length, uint8_t *frame);
uint8_t * AT86RF212_SPI_ReadFrame();
void Init_AT86RF212_SLP_IRQ();
void transmissionTest();
void AT86RF212_SPI_TXframe(uint8_t length, uint8_t* data);



#endif /* EMDRV_SPIDRV_868MHZ_PHY_INC_AT86RF212_SPI_H_ */
