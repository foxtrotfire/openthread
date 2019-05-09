/**
 * @file
 *      This file implements the platform specific abstraction for GPIO
 * 
 */

#include <stdbool.h>
#include "em_device.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_emu.h"
#include "em_core.h"
#include "em_timer.h"
#include "em_usart.h"
#include "gpiointerrupt.h"
#include "hal-config.h"
#include "openthread-system.h"
#include <openthread/instance.h>


static otSysButtonCallback sButtonHandler;
static otSysTimer0Callback sTimer0Handler;
static bool sButtonPressed;
static bool sTimer0Expired;

#define TX_BUFFER_SIZE   10
#define RX_BUFFER_SIZE   TX_BUFFER_SIZE

uint8_t TxBuffer[TX_BUFFER_SIZE] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
		0x07, 0x08, 0x09 };
uint32_t TxBufferIndex = 0;

uint8_t RxBuffer[RX_BUFFER_SIZE] = { 0 };
uint32_t RxBufferIndex = 0;

/**
 * IRQ
 */

static void otSysButtonIRQ(uint8_t pin)
{
    OT_UNUSED_VARIABLE(pin);
    sButtonPressed = true;
}

static void otSysTimer0IRQ(void)
{
    sTimer0Expired = true;
}

void otSysInitTimer1(void)
{
  // Enable clock for TIMER0 module
  CMU_ClockEnable(cmuClock_TIMER1, true);

  // Configure TIMER0 Compare/Capture for output compare
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
  timerCCInit.mode = timerCCModeCompare;
  timerCCInit.cmoa = timerOutputActionToggle; // Toggle output on compare match
  TIMER_InitCC(TIMER0, 0, &timerCCInit);



  // Initialize and start timer with highest prescale
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  timerInit.enable = false;
  timerInit.prescale = TIMER_PRESCALE;
  timerInit.oneShot = true; // Generate only one pulse
  TIMER_Init(TIMER1, &timerInit);

  // Set the first compare value
  compareValue1 = CMU_ClockFreqGet(cmuClock_TIMER0)
                    * NUM_SEC_DELAY
                    / (1 << TIMER_PRESCALE);
  TIMER_CompareSet(TIMER1, 0, compareValue1);

  // Set the second compare value (don't actually use it, just set the global so
  // that it can be used by the handler later)
  compareValue2 = (CMU_ClockFreqGet(cmuClock_TIMER0)
                    * PULSE_WIDTH
                    / 1000
                    / (1 << TIMER_PRESCALE))
                    + compareValue1;

  // Enable TIMER0 interrupts
  TIMER_IntEnable(TIMER1, TIMER_IEN_CC0);
  NVIC_EnableIRQ(TIMER1_IRQn);

  // Enable the TIMER
    TIMER_Enable(TIMER1, true); 

}


/*
 * Initialize LEDs
 */
void otSysLedInit(void)
{
    /* Configure GPIO clock */
    CMU_ClockEnable(cmuClock_GPIO, true);

    /* Configure LEDs as output */
    GPIO_PinModeSet(PORTIO_GPIO_LED0_PORT, PORTIO_GPIO_LED0_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(PORTIO_GPIO_LED1_PORT, PORTIO_GPIO_LED1_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(PORTIO_GPIO_LED2_PORT, PORTIO_GPIO_LED2_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(PORTIO_GPIO_LED3_PORT, PORTIO_GPIO_LED3_PIN, gpioModePushPull, 0);
}

void otSysButtonInit(otSysButtonCallback aCallback)
{
    /* Configure GPIO clock */
    CMU_ClockEnable(cmuClock_GPIO, true);
    /* Enable IRQ for button */
    GPIOINT_Init();
    /* Configure Button as input */
    GPIO_PinModeSet(PORTIO_GPIO_BTN0_PORT, PORTIO_GPIO_BTN0_PIN, gpioModeInputPullFilter, 1);
    GPIOINT_CallbackRegister(PORTIO_GPIO_BTN0_PIN, &otSysButtonIRQ);
    GPIO_IntEnable(1<<PORTIO_GPIO_BTN0_PIN);


    sButtonHandler = aCallback;
    sButtonPressed = false;

    GPIO_IntConfig(PORTIO_GPIO_BTN0_PORT, PORTIO_GPIO_BTN0_PIN, false, true, true);
}

void otSysButtonProcess(otInstance *aInstance)
{
    if (sButtonPressed)
    {
        sButtonPressed = false;
        sButtonHandler(aInstance);
    }
}

void otSysTimer0Process(otInstance *aInstance)
{
    if(sTimer0Expired)
    {
        sTimer0Expired = false;
        sTimer0Handler(aInstance);
    }
}

/*
 * Set a single LED
 */
void otSysLedSet(uint8_t aLed, bool aOn)
{
    switch(aLed)
    {
        case 0: 
            if(aOn)
            {
                GPIO_PinOutSet(PORTIO_GPIO_LED0_PORT, PORTIO_GPIO_LED0_PIN);
            }
            else
            {
                GPIO_PinOutClear(PORTIO_GPIO_LED0_PORT, PORTIO_GPIO_LED0_PIN);
            }
            break;
        case 1: 
            if(aOn)
            {
                GPIO_PinOutSet(PORTIO_GPIO_LED1_PORT, PORTIO_GPIO_LED1_PIN);
            }
            else
            {
                GPIO_PinOutClear(PORTIO_GPIO_LED1_PORT, PORTIO_GPIO_LED1_PIN);
            }
            break;
        case 2: 
            if(aOn)
            {
                GPIO_PinOutSet(PORTIO_GPIO_LED2_PORT, PORTIO_GPIO_LED2_PIN);
            }
            else
            {
                GPIO_PinOutClear(PORTIO_GPIO_LED2_PORT, PORTIO_GPIO_LED2_PIN);
            }
            break;
        case 3: 
            if(aOn)
            {
                GPIO_PinOutSet(PORTIO_GPIO_LED3_PORT, PORTIO_GPIO_LED3_PIN);
            }
            else
            {
                GPIO_PinOutClear(PORTIO_GPIO_LED3_PORT, PORTIO_GPIO_LED3_PIN);
            }
            break;
        default:
            break;
    }
}

/*
 * Toggle a single LED
 */
void otSysLedToggle(uint8_t aLed)
{
    switch(aLed)
    {
        case 0: 
            if(GPIO_PinOutGet(PORTIO_GPIO_LED0_PORT, PORTIO_GPIO_LED0_PIN))
            {
                GPIO_PinOutClear(PORTIO_GPIO_LED0_PORT, PORTIO_GPIO_LED0_PIN);
            }
            else
            {
                GPIO_PinOutSet(PORTIO_GPIO_LED0_PORT, PORTIO_GPIO_LED0_PIN);
            }
            break;
        case 1: 
            if(GPIO_PinOutGet(PORTIO_GPIO_LED1_PORT, PORTIO_GPIO_LED1_PIN))
            {
                GPIO_PinOutClear(PORTIO_GPIO_LED1_PORT, PORTIO_GPIO_LED1_PIN);
            }
            else
            {
                GPIO_PinOutSet(PORTIO_GPIO_LED1_PORT, PORTIO_GPIO_LED1_PIN);
            }
            break;
        case 2: 
            if(GPIO_PinOutGet(PORTIO_GPIO_LED2_PORT, PORTIO_GPIO_LED2_PIN))
            {
                GPIO_PinOutClear(PORTIO_GPIO_LED2_PORT, PORTIO_GPIO_LED2_PIN);
            }
            else
            {
                GPIO_PinOutSet(PORTIO_GPIO_LED2_PORT, PORTIO_GPIO_LED2_PIN);
            }
            break;
        case 3: 
            if(GPIO_PinOutGet(PORTIO_GPIO_LED3_PORT, PORTIO_GPIO_LED3_PIN))
            {
                GPIO_PinOutClear(PORTIO_GPIO_LED3_PORT, PORTIO_GPIO_LED3_PIN);
            }
            else
            {
                GPIO_PinOutSet(PORTIO_GPIO_LED3_PORT, PORTIO_GPIO_LED3_PIN);
            }
            break;
        default:
            break;
    }
}