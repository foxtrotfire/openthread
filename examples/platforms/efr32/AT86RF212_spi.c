#include <stdio.h>
#include <stdint.h>

#include "AT86RF212_spi.h"
#include "em_assert.h"
#include "em_common.h"
#include "em_gpio.h"
#include "spidrv.h"

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_usart.h"
#include "em_ldma.h"
#include "em_prs.h"

#include "hal-config.h"

#define BOGUS_BYTE           0xFF               /**< Bogus byte used for receiving via SPI */


#define AT86RF212SNL_RST_PIN	7U
#define AT86RF212SNL_RST_PORT 	gpioPortF

#define AT86RF212SNL_CS_PIN		7U
#define AT86RF212SNL_CS_PORT 	gpioPortC

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
static SPIDRV_HandleData_t spiHandleData;
static SPIDRV_Handle_t spiHandle = &spiHandleData;

/** @endcond */

void AT86RF212_SPI_Init(void) {

	Ecode_t result;
	SPIDRV_Init_t initData = SPIDRV_MASTER_USART0;

	initData.csControl = spidrvCsControlAuto;
	initData.portLocationRx = PORTIO_USART0_RX_LOC;
	initData.portLocationTx = PORTIO_USART0_TX_LOC;
	initData.portLocationClk = PORTIO_USART0_CLK_LOC;
	initData.portLocationCs = PORTIO_USART0_CS_LOC;
	initData.bitRate = 1000000; // 8 MHz max safe speed
	initData.frameLength = 8;
	initData.dummyTxValue = BOGUS_BYTE;
	initData.bitOrder = spidrvBitOrderMsbFirst;
	initData.clockMode = spidrvClockMode0;


	result = SPIDRV_Init(spiHandle, &initData);
	EFM_ASSERT(result == ECODE_EMDRV_SPIDRV_OK);
}

void AT86RF212_SPI_Transmit(int numBytes, const uint8_t * data) {
	Ecode_t result;
	int remaining = numBytes;

	/* The spidrv is using dma to transfer bytes to the USART (spi)
	 * DMADRV_MAX_XFER_COUNT (1024 on EFM32GG990F1024)
	 * this is why we must transmit bytes in chunks of at most
	 */
	do {
		int count = SL_MIN(remaining, DMADRV_MAX_XFER_COUNT);
		result = SPIDRV_MTransmitB(spiHandle, data, count);
		remaining -= count;
		data += count;
	} while (result == ECODE_EMDRV_SPIDRV_OK && remaining > 0);

	EFM_ASSERT(result == ECODE_EMDRV_SPIDRV_OK);

}

void AT86RF212_SPI_Receive(int numBytes, uint8_t * buffer) {
	Ecode_t result;
	int remaining = numBytes;

	/* The spidrv is using dma to receive bytes from the USART (spi)
	 * this is why we must receive bytes in chunks of at most
	 * DMADRV_MAX_XFER_COUNT (1024 on EFM32GG990F1024)
	 */
	do {
		int count = SL_MIN(remaining, DMADRV_MAX_XFER_COUNT);
		result = SPIDRV_MReceiveB(spiHandle, buffer, count);
		remaining -= count;
		buffer += count;
	} while (result == ECODE_EMDRV_SPIDRV_OK && remaining > 0);

	EFM_ASSERT(result == ECODE_EMDRV_SPIDRV_OK);
}

uint16_t AT86RF212_SPI_ReadRegister(uint8_t reg) {

	uint16_t value = 0x0000;
	uint8_t txBuffer[4];
	uint8_t rxBuffer[4];

	txBuffer[0] = reg;

	SPIDRV_MTransferB(spiHandle, txBuffer, rxBuffer, 4);

	value = (rxBuffer[3] << 8) | rxBuffer[2];
	return value;

}

void AT86RF212_SPI_WriteRegister(uint8_t reg, uint16_t value) {
	uint8_t txBuffer[4];

	txBuffer[0] = reg;
	txBuffer[1] = value & 0xff;
	txBuffer[2] = (value >> 8) & 0xff;

	AT86RF212_SPI_Transmit(4, txBuffer);
}

void AT86RF212_SPI_WriteByte(uint8_t value) {
	uint8_t txBuffer;
	AT86RF212_SPI_Transmit(1, &txBuffer);
}
