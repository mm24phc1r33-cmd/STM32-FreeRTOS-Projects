/*
 * led_effect.c
 *
 * Created on: Aug 31, 2026
 * Author: Monika
 */

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* =========================================================
 *                EXTERNAL TIMER HANDLES
 * =========================================================
 */

extern TimerHandle_t handle_led_timer[4];


/* =========================================================
 *                  FUNCTION PROTOTYPES
 * =========================================================
 */

void led_effect(int n);
void led_effect_stop(void);
void led_effect_callback(TimerHandle_t xTimer);

void turn_off_all_leds(void);
void turn_on_all_leds(void);
void turn_on_even_leds(void);
void turn_on_odd_leds(void);

void LED_control(int value);

void LED_effect1(void);
void LED_effect2(void);
void LED_effect3(void);
void LED_effect4(void);


/* =========================================================
 *                  LED EFFECT START
 * =========================================================
 */

void led_effect(int n)
{
    /* Stop any previously running LED effect */
    led_effect_stop();

    /* Check valid effect number */
    if (n >= 1 && n <= 4)
    {
        if (handle_led_timer[n - 1] != NULL)
        {
            // Use 0 ticks to avoid blocking execution inside task context
            xTimerStart(handle_led_timer[n - 1], 0);
        }
    }
}


/* =========================================================
 *                  STOP LED EFFECT
 * =========================================================
 */

void led_effect_stop(void)
{
    int i;

    /* Stop all LED timers */
    for (i = 0; i < 4; i++)
    {
        if (handle_led_timer[i] != NULL)
        {
            xTimerStop(handle_led_timer[i], 0);
        }
    }

    /* Turn OFF all LEDs */
    turn_off_all_leds();
}


/* =========================================================
 *                  TIMER CALLBACK
 * =========================================================
 */

void led_effect_callback(TimerHandle_t xTimer)
{
    // Properly cast the uintptr_t timer ID stored from main.c
    uint32_t id = (uint32_t)(uintptr_t)pvTimerGetTimerID(xTimer);

    switch (id)
    {
        case 1:
            LED_effect1();
            break;

        case 2:
            LED_effect2();
            break;

        case 3:
            LED_effect3();
            break;

        case 4:
            LED_effect4();
            break;

        default:
            break;
    }
}


/* =========================================================
 *                  TURN OFF ALL LEDs
 * =========================================================
 */

void turn_off_all_leds(void)
{
    HAL_GPIO_WritePin(GPIOB, LED_GREEN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, LED_ORANGE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, LED_RED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, LED_YELLOW_Pin, GPIO_PIN_RESET);
}


/* =========================================================
 *                  TURN ON ALL LEDs
 * =========================================================
 */

void turn_on_all_leds(void)
{
    HAL_GPIO_WritePin(GPIOB, LED_GREEN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, LED_ORANGE_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, LED_RED_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, LED_YELLOW_Pin, GPIO_PIN_SET);
}


/* =========================================================
 *                    LED CONTROL
 * =========================================================
 */

void LED_control(int value)
{
    /* GREEN LED (Bit 0 -> PB0) */
    HAL_GPIO_WritePin(
        GPIOB,
        LED_GREEN_Pin,
        (value & (1 << 0)) ? GPIO_PIN_SET : GPIO_PIN_RESET
    );

    /* ORANGE LED (Bit 1 -> PB1) */
    HAL_GPIO_WritePin(
        GPIOB,
        LED_ORANGE_Pin,
        (value & (1 << 1)) ? GPIO_PIN_SET : GPIO_PIN_RESET
    );

    /* RED LED (Bit 2 -> PB2) */
    HAL_GPIO_WritePin(
        GPIOB,
        LED_RED_Pin,
        (value & (1 << 2)) ? GPIO_PIN_SET : GPIO_PIN_RESET
    );

    /* YELLOW LED (Bit 3 -> PB3) */
    HAL_GPIO_WritePin(
        GPIOB,
        LED_YELLOW_Pin,
        (value & (1 << 3)) ? GPIO_PIN_SET : GPIO_PIN_RESET
    );
}


/* =========================================================
 *                  EVEN LEDs
 * =========================================================
 */

void turn_on_even_leds(void)
{
    LED_control(0x05); // GREEN (PB0) + RED (PB2)
}


/* =========================================================
 *                   ODD LEDs
 * =========================================================
 */

void turn_on_odd_leds(void)
{
    LED_control(0x0A); // ORANGE (PB1) + YELLOW (PB3)
}


/* =========================================================
 *                    EFFECT 1
 * =========================================================
 */

void LED_effect1(void)
{
    static int flag = 0;
    flag ^= 1;

    if (flag)
    {
        turn_on_all_leds();
    }
    else
    {
        turn_off_all_leds();
    }
}


/* =========================================================
 *                    EFFECT 2
 * =========================================================
 */

void LED_effect2(void)
{
    static int flag = 0;
    flag ^= 1;

    if (flag)
    {
        turn_on_even_leds();
    }
    else
    {
        turn_on_odd_leds();
    }
}


/* =========================================================
 *                    EFFECT 3 (Forward Chaser)
 * =========================================================
 */

void LED_effect3(void)
{
    static int i = 0;
    LED_control(0x01 << (i % 4));
    i++;
}


/* =========================================================
 *                    EFFECT 4 (Reverse Chaser)
 * =========================================================
 */

void LED_effect4(void)
{
    static int i = 0;
    LED_control(0x08 >> (i % 4));
    i++;
}
