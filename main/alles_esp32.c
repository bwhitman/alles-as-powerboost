// alles_esp32.c
// Alles multicast synthesizer
// Brian Whitman
// brian@variogr.am

#include "alles.h"
#define configUSE_TASK_NOTIFICATIONS 1
//#define configTASK_NOTIFICATION_ARRAY_ENTRIES 2
#define MAX_WIFI_WAIT_S 120

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_intr_alloc.h"
#include "esp_attr.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_sleep.h"
#include "driver/uart.h"
#include "nvs_flash.h"
#include "power.h"
// This can be 32 bit, int32_t -- helpful for digital output to a i2s->USB teensy3 board

extern esp_err_t buttons_init();
uint8_t board_level;
uint8_t status;
uint8_t debug_on = 0;


void delay_ms(uint32_t ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

uint8_t board_level = ALLES_BOARD_V2;
uint8_t status = RUNNING;

char githash[8];

// Button event
extern xQueueHandle gpio_evt_queue;

// Battery status for V2 board. If no v2 board, will stay at 0
uint8_t battery_mask = 0;



void power_monitor() {
    power_status_t power_status;

    const esp_err_t ret = power_read_status(&power_status);
    if(ret != ESP_OK)
        return;

    // print a debugging power status every few seconds to the monitor 
    
    char buf[100];
    snprintf(buf, sizeof(buf),
        "powerStatus: power_source=\"%s\",charge_status=\"%s\",wall_v=%0.3f,battery_v=%0.3f\n",
        (power_status.power_source == POWER_SOURCE_WALL ? "wall" : "battery"),
        (power_status.charge_status == POWER_CHARGE_STATUS_CHARGING ? "charging" :
            (power_status.charge_status == POWER_CHARGE_STATUS_CHARGED ? "charged" : " discharging")),
        power_status.wall_voltage/1000.0,
        power_status.battery_voltage/1000.0
        );

    printf(buf);
    

    battery_mask = 0;

    switch(power_status.charge_status) {
        case POWER_CHARGE_STATUS_CHARGED:
            battery_mask = battery_mask | BATTERY_STATE_CHARGED;
            break;
        case POWER_CHARGE_STATUS_CHARGING:
            battery_mask = battery_mask | BATTERY_STATE_CHARGING;
            break;
        case POWER_CHARGE_STATUS_DISCHARGING:
            battery_mask = battery_mask | BATTERY_STATE_DISCHARGING;
            break;        
    }

    float voltage = power_status.battery_voltage/1000.0;
    if(voltage > 3.95) battery_mask = battery_mask | BATTERY_VOLTAGE_4; else 
    if(voltage > 3.80) battery_mask = battery_mask | BATTERY_VOLTAGE_3; else 
    if(voltage > 3.60) battery_mask = battery_mask | BATTERY_VOLTAGE_2; else 
    if(voltage > 3.30) battery_mask = battery_mask | BATTERY_VOLTAGE_1;
}

void turn_off() {
    delay_ms(500);
    // TODO: Where did these come from? JTAG?
    gpio_pullup_dis(14);
    gpio_pullup_dis(15);
    esp_sleep_enable_ext1_wakeup((1ULL<<BUTTON_WAKEUP),ESP_EXT1_WAKEUP_ALL_LOW);
    esp_deep_sleep_start();
}
void app_main() {
    const esp_app_desc_t * app_desc = esp_ota_get_app_description();
    // version comes back as version "v0.1-alpha-259-g371d500-dirty"
    // the v0.1-alpha seems hardcoded, setting cmake PROJECT_VER replaces the more useful git describe line
    // so we'll have to parse the commit ID out
    // or maybe just get date time as YYYYMMDDHHMMSS? 

    if(strlen(app_desc->version) > 20) {
        if(app_desc->version[strlen(app_desc->version)-1] == 'y') {
            strncpy(githash, app_desc->version + strlen(app_desc->version)-13, 7);
        } else {
            strncpy(githash, app_desc->version + strlen(app_desc->version)-7, 7);            
        }
        githash[7] = 0;
    }
    printf("Welcome to %s -- date %s time %s version %s [%s]\n", app_desc->project_name, app_desc->date, app_desc->time, app_desc->version, githash);

    esp_event_loop_create_default();
    power_init();
    if(board_level == ALLES_BOARD_V2) {
        printf("Detected revB+ Alles\n");
        TimerHandle_t power_monitor_timer = xTimerCreate(
            "power_monitor",
            pdMS_TO_TICKS(5000),
            pdTRUE,
            NULL,
            power_monitor);
        xTimerStart(power_monitor_timer, 0);

    }

    // TODO: one of these interferes with the power monitor, not a big deal, we don't use both at once
    // Setup GPIO outputs for watching CPU usage on an oscilloscope 
    /*
    const gpio_config_t out_conf = {
         .mode = GPIO_MODE_OUTPUT,            
         .pin_bit_mask = (1ULL<<CPU_MONITOR_0) | (1ULL<<CPU_MONITOR_1) | (1ULL<<CPU_MONITOR_2),
    };
    gpio_config(&out_conf); 

    // Set them all to low
    gpio_set_level(CPU_MONITOR_0, 0); // use 0 as ground for the scope 
    gpio_set_level(CPU_MONITOR_1, 0); // use 1 for the rendering loop 
    gpio_set_level(CPU_MONITOR_2, 0); // use 2 for whatever you want 
    */
    buttons_init();

    // Spin this core until the power off button is pressed, parsing events and making sounds
    while(status & RUNNING) {
        delay_ms(10);
    }

    // If we got here, the power off button was pressed 
    turn_off();
}

