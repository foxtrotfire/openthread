#include <stdint.h>
#include <stdio.h>

#include "AT86RF212_def.h"
#include "AT86RF212_spi.h"
#include "em_assert.h"
#include "em_common.h"
#include "em_gpio.h"
#include "gpiointerrupt.h"
#include "spidrv.h"

#include "em_chip.h"
#include "em_cmu.h"
#include "em_device.h"
#include "em_ldma.h"
#include "em_prs.h"
#include "em_usart.h"
#include "hal-config.h"
#include "openthread-system.h"
#include <openthread/cli.h>

#define BOGUS_BYTE 0x00 /**< Bogus byte used for receiving via SPI */

#define AT86RF212SNL_RST_PIN 6U
#define AT86RF212SNL_RST_PORT gpioPortF

#define AT86RF212SNL_CS_PIN 7U
#define AT86RF212SNL_CS_PORT gpioPortC

#define AT86RF212SNL_SLP_TR_PIN 7U
#define AT86RF212SNL_SLP_TR_PORT gpioPortF

#define AT86RF212SNL_IRQ_PIN 6U
#define AT86RF212SNL_IRQ_PORT gpioPortC

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
static SPIDRV_HandleData_t spiHandleData;
static SPIDRV_Handle_t     spiHandle = &spiHandleData;

/** @endcond */

void AT86RF212_SPI_Init(void)
{
    CMU_ClockEnable(cmuClock_GPIO, true);
    GPIO_PinModeSet(AT86RF212SNL_RST_PORT, AT86RF212SNL_RST_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(AT86RF212SNL_SLP_TR_PORT, AT86RF212SNL_SLP_TR_PIN, gpioModePushPull, 0);

    GPIO_PinModeSet(AT86RF212SNL_IRQ_PORT, AT86RF212SNL_IRQ_PIN, gpioModeInputPull, 0);
    // Init_AT86RF212_SLP_IRQ();

    Ecode_t       result;
    SPIDRV_Init_t initData = SPIDRV_MASTER_USART0;

    initData.csControl       = spidrvCsControlAuto;
    initData.portLocationRx  = PORTIO_USART0_RX_LOC;
    initData.portLocationTx  = PORTIO_USART0_TX_LOC;
    initData.portLocationClk = PORTIO_USART0_CLK_LOC;
    initData.portLocationCs  = PORTIO_USART0_CS_LOC;

    initData.bitRate      = 1000000; // 8 MHz max safe speed
    initData.frameLength  = 8;
    initData.dummyTxValue = BOGUS_BYTE;
    initData.bitOrder     = spidrvBitOrderMsbFirst;
    initData.clockMode    = spidrvClockMode0;

    result = SPIDRV_Init(spiHandle, &initData);
    EFM_ASSERT(result == ECODE_EMDRV_SPIDRV_OK);

    GPIO_PinOutSet(AT86RF212SNL_RST_PORT, AT86RF212SNL_RST_PIN);
    GPIO_PinOutClear(AT86RF212SNL_SLP_TR_PORT, AT86RF212SNL_SLP_TR_PIN);

    GPIO_PinOutClear(AT86RF212SNL_RST_PORT, AT86RF212SNL_RST_PIN);
    GPIO_PinOutSet(AT86RF212SNL_RST_PORT, AT86RF212SNL_RST_PIN);

    AT86RF212_SPI_WriteRegister(TRX_STATUS_REG, TRX_CMD_TRX_OFF); // clock reset

    AT86RF212_SPI_WriteRegister(TRX_STATUS_REG, TRX_CMD_TRX_OFF);

    AT86RF212_SPI_WriteRegister(TRX_STATUS_REG, TRX_CMD_PLL_ON);
}

void AT86RF212_SPI_Transmit(int numBytes, const uint8_t *data)
{
    Ecode_t result;
    int     remaining = numBytes;

    /* The spidrv is using dma to transfer bytes to the USART (spi)
     * DMADRV_MAX_XFER_COUNT (1024 on EFM32GG990F1024)
     * this is why we must transmit bytes in chunks of at most
     */
    do
    {
        int count = SL_MIN(remaining, DMADRV_MAX_XFER_COUNT);
        result    = SPIDRV_MTransmitB(spiHandle, data, count);
        remaining -= count;
        data += count;
    } while (result == ECODE_EMDRV_SPIDRV_OK && remaining > 0);

    EFM_ASSERT(result == ECODE_EMDRV_SPIDRV_OK);
}

void AT86RF212_SPI_Receive(int numBytes, uint8_t *buffer)
{
    Ecode_t result;
    int     remaining = numBytes;

    /* The spidrv is using dma to receive bytes from the USART (spi)
     * this is why we must receive bytes in chunks of at most
     * DMADRV_MAX_XFER_COUNT (1024 on EFM32GG990F1024)
     */
    do
    {
        int count = SL_MIN(remaining, DMADRV_MAX_XFER_COUNT);
        result    = SPIDRV_MReceiveB(spiHandle, buffer, count);
        remaining -= count;
        buffer += count;
    } while (result == ECODE_EMDRV_SPIDRV_OK && remaining > 0);

    EFM_ASSERT(result == ECODE_EMDRV_SPIDRV_OK);
}

uint16_t AT86RF212_SPI_ReadRegister(uint8_t reg)
{
    uint8_t txBuffer[2];
    uint8_t rxBuffer[2];
    txBuffer[0] = (reg | AT86RF212_REG_READ_FLAG);
    txBuffer[1] = (BOGUS_BYTE);

    SPIDRV_MTransferB(spiHandle, txBuffer, rxBuffer, 2);
    return (rxBuffer[0] << 8) | rxBuffer[1];
}

void AT86RF212_SPI_WriteRegister(uint8_t reg, uint16_t value)
{
    uint8_t txBuffer[4];

    txBuffer[0] = (reg | (AT86RF212_REG_WRITE_FLAG));
    txBuffer[1] = value & 0xff;
    txBuffer[2] = (value >> 8) & 0xff;

    AT86RF212_SPI_Transmit(4, txBuffer);
}

void AT86RF212_SPI_WriteByte(uint8_t value)
{
    uint8_t txBuffer;
    AT86RF212_SPI_Transmit(1, &txBuffer);
}

void AT86RF212_SPI_WriteFrame(uint8_t length, uint8_t *frame)
{
    AT86RF212_SPI_WriteRegister(TRX_STATUS_REG, TRX_CMD_TRX_OFF);
    AT86RF212_SPI_WriteRegister(TRX_STATUS_REG, TRX_CMD_PLL_ON);

    uint8_t data_out[127];

    data_out[0] = length + 2;
    for (int i = 0; i < length; i++)
    {
        data_out[i + 1] = frame[i];
    }
    data_out[length + 1] = 0x00;
    data_out[length + 2] = 0x00;

    // Write frame to device
    // Note that data[0] must be length - AT86RF212_LEN_FIELD_LEN
    // at86rf212_write_frame(length + AT86RF212_LEN_FIELD_LEN + AT86RF212_CRC_LEN, send_data);ta_out);
    AT86RF212_SPI_TXframe(length + 2 + 1, data_out);

    AT86RF212_SPI_WriteRegister(TRX_STATUS_REG, TRX_CMD_TX_START);

    // otCliOutputFormat("STATUS: %i \r\n", AT86RF212_SPI_ReadRegister(0x01));
}

uint8_t *AT86RF212_SPI_ReadFrame()
{
    uint8_t        txBuffer[0];
    static uint8_t TrxBuffer[128];
    txBuffer[0] = (0x1 << 5);

    SPIDRV_MTransferB(spiHandle, txBuffer, TrxBuffer, 7);

    return TrxBuffer;
}

static void AT86RF212_IRQ(uint8_t pin)
{
    OT_UNUSED_VARIABLE(pin);
    int test;
    test = (AT86RF212_SPI_ReadRegister(0x1) && 0x1F);
    otCliOutputFormat("IRQ Status: %i \r\n", test);
}

void Init_AT86RF212_SLP_IRQ()
{
    /* Enable IRQ */
    GPIOINT_Init();
    /* Configure  as input */
    GPIOINT_CallbackRegister(AT86RF212SNL_SLP_TR_PIN, AT86RF212_IRQ);
    GPIO_IntEnable(2 << AT86RF212SNL_SLP_TR_PIN);

    GPIO_IntConfig(AT86RF212SNL_SLP_TR_PORT, AT86RF212SNL_SLP_TR_PIN, false, true, true);
}

void transmissionTest()
{
    uint8_t data[6] = {0x01, 0x01, 0x02, 0x03, 0x04, 0x05};
    static uint8_t test;

    AT86RF212_SPI_WriteRegister(0x0E, 0x1);
    AT86RF212_SPI_WriteRegister(0x02, 0x03);

    if (AT86RF212_SPI_ReadRegister(0x01) != 0x08)
    {
        otCliOutputFormat("ERROR: TRX_OFF: %i\r\n", AT86RF212_SPI_ReadRegister(0x01));
    }
    else
    {
        otCliOutputFormat("TRX_OFF: %b\r\n", AT86RF212_SPI_ReadRegister(0x01));
    }

    AT86RF212_SPI_WriteRegister(0x36, 0x0F);
    AT86RF212_SPI_WriteRegister(0x0C, 0x0C);
    AT86RF212_SPI_Transmit(sizeof(data), data);

    AT86RF212_SPI_WriteRegister(0x1C, 0x54);
    AT86RF212_SPI_WriteRegister(0x1C, 0x46);
    AT86RF212_SPI_WriteRegister(0x02, 0x09);

    test = AT86RF212_SPI_ReadRegister(0x01);
    while (test == 0x1F)
    {
        test = AT86RF212_SPI_ReadRegister(0x01);
        __asm("nop");
    }

    if (test != 0x09)
    {
        otCliOutputFormat("ERROR: PLL_LOCK: %i\r\n", test);
    }
    else
    {
        otCliOutputFormat("PLL_LOCK: %i\r\n", test);
    }

    AT86RF212_SPI_WriteRegister(0x02, 0x02);

    while (test == 0x09)
    {
        test = AT86RF212_SPI_ReadRegister(0x01);
        __asm("nop");
    }
    otCliOutputFormat("Status: %i\r\n", test);

    for (int i = 0; i < 500; i++)
    {
        __asm("nop");
    }

    otCliOutputFormat("DEBUG: %i\r\n", AT86RF212_SPI_ReadRegister(0x01));

    otCliOutputFormat("PART NUMBER: %i \r\n", AT86RF212_SPI_ReadRegister(0x1C));
}

void AT86RF212_SPI_TXframe(uint8_t length, uint8_t *data)
{
    uint8_t data_out[length + 1];
    uint8_t data_in[length + 1];

    data_out[0] = AT86RF212_FRAME_WRITE_FLAG;
    for (int i = 0; i < length; i++)
    {
        data_out[i + 1] = data[i];
    }

    SPIDRV_MTransferB(spiHandle, data_out, data_in, length + 1);
}
