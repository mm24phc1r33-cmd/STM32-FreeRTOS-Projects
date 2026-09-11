/*
 * task_handler.c
 *
 *  Created on: Aug 31, 2026
 *      Author: Monika
 */

#ifndef TASK_HANDLER_H
#define TASK_HANDLER_H

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void menu_task(void *parameters);

void cmd_handler_task(void *param);

void print_task(void *parameters);

void led_task(void *parameters);

void rtc_task(void *parameters);

int extract_command(command_t *cmd);

void process_command(command_t *cmd);

#endif
