/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "input_iot.h"
#include "output_iot.h"

#define BLINK_GPIO CONFIG_BLINK_GPIO

void input_event_callback(int pin) {
    if(pin == GPIO_NUM_0) {
        output_io_toggle(BLINK_GPIO);
    }
}

void app_main(void)
{
    output_io_create(BLINK_GPIO);
    input_io_create(GPIO_NUM_0, HI_TO_LO);
    input_set_callback(input_event_callback);
}
