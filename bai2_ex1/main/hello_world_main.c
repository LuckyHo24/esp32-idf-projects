/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "input_iot.h"
#include "output_iot.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "hal/gpio_types.h"

#define NUM_TIMERS 2

#define BIT_EVENT_SHORT_PRESS (1 << 0)
#define BIT_EVENT_NORMAL_PRESS (1 << 1)
#define BIT_EVENT_LONG_PRESS (1 << 2)

/* An array to hold handles to the created timers. */
TimerHandle_t xTimers[NUM_TIMERS];

/* Declare a variable to hold the created event group. */
static EventGroupHandle_t xCreatedEventGroup;
static uint64_t __start, __stop, __press_tick;

void button_callback(int pin)
{
    EventBits_t uxBits;
    BaseType_t xHigherPriorityTaskWoken;
    uint64_t rtc = xTaskGetTickCountFromISR();
    if (pin == GPIO_NUM_0)
    {
        if(input_io_get_level(pin) == 0) {
            xTimerStartFromISR(xTimers[0],
                               &xHigherPriorityTaskWoken);
            __start = rtc;
        } else {
            xTimerStopFromISR( xTimers[0], &xHigherPriorityTaskWoken );
            __stop = rtc;
            __press_tick = __stop - __start;

            uint64_t press_time_ms = __press_tick * portTICK_PERIOD_MS;
            if(press_time_ms <= 1000 && press_time_ms > 0)  {
                //short press
                /* Set bit 0 in xEventGroup. */
                uxBits = xEventGroupSetBitsFromISR(
                    xCreatedEventGroup,                                /* The event group being updated. */
                    BIT_EVENT_SHORT_PRESS, &xHigherPriorityTaskWoken); /* The bits being set. */
            } else if(press_time_ms <= 3000 && press_time_ms > 1000)
            {
                //normal press
                /* Set bit 1 in xEventGroup. */
                uxBits = xEventGroupSetBitsFromISR(
                    xCreatedEventGroup,                                /* The event group being updated. */
                    BIT_EVENT_NORMAL_PRESS, &xHigherPriorityTaskWoken); /* The bits being set. */
            } else if(press_time_ms <= 5000 && press_time_ms > 3000)
            {
                //long press
                /* Set bit 2 in xEventGroup. */
                uxBits = xEventGroupSetBitsFromISR(
                    xCreatedEventGroup,                                /* The event group being updated. */
                    BIT_EVENT_LONG_PRESS, &xHigherPriorityTaskWoken); /* The bits being set. */
            }
        }
    }
}

void vTimerCallback(TimerHandle_t xTimer)
{
    /* Optionally do something if the pxTimer parameter is NULL. */
    configASSERT(xTimer);

    /* The number of times this timer has expired is saved as the
    timer's ID.  Obtain the count. */
    uint32_t ulCount = (uint32_t)pvTimerGetTimerID(xTimer);
    if (ulCount == 0)
    {
        printf("Timeout\n");
    }
}

/* Task to be created. */
void vTaskButtonHandle(void *pvParameters)
{
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. */
    configASSERT(((uint32_t)pvParameters) == 1);

    for (;;)
    {
        EventBits_t uxBits = xEventGroupWaitBits(
            xCreatedEventGroup,   /* The event group being tested. */
            BIT_EVENT_SHORT_PRESS | BIT_EVENT_NORMAL_PRESS | BIT_EVENT_LONG_PRESS, /* The bits within the event group to wait for. */
            pdTRUE,        /* BIT_0 & BIT_4 should be cleared before returning. */
            pdFALSE,       /* Don't wait for both bits, either bit will do. */
            portMAX_DELAY); /* Wait a maximum of 100ms for either bit to be set. */
        if(uxBits & BIT_EVENT_SHORT_PRESS) {
            printf("Short press\n");
        }
        if(uxBits & BIT_EVENT_NORMAL_PRESS) {
            printf("Normal press\n");
        } 
        if(uxBits & BIT_EVENT_LONG_PRESS) {
            printf("Long press\n");
        }
    }
}

void app_main(void)
{
    printf("Hello world!\n");

    BaseType_t xReturned;

    /* Attempt to create the event group. */
    xCreatedEventGroup = xEventGroupCreate();
    /* Was the event group created successfully? */
    if (xCreatedEventGroup == NULL)
    {
        /* The event group was not created because there was insufficient
        FreeRTOS heap available. */
    }
    else
    {
        /* The event group was created. */
        printf("xCreatedEventGroup is created\n");
    }

    output_io_create(2);
    input_io_create(0, GPIO_INTR_ANYEDGE);
    input_set_callback(button_callback);

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
        vTaskButtonHandle,     /* Function that implements the task. */
        "vTaskButtonHandle",   /* Text name for the task. */
        2000,       /* Stack size in words, not bytes. */
        (void *)1,  /* Parameter passed into the task. */
        4,          /* Priority at which the task is created. */
        NULL); /* Used to pass out the created task's handle. */

    if (xReturned == pdPASS)
    {
    }

    /* Create then start some timers.  Starting the timers before
    the RTOS scheduler has been started means the timers will start
    running immediately that the RTOS scheduler starts. */

    xTimers[0] = xTimerCreate(/* Just a text name, not used by the RTOS
                              kernel. */
                              "TimerBlink",
                              /* The timer period in ticks, must be
                              greater than 0. */
                              pdMS_TO_TICKS(7000),
                              /* The timers will auto-reload themselves
                              when they expire. */
                              pdFAIL,
                              /* The ID is used to store a count of the
                              number of times the timer has expired, which
                              is initialised to 0. */
                              (void *)0,
                              /* Each timer calls the same callback when
                              it expires. */
                              vTimerCallback);

    if (xTimers[0] == NULL)
    {
        /* The timer was not created. */
    }
    else
    {
    }
}
