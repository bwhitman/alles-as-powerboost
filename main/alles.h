// alles.h
#ifndef __ALLES_H
#define __ALLES_H

#include <stdio.h>
#include <stddef.h>
#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_intr_alloc.h"
#include "esp_attr.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "lwip/netdb.h"
#include "driver/gpio.h"

#define MAX_TASKS 9

// Pins & buttons
#define BUTTON_WAKEUP 34
#define BUTTON_MINUS 17
#define BUTTON_WIFI 16
#define BUTTON_PLUS 0
#define ESP_INTR_FLAG_DEFAULT 0
#define CONFIG_I2S_LRCLK 25
#define CONFIG_I2S_BCLK 26
#define CONFIG_I2S_DIN 27
#define CONFIG_I2S_NUM 0 
#define BAT_SENSE_EN 32
#define CHARGE_STAT 33
#define POWER_5V_EN 21
#define BATT_SENSE_CHANNEL ADC_CHANNEL_7 // GPIO35 / ADC1_7
#define WALL_SENSE_CHANNEL ADC_CHANNEL_3 // GPIO39 / ADC1_3
#define CPU_MONITOR_0 13
#define CPU_MONITOR_1 12
#define CPU_MONITOR_2 15

void wifi_reconfigure();
extern esp_err_t buttons_init();
void esp_show_debug(uint8_t type);
void delay_ms(uint32_t ms);

#endif

// enums
#define DEVBOARD 0
#define ALLES_BOARD_V1 1
#define ALLES_BOARD_V2 2
#define ALLES_DESKTOP 3
#define BATTERY_STATE_CHARGING 0x01
#define BATTERY_STATE_CHARGED 0x02
#define BATTERY_STATE_DISCHARGING 0x04
#define BATTERY_VOLTAGE_4 0x10
#define BATTERY_VOLTAGE_3 0x20
#define BATTERY_VOLTAGE_2 0x40
#define BATTERY_VOLTAGE_1 0x80

// Status mask 
#define RUNNING 1
#define WIFI_MANAGER_OK 2
#define UPDATE 4




#endif



