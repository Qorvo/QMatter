/*
 * Copyright (c) 2017, Qorvo Inc
 *
 * This software is owned by Qorvo Inc
 * and protected under applicable copyright laws.
 * It is delivered under the terms of the license
 * and is intended and supplied for use solely and
 * exclusively with products manufactured by
 * Qorvo Inc.
 *
 *
 * THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 * CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 * IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 * LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * QORVO INC. SHALL NOT, IN ANY
 * CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 * FOR ANY REASON WHATSOEVER.
 *
 * $Header$
 * $Change$
 * $DateTime$
 *
 */

 /** @file pwm.c
 *
 * Master SPI example application
 * This example shows configuration of SPI master to read/write/erase flash on the DB09 development board.
 * This example demonstartes write or read single byte, multiple bytes to or from flash, erase flash operations.
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "hal.h"
#include "gpHal.h"
#include "gpBaseComps.h"
#include "gpLog.h"
#include "gpCom.h"
#include "gpSched.h"

#include "pwm.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP

#ifndef PWM_APPLICATION_STEPS

#ifdef PWM_CHANNEL_SPEAKER
#ifdef HAL_DIVERSITY_PWM_WITH_DMA
#define PWM_SCENARIOS 3
#else
#define PWM_SCENARIOS 2
#endif
#else
#define PWM_SCENARIOS 1
#endif

/* We only want to the execute the LED tests if ALL the LEDs are defined: */
#if defined(PWM_GPIO_RED) && defined(PWM_GPIO_GREEN) && defined(PWM_GPIO_BLUE)
#define PERFORM_LED_TESTS
#else
#undef PERFORM_LED_TESTS
#endif //defined(PWM_GPIO_RED) && defined(PWM_GPIO_GREEN) && defined(PWM_GPIO_BLUE)

#if defined PERFORM_LED_TESTS && defined PWM_CHANNEL_GPIO_PIN
    #error "PERFORM_LED_TESTS and PWM_CHANNEL_GPIO_PIN cannot be used simultaneously - code moficiations would be necessary"
#endif

#endif //PWM_APPLICATION_STEPS

/*****************************************************************************
 *                    LED Macro Definitions
 *****************************************************************************/

#define LED_PWM_COUNTERRESOLUTION 2

#ifndef PWM_APPLICATION_STEPS

/* { Red, Green, Blue } thresholds */
#define LED_COLOR_RED    {0xFF, 0x00, 0x00}
#define LED_COLOR_ORANGE {0xFF, 0x3F, 0x00}
#define LED_COLOR_YELLOW {0xFF, 0xFF, 0x00}
#define LED_COLOR_GREEN  {0x00, 0xFF, 0x00}
#define LED_COLOR_BLUE   {0x00, 0x00, 0xFF}
#define LED_COLOR_PURPLE {0xFF, 0x00, 0xFF}
#define LED_COLOR_WHITE  {0xFF, 0xFF, 0xFF}

/* PWM channel mapping to RGB LED */
#define PWM_CHANNEL_RED   PWM_GPIO_RED
#define PWM_CHANNEL_GREEN PWM_GPIO_GREEN
#define PWM_CHANNEL_BLUE  PWM_GPIO_BLUE

/*****************************************************************************
 *                    Speaker Macro Definitions
 *****************************************************************************/

#ifdef PWM_CHANNEL_SPEAKER

#define SPEAKER_PWM_CARRIER           3
#define SPEAKER_PWM_COUNTERRESOLUTION 255
#endif //PWM_CHANNEL_SPEAKER

#endif //PWM_APPLICATION_STEPS

/*****************************************************************************
 *                    PWM Steps Macro Definitions
 *****************************************************************************/

#ifdef PWM_APPLICATION_STEPS

/** @brief Button debounce check delay in ms */
#define DELAY_DEBOUNCE_MS      10


#endif //PWM_APPLICATION_STEPS

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

#ifdef PWM_APPLICATION_STEPS

#if PWM_OUTPUT_COUNT == 6

//Array of set PWM values in 0.01 percent
static const UInt16 Application_PWM_Step_Sequence[][6] = {
    {1000 , 0    , 0    , 0    , 0    , 0    },
    {0    , 1000 , 0    , 0    , 0    , 0    },
    {0    , 0    , 1000 , 0    , 0    , 0    },
    {0    , 0    , 0    , 1000 , 0    , 0    },
    {0    , 0    , 0    , 0    , 1000 , 0    },
    {0    , 0    , 0    , 0    , 0    , 1000 },
    {0    , 100  , 1000 , 9000 , 9900 , 10000},
    {1000 , 2500 , 4000 , 6000 , 7500 , 9000 }
};

#elif PWM_OUTPUT_COUNT == 8

//Array of set PWM values in 0.01 percent
static const UInt16 Application_PWM_Step_Sequence[][8] = {
    {1000 , 0    , 0    , 0    , 0    , 0    , 0    , 0    },
    {0    , 1000 , 0    , 0    , 0    , 0    , 0    , 0    },
    {0    , 0    , 1000 , 0    , 0    , 0    , 0    , 0    },
    {0    , 0    , 0    , 1000 , 0    , 0    , 0    , 0    },
    {0    , 0    , 0    , 0    , 1000 , 0    , 0    , 0    },
    {0    , 0    , 0    , 0    , 0    , 1000 , 0    , 0    },
    {0    , 0    , 0    , 0    , 0    , 0    , 1000 , 0    },
    {0    , 0    , 0    , 0    , 0    , 0    , 0    , 1000 },
    {0    , 100  , 200  , 1000 , 9000 , 9800 , 9900 , 10000},
    {1000 , 2000 , 3000 , 4000 , 6000 , 7000 , 8000 , 9000 }
};

#elif PWM_OUTPUT_COUNT == 2

//Array of set PWM values in 0.01 percent
static const UInt16 Application_PWM_Step_Sequence[][2] = {
    {1000 , 0    },
    {0    , 1000 },
    {0    , 100  },
    {9900 , 9800 },
    {1000 , 9000 },
};

#endif //PWM_OUTPUT_COUNT

static UInt8 Application_StepCount;
static Bool Application_BtnPressed;


#else //PWM_APPLICATION_STEPS

#ifdef PERFORM_LED_TESTS
static const UInt16 Application_Colors[][3] = {
    LED_COLOR_RED,
    LED_COLOR_ORANGE,
    LED_COLOR_YELLOW,
    LED_COLOR_GREEN,
    LED_COLOR_BLUE,
    LED_COLOR_PURPLE,
    LED_COLOR_WHITE,
};

static const char Application_ColorNames[] = {
    'R',
    'O',
    'Y',
    'G',
    'B',
    'P',
    'W'
};

static UInt8 Application_ColorCount;
#endif //PERFORM_LED_TESTS

#ifdef PWM_CHANNEL_GPIO_PIN
static UInt8 Application_DutyCycleCount;
static const UInt16 Application_DutyCycles[] = {52,128,204};
static const UInt8 Application_DutyCyclesPercentage[] = {20,50,80};

#endif //PWM_CHANNEL_GPIO_PIN

#ifdef PWM_CHANNEL_SPEAKER
static UInt8 Application_SpeakerCount;
#endif //PWM_CHANNEL_SPEAKER

static UInt8 Application_Counter;

#endif //PWM_APPLICATION_STEPS

#ifndef PWM_APPLICATION_STEPS

/*****************************************************************************
 *                    RGB LED steering
 *****************************************************************************/

#ifdef PERFORM_LED_TESTS
/** @brief Setup PWM signals used for RGB LED */
static void Application_LedInit(void)
{
    Application_ColorCount = 0;

    //Set to a smaller scale - max dutycycle:
    // 6 bit (for brightness control)
    // 8-bit (from definition)
    hal_SetFrequencyCorrectionFactor(LED_PWM_COUNTERRESOLUTION);

    hal_SetDutyCycle(PWM_CHANNEL_RED,   0x0);
    hal_SetDutyCycle(PWM_CHANNEL_GREEN, 0x0);
    hal_SetDutyCycle(PWM_CHANNEL_BLUE,  0x0);

    hal_SetChannelEnabled(PWM_CHANNEL_RED, true);
    hal_SetChannelEnabled(PWM_CHANNEL_GREEN, true);
    hal_SetChannelEnabled(PWM_CHANNEL_BLUE, true);

    hal_EnablePwm(true);
}
/** @brief Free up PWM for other usage */
static void Application_LedDeInit(void)
{
    hal_EnablePwm(false);

    hal_SetChannelEnabled(PWM_CHANNEL_RED, false);
    hal_SetChannelEnabled(PWM_CHANNEL_GREEN, false);
    hal_SetChannelEnabled(PWM_CHANNEL_BLUE, false);
    hal_SetFrequencyCorrectionFactor(0);
}

/** @brief Cycle through different colors */
static Bool Application_LedPattern(void)
{
    UIntLoop i;
    GP_LOG_SYSTEM_PRINTF("Switching to color: %c", 0, Application_ColorNames[Application_ColorCount]);

    //Cycle brightness levels to fade in color
    for(i = 0; i < 6; i++)
    {
        hal_SetDutyCycle(PWM_CHANNEL_RED,   Application_Colors[Application_ColorCount][0] << (i));
        hal_SetDutyCycle(PWM_CHANNEL_GREEN, Application_Colors[Application_ColorCount][1] << (i));
        hal_SetDutyCycle(PWM_CHANNEL_BLUE,  Application_Colors[Application_ColorCount][2] << (i));
        HAL_WAIT_MS(50);
    }

    Application_ColorCount++;
    Application_ColorCount%= number_of_elements(Application_Colors);

    return (Application_ColorCount == 0);
}
#endif //PERFORM_LED_TESTS


/*****************************************************************************
 *                    GPIO PWM Duty Cycle
 *****************************************************************************/
#ifdef PWM_CHANNEL_GPIO_PIN
/** @brief Setup PWM signal for gpio */
static void Application_GpioPWMInit(void)
{
    hal_SetDutyCycle(PWM_CHANNEL_GPIO_PIN, 0);
    // Configure PWM for 256 levels, 15.625 kS/s
    hal_ConfigPWM(255, 2, true, false, PWM_CHANNEL_GPIO_PIN);
    hal_SetChannelEnabled(PWM_CHANNEL_GPIO_PIN, true);
    hal_EnablePwm(true);
}
/** @brief Free up PWM for other usage */
static void Application_GpioPWMDeInit(void)
{
    hal_EnablePwm(false);
    hal_SetChannelEnabled(PWM_CHANNEL_GPIO_PIN, false);
    hal_SetPrescalerCounterWrapPower(0);
}

/** @brief Cycle through diffrent dutycycles */
static Bool Application_GpioPWMPattern(void)
{
    GP_LOG_SYSTEM_PRINTF("Setting dutycycle to %u percentage", 0, Application_DutyCyclesPercentage[Application_DutyCycleCount]);

    hal_SetDutyCycle(PWM_CHANNEL_GPIO_PIN, Application_DutyCycles[Application_DutyCycleCount]);

    Application_DutyCycleCount++;
    Application_DutyCycleCount%= number_of_elements(Application_DutyCycles);

    return (Application_DutyCycleCount == 0);

}
#endif //PWM_CHANNEL_GPIO_PIN

/*****************************************************************************
 *                    Speaker steering
 *****************************************************************************/
#ifdef PWM_CHANNEL_SPEAKER

/** @brief Setup PWM signal used for speaker */
static void Application_SpeakerInit(void)
{
    Application_SpeakerCount = 0;

    hal_SetChannelEnabled(PWM_CHANNEL_SPEAKER, false);
    hal_SetDutyCycle(PWM_CHANNEL_SPEAKER, 0x80); //50%

    //Settings affect all PWM channels
    hal_ConfigPWM(SPEAKER_PWM_COUNTERRESOLUTION, SPEAKER_PWM_CARRIER, true, false, PWM_CHANNEL_SPEAKER);
    hal_EnablePwm(true);
}

/** @brief Free up PWM for other usage */
static void Application_SpeakerDeInit(void)
{
    hal_EnablePwm(false);
    hal_SetChannelEnabled(PWM_CHANNEL_SPEAKER, false);
    hal_SetPrescalerCounterWrapPower(0);

}

/** @brief Cycle through different frequencies */
static Bool Application_PlayTone(void)
{
    GP_LOG_SYSTEM_PRINTF("Playing tone:%u", 0, Application_SpeakerCount);

    //Adjust frequency of PWM
    hal_SetPrescalerCounterWrapPower(7-Application_SpeakerCount);

    //Play tone
    hal_SetChannelEnabled(PWM_CHANNEL_SPEAKER, true);
    HAL_WAIT_MS(100);
    hal_SetChannelEnabled(PWM_CHANNEL_SPEAKER, false);

    Application_SpeakerCount++;
    Application_SpeakerCount%=4;

    return (Application_SpeakerCount == 0);
}
#endif //PWM_CHANNEL_SPEAKER

/*****************************************************************************
 *                    Sampled output
 *****************************************************************************/
#if defined(PWM_CHANNEL_SPEAKER) && defined(HAL_DIVERSITY_PWM_WITH_DMA)

#include "pwm_qorvo_samples.h"

static UInt16 Application_sampleptr;

Bool halPWM_cbWriteDmaBufferIndication(UInt16 sizeToFill, UInt8* dma_Buffer, UInt16 writeIndex)
{
    if (Application_sampleptr >= sizeof(Application_qorvo_samples))
    {
        return false;
    }
    else
    {
        UInt16 k = sizeof(Application_qorvo_samples) - Application_sampleptr;
        if (k > sizeToFill)
        {
            k = sizeToFill;
        }
        MEMCPY(dma_Buffer + writeIndex, Application_qorvo_samples + Application_sampleptr, k);
        Application_sampleptr += k;
        return true;
    }
}

static void Application_SampleInit(void)
{
    GP_LOG_SYSTEM_PRINTF("Playing sampled audio", 0);

    Application_sampleptr = 0;

    // Configure PWM for 256 levels, 15.625 kS/s
    hal_ConfigPWM(255, 2, false, false, PWM_CHANNEL_SPEAKER);

    halPWM_DmaStart();
}

/** @brief Free up PWM for other usage */
static void Application_SampleDeInit(void)
{
    halPWM_DmaStop();
}

#endif //defined(PWM_CHANNEL_SPEAKER) && defined(HAL_DIVERSITY_PWM_WITH_DMA)
#endif //PWM_APPLICATION_STEPS

#ifdef PWM_APPLICATION_STEPS

/*****************************************************************************
 *                    PWM Step Application
 *****************************************************************************/


/** @brief Setup PWM signals used for stepping application */
static void Application_StepInit(void)
{
    UIntLoop i;
    Application_StepCount = 0;
    Application_BtnPressed = false;

    //Init PWM in the hal
    hal_InitPWM();

    //Set to a smaller scale - max dutycycle:
    hal_SetFrequencyCorrectionFactor(LED_PWM_COUNTERRESOLUTION);

    //Enable all PWM channels and initalize to 0 duty cycle
    for(i=0;i<PWM_OUTPUT_COUNT;i++)
    {
        hal_SetDutyCycle(i, 0x0);
        hal_SetChannelEnabled(i, true);
    }

    hal_EnablePwm(true);

    //Initialize the button for cycling through the Steps
    /* Set internal pull up */
    hal_gpioModePU(GPIO_BTTN_NEXT_PWM_STEP, true);
    /* configure push pull - input */
    hal_gpioModePP(gpios[GPIO_BTTN_NEXT_PWM_STEP], false);

}


/** @brief Free up PWM for other usage */
static void Application_StepDeInit(void)
{
    GP_LOG_SYSTEM_PRINTF("Calling Application_StepDeInit", 0);

    hal_EnablePwm(false);
}


/** @brief Cycle through different colors */
static Bool Application_StepNext(void)
{
    UIntLoop i;
    GP_LOG_SYSTEM_PRINTF("Switching to PWM sequence: %d", 0, Application_StepCount);

    if (Application_StepCount < number_of_elements(Application_PWM_Step_Sequence))
    {

        //Set PWM duty cycle
        for(i=0;i<PWM_OUTPUT_COUNT;i++)
        {
            hal_SetDutyCyclePercentage(i, Application_PWM_Step_Sequence[Application_StepCount][i]);
        }
    }
    else if(Application_StepCount < number_of_elements(Application_PWM_Step_Sequence)+1)
    {

        //Invert PWM
        for(i=0;i<PWM_OUTPUT_COUNT;i++)
        {
            hal_InvertOutput(i, true);
        }
    }
    else if(Application_StepCount < number_of_elements(Application_PWM_Step_Sequence)+2)
    {
        //De-Invert PWM
        for(i=0;i<PWM_OUTPUT_COUNT;i++)
        {
            hal_InvertOutput(i, false);
        }
    }

    //Increase counter
    Application_StepCount++;
    Application_StepCount %= (number_of_elements(Application_PWM_Step_Sequence)+2);

    //Return true if the entire cycle has completed
    return (Application_StepCount == 0);
}

/** @brief Function to handle GPIO changes - Button press
*/
Bool Application_PollButton(void)
{
    Bool state;

    /* Push Button for next step */
    state = hal_gpioGet(gpios[GPIO_BTTN_NEXT_PWM_STEP]);
    if (!state)
    {
        if (Application_BtnPressed)
        {
            /* check for debounce */
            HAL_WAIT_MS(DELAY_DEBOUNCE_MS);
            if (!(hal_gpioGet(gpios[GPIO_BTTN_NEXT_PWM_STEP])))
            {
                Application_BtnPressed = false;
                return Application_StepNext();
            }
        }
    }
    else
    {
        /* check for debounce */
        HAL_WAIT_MS(DELAY_DEBOUNCE_MS);
        if (hal_gpioGet(gpios[GPIO_BTTN_NEXT_PWM_STEP]))
        {
            Application_BtnPressed = true;
        }
    }

    //Return false to indicate the cycle is not completed
    return false;
}

#endif //PWM_APPLICATION_STEPS

/*****************************************************************************
 *                    Application Init
 *****************************************************************************/

#ifdef PWM_APPLICATION_STEPS
/** @brief Run Steps application */
void Application_Steps(void)
{
    GP_LOG_SYSTEM_PRINTF("Application_Steps Started", 0);
    Bool phaseDone = false;


    while(!phaseDone)
    {
        /* Check input button */
        phaseDone = Application_PollButton();
    }

    //deinit all pins and blocks
    Application_StepDeInit();
}
#endif //PWM_APPLICATION_STEPS

#ifndef PWM_APPLICATION_STEPS
/** @brief Run PWM application */
void Application_Pwm()
{
    Bool phaseDone = true;
    while(1)
    {

        if (false)
        {
            // Force an "else if" conditioned on diversities.
        }

#ifdef PERFORM_LED_TESTS
        else if(Application_Counter == 0)
        {
            if(phaseDone)
            {
                Application_LedInit();
            }
            //Cycle through colors
            phaseDone = Application_LedPattern();
            HAL_WAIT_MS(500);
            if(phaseDone)
            {
                Application_LedDeInit();
            }
        }
#endif //PERFORM_LED_TESTS

#ifdef PWM_CHANNEL_GPIO_PIN
        else if(Application_Counter == 0)
        {
            if(phaseDone)
            {
                Application_GpioPWMInit();
            }
            phaseDone = Application_GpioPWMPattern();
            HAL_WAIT_MS(500);
            if(phaseDone)
            {
                Application_GpioPWMDeInit();
            }
        }
#endif //PWM_CHANNEL_GPIO_PIN
#ifdef PWM_CHANNEL_SPEAKER

        else if(Application_Counter == 1)
        {
            if(phaseDone)
            {
                Application_SpeakerInit();
            }
            //Cycle through speaker frequencies

            phaseDone = Application_PlayTone();
            GP_LOG_SYSTEM_PRINTF("Application_PlayTone phaseDone %u", 0, phaseDone);

            HAL_WAIT_MS(500);
            if(phaseDone)
            {
                Application_SpeakerDeInit();
            }
        }

#ifdef HAL_DIVERSITY_PWM_WITH_DMA

        else if(Application_Counter == 2)
        {
            Application_SampleInit();
            HAL_WAIT_MS(2000);
            Application_SampleDeInit();
            phaseDone = true;
        }

#endif //HAL_DIVERSITY_PWM_WITH_DMA
#endif //PWM_CHANNEL_SPEAKER

        if(phaseDone)
        {
            Application_Counter++;
            Application_Counter%=PWM_SCENARIOS;

            LED_INDICATOR_ON();
            HAL_WAIT_MS(50);
            LED_INDICATOR_OFF();
            HAL_WAIT_MS(50);
            LED_INDICATOR_ON();
            HAL_WAIT_MS(50);
            LED_INDICATOR_OFF();
        }
        else
        {
            HAL_WAIT_MS(150);
        }

        HAL_LED_SET(RED);
        HAL_WAIT_MS(50);
        HAL_LED_CLR(RED);
    }
}
#endif //!PWM_APPLICATION_STEPS

/** @brief Initialize application */
void Application_Init(void)
{
    // GP_BSP_IR_DEINIT();
    // GP_BSP_PWM0_INIT();
    /* Initialize whole stack */
    gpBaseComps_StackInit();
    HAL_WDT_DISABLE();

#ifdef PWM_APPLICATION_STEPS
    // Initialize for the application
    Application_StepInit();

    //Run the steps application
    Application_Steps();
#endif //PWM_APPLICATION_STEPS

#ifndef PWM_APPLICATION_STEPS
    //Run the PWM application
    Application_Pwm();
#endif //!PWM_APPLICATION_STEPS
}
