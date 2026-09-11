/*
 * led_effect.h
 *
 *  Created on: Aug 31, 2026
 *      Author: Monika
 */

#ifndef INC_LED_EFFECT_H_
#define INC_LED_EFFECT_H_

#include "main.h"
#include "FreeRTOS.h"
#include "timers.h"

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

#endif /* INC_LED_EFFECT_H_ */
