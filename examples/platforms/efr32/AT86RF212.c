#include "AT86RF212.h"
#include "AT86RF212_def.h"
#include "AT86RF212_spi.h"
#include "em_gpio.h"

/*- Definitions ------------------------------------------------------------*/
#define PHY_CRC_SIZE    2

/*- Types ------------------------------------------------------------------*/
typedef enum {
	PHY_STATE_INITIAL, PHY_STATE_IDLE, PHY_STATE_SLEEP, PHY_STATE_TX_WAIT_END,
} PhyState_t;

/*- Prototypes -------------------------------------------------------------*/
void AT86RF212_Wait_State(uint8_t state);
void AT86RF212_Trx_Set_State(uint8_t state);
int8_t AT86RF212_Rssi_Base_Value(void);
void AT86RF212_Set_Channel(void);
void AT86RF212_Set_Rx_State(void);

/*- Variables --------------------------------------------------------------*/
static PhyState_t phyState = PHY_STATE_INITIAL;
static uint8_t phyRxBuffer[128];
static bool phyRxState;
static uint8_t phyChannel;
static uint8_t phyBand;
static uint8_t phyModulation;

/*************************************************************************//**
 *****************************************************************************/

void AT86RF212_Init(void) {
	AT86RF212_SPI_Init();
	phyBand = 0;
	phyModulation = AT86RF212_SPI_ReadRegister(TRX_CTRL_2_REG) & 0x3f;

	AT86RF212_SPI_WriteRegister(TRX_STATE_REG, TRX_CMD_TRX_OFF);

	AT86RF212_SPI_WriteRegister(PHY_TX_PWR_REG, 0x41);

	AT86RF212_SPI_WriteRegister(TRX_CTRL_1_REG,
			(1 << TX_AUTO_CRC_ON) | (3 << SPI_CMD_MODE) | (1 << IRQ_MASK_MODE));

	AT86RF212_SPI_WriteRegister(TRX_CTRL_2_REG, (1 << RX_SAFE_MODE));
}

/*************************************************************************//**
 *****************************************************************************/

/*************************************************************************//**
 *****************************************************************************/
void AT86RF212_SetRxState(bool rx) {
	phyRxState = rx;
	AT86RF212_Set_Rx_State();
}

/*************************************************************************//**
 *****************************************************************************/
void AT86RF212_SetChannel(uint8_t channel) {
	phyChannel = channel;
	AT86RF212_Set_Channel();
}

/*************************************************************************//**
 *****************************************************************************/
void AT86RF212_Set_Band(uint8_t band) {
	phyBand = band;
	AT86RF212_Set_Channel();
}

/*************************************************************************//**
 *****************************************************************************/
void AT86RF212_Set_Modulation(uint8_t modulation) {
	phyModulation = modulation;
	AT86RF212_Set_Channel();
}

/*************************************************************************//**
 *****************************************************************************/
void AT86RF212_Set_PanId(uint16_t panId) {
	uint8_t *d = (uint8_t *) &panId;

	AT86RF212_SPI_WriteRegister(PAN_ID_0_REG, d[0]);
	AT86RF212_SPI_WriteRegister(PAN_ID_1_REG, d[1]);
}

/*************************************************************************//**
 *****************************************************************************/
void AT86RF212_Set_Short_Addr(uint16_t addr) {
	uint8_t *d = (uint8_t *) &addr;

	AT86RF212_SPI_WriteRegister(SHORT_ADDR_0_REG, d[0]);
	AT86RF212_SPI_WriteRegister(SHORT_ADDR_1_REG, d[1]);
	AT86RF212_SPI_WriteRegister(CSMA_SEED_0_REG, d[0] + d[1]);
}

/*************************************************************************//**
 *****************************************************************************/
void AT86RF212_Set_Tx_Power(uint8_t txPower) {
	uint8_t reg;

	reg = AT86RF212_SPI_ReadRegister(PHY_TX_PWR_REG) & ~0x0f;
	AT86RF212_SPI_WriteRegister(PHY_TX_PWR_REG, reg | txPower);
}

/*************************************************************************//**
 *****************************************************************************/
void PHY_Sleep(void) {
	AT86RF212_Trx_Set_State(TRX_CMD_TRX_OFF);
	phyState = PHY_STATE_SLEEP;
}

/*************************************************************************//**
 *****************************************************************************/
void PHY_Wakeup(void) {

	AT86RF212_Set_Rx_State();
	phyState = PHY_STATE_IDLE;
}

/*************************************************************************//**
 *****************************************************************************/
void AT86RF212_Data_Req(uint8_t *data, uint8_t size) {
	AT86RF212_Trx_Set_State(TRX_CMD_TX_ARET_ON);

	AT86RF212_SPI_ReadRegister(IRQ_STATUS_REG);

	AT86RF212_SPI_WriteByte(RF_CMD_FRAME_W);
	AT86RF212_SPI_WriteByte(size + PHY_CRC_SIZE);
	for (uint8_t i = 0; i < size; i++)
		AT86RF212_SPI_WriteByte(data[i]);

	phyState = PHY_STATE_TX_WAIT_END;

}

#ifdef PHY_ENABLE_RANDOM_NUMBER_GENERATOR
/*************************************************************************//**
 *****************************************************************************/
uint16_t AT86RF212_Req(void)
{
	uint16_t rnd = 0;
	uint8_t rndValue;

	AT86RF212_Trx_Set_State(TRX_CMD_RX_ON);

	for (uint8_t i = 0; i < 16; i += 2)
	{
		rndValue = (AT86RF212_SPI_ReadRegister(PHY_RSSI_REG) >> RND_VALUE) & 3;
		rnd |= rndValue << i;
	}

	AT86RF212_Set_Rx_State();

	return rnd;
}
#endif

#ifdef PHY_ENABLE_AES_MODULE
/*************************************************************************//**
 *****************************************************************************/
void PHY_EncryptReq(uint8_t *text, uint8_t *key)
{
	AT86RF212_SPI_WriteByte(RF_CMD_SRAM_W);
	AT86RF212_SPI_WriteByte(AES_CTRL_REG);
	AT86RF212_SPI_WriteByte((1<<AES_CTRL_MODE) | (0<<AES_CTRL_DIR));
	for (uint8_t i = 0; i < AES_BLOCK_SIZE; i++)
	AT86RF212_SPI_WriteByte(key[i]);

	AT86RF212_SPI_WriteByte(RF_CMD_SRAM_W);
	AT86RF212_SPI_WriteByte(AES_CTRL_REG);
	AT86RF212_SPI_WriteByte((0<<AES_CTRL_MODE) | (0<<AES_CTRL_DIR));
	for (uint8_t i = 0; i < AES_BLOCK_SIZE; i++)
	AT86RF212_SPI_WriteByte(text[i]);
	AT86RF212_SPI_WriteByte((1<<AES_CTRL_REQUEST) | (0<<AES_CTRL_MODE) | (0<<AES_CTRL_DIR));

	HAL_Delay(AES_CORE_CYCLE_TIME);

	AT86RF212_SPI_WriteByte(RF_CMD_SRAM_R);
	AT86RF212_SPI_WriteByte(AES_STATE_REG);
	for (uint8_t i = 0; i < AES_BLOCK_SIZE; i++)
	text[i] = AT86RF212_SPI_WriteByte(0);

}
#endif

/*************************************************************************//**
 *****************************************************************************/
void AT86RF212_Wait_State(uint8_t state) {
	while (state
			!= (AT86RF212_SPI_ReadRegister(TRX_STATUS_REG) & TRX_STATUS_MASK))
		;
}

/*************************************************************************//**
 *****************************************************************************/
void AT86RF212_Set_Channel(void) {
	uint8_t reg;

	reg = AT86RF212_SPI_ReadRegister(TRX_CTRL_2_REG) & ~0x3f;
	AT86RF212_SPI_WriteRegister(TRX_CTRL_2_REG, reg | phyModulation);

	AT86RF212_SPI_WriteRegister(CC_CTRL_1_REG, phyBand);

	if (0 == phyBand) {
		reg = AT86RF212_SPI_ReadRegister(PHY_CC_CCA_REG) & ~0x1f;
		AT86RF212_SPI_WriteRegister(PHY_CC_CCA_REG, reg | phyChannel);
	} else {
		AT86RF212_SPI_WriteRegister(CC_CTRL_0_REG, phyChannel);
	}
}

/*************************************************************************//**
 *****************************************************************************/
void AT86RF212_Set_Rx_State(void) {
	AT86RF212_Trx_Set_State(TRX_CMD_TRX_OFF);

	AT86RF212_SPI_ReadRegister(IRQ_STATUS_REG);

	if (phyRxState)
		AT86RF212_Trx_Set_State(TRX_CMD_RX_AACK_ON);
}

/*************************************************************************//**
 *****************************************************************************/
void AT86RF212_Trx_Set_State(uint8_t state) {
	AT86RF212_SPI_WriteRegister(TRX_STATE_REG, TRX_CMD_FORCE_TRX_OFF);
	AT86RF212_Wait_State(TRX_STATUS_TRX_OFF);

	AT86RF212_SPI_WriteRegister(TRX_STATE_REG, state);
	AT86RF212_Wait_State(state);
}

/*************************************************************************//**
 *****************************************************************************/
int8_t AT86RF212_Rssi_Base_Value(void) {
	bool oqpsk = (phyModulation & (1 << BPSK_OQPSK));
	bool sub = (phyModulation & (1 << SUB_MODE));
	bool rc = (phyModulation & (1 << OQPSK_SUB1_RC_EN));

	if (0 == oqpsk) {
		if (0 == sub)
			return PHY_RSSI_BASE_VAL_BPSK_20;
		else
			return PHY_RSSI_BASE_VAL_BPSK_40;
	} else {
		if (0 == sub)
			return PHY_RSSI_BASE_VAL_OQPSK_SIN_RC_100;
		else {
			if (0 == rc)
				return PHY_RSSI_BASE_VAL_OQPSK_SIN_250;
			else
				return PHY_RSSI_BASE_VAL_OQPSK_RC_250;
		}
	}
}

/*************************************************************************//**
 *****************************************************************************/
void PHY_TaskHandler(void) {
	if (PHY_STATE_SLEEP == phyState)
		return;

	if (AT86RF212_SPI_ReadRegister(IRQ_STATUS_REG) & (1 << TRX_END)) {
		if (PHY_STATE_IDLE == phyState) {
			// PHY_DataInd_t ind;
			uint8_t size;
			// int8_t rssi;

			// rssi = (int8_t) AT86RF212_SPI_ReadRegister(PHY_ED_LEVEL_REG);

			AT86RF212_SPI_WriteByte(RF_CMD_FRAME_R);
			size = AT86RF212_SPI_ReadRegister(0);
			for (uint8_t i = 0; i < size + 1/*lqi*/; i++)
				phyRxBuffer[i] = AT86RF212_SPI_ReadRegister(0);

			// ind.data = phyRxBuffer;
			// ind.size = size - PHY_CRC_SIZE;
			// ind.lqi = phyRxBuffer[size];
			// ind.rssi = rssi + AT86RF212_Rssi_Base_Value();

			AT86RF212_Wait_State(TRX_STATUS_RX_AACK_ON);
		}

		else if (PHY_STATE_TX_WAIT_END == phyState) {
			uint8_t status = (AT86RF212_SPI_ReadRegister(TRX_STATE_REG)
					>> TRAC_STATUS) & 7;

			if (TRAC_STATUS_SUCCESS == status)
				status = PHY_STATUS_SUCCESS;
			else if (TRAC_STATUS_CHANNEL_ACCESS_FAILURE == status)
				status = PHY_STATUS_CHANNEL_ACCESS_FAILURE;
			else if (TRAC_STATUS_NO_ACK == status)
				status = PHY_STATUS_NO_ACK;
			else
				status = PHY_STATUS_ERROR;

			AT86RF212_Set_Rx_State();
			phyState = PHY_STATE_IDLE;

		}
	}
}
