#ifndef _PHY_H_
#define _PHY_H_

/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>


/*- Definitions ------------------------------------------------------------*/
#define PHY_RSSI_BASE_VAL_BPSK_20             (-100)
#define PHY_RSSI_BASE_VAL_BPSK_40             (-99)
#define PHY_RSSI_BASE_VAL_OQPSK_SIN_RC_100    (-98)
#define PHY_RSSI_BASE_VAL_OQPSK_SIN_250       (-97)
#define PHY_RSSI_BASE_VAL_OQPSK_RC_250        (-97)

#define PHY_HAS_RANDOM_NUMBER_GENERATOR
#define PHY_HAS_AES_MODULE

/*- Types ------------------------------------------------------------------*/
typedef struct PHY_DataInd_t
{
  uint8_t    *data;
  uint8_t    size;
  uint8_t    lqi;
  int8_t     rssi;
} PHY_DataInd_t;

enum
{
  PHY_STATUS_SUCCESS                = 0,
  PHY_STATUS_CHANNEL_ACCESS_FAILURE = 1,
  PHY_STATUS_NO_ACK                 = 2,
  PHY_STATUS_ERROR                  = 3,
};

/*- Prototypes -------------------------------------------------------------*/
void AT86RF212_Init(void);
void AT86RF212_SetRxState(bool rx);
void AT86RF212_SetChannel(uint8_t channel);
void AT86RF212_Set_Band(uint8_t band);
void AT86RF212_Set_Modulation(uint8_t modulation);
void AT86RF212_Set_PanId(uint16_t panId);
void AT86RF212_Set_Short_Addr(uint16_t addr);
void AT86RF212_Set_Tx_Power(uint8_t txPower);
void PHY_Sleep(void);
void PHY_Wakeup(void);
void AT86RF212_Data_Req(uint8_t *data, uint8_t size);
void PHY_DataConf(uint8_t status);
void PHY_DataInd(PHY_DataInd_t *ind);
void PHY_TaskHandler(void);

#ifdef PHY_ENABLE_RANDOM_NUMBER_GENERATOR
uint16_t PHY_RandomReq(void);
#endif

#ifdef PHY_ENABLE_AES_MODULE
void PHY_EncryptReq(uint8_t *text, uint8_t *key);
#endif

#ifdef PHY_ENABLE_ENERGY_DETECTION
int8_t PHY_EdReq(void);
#endif

#endif // _PHY_H_
