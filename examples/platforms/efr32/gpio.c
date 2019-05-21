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
#include "em_usart.h"
#include "gpiointerrupt.h"
#include "hal-config.h"
#include "openthread-system.h"
#include <openthread/instance.h>


static otSysButtonCallback sButtonHandler;
static bool sButtonPressed;

/**
 * IRQ
 */

static void otSysButtonIRQ(uint8_t pin)
{
    OT_UNUSED_VARIABLE(pin);
    sButtonPressed = true;
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