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
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "input_iot.h"
#include "output_iot.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#define NUM_TIMERS 2

#define BIT_EVENT_BUTTON_PRESS (1 << 0)
#define BIT_EVENT_UART_RECV (1 << 1)

/* Declare a variable to hold the created event group. */
EventGroupHandle_t xCreatedEventGroup;

/* An array to hold handles to the created timers. */
TimerHandle_t xTimers[NUM_TIMERS];

/* Task to be created. */
void vTask1(void *pvParameters)
{
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. */
    configASSERT(((uint32_t)pvParameters) == 1);

    for (;;)
    {
        /* Task code goes here. */
        printf("Task 1\n");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

/* Task to be created. */
void vTask2(void *pvParameters)
{
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. */
    configASSERT(((uint32_t)pvParameters) == 1);

    for (;;)
    {
        /* Task code goes here. */
        printf("Task 2\n");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

/* Task to be created. */
void vTask3(void *pvParameters)
{
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. */
    configASSERT(((uint32_t)pvParameters) == 1);

    for (;;)
    {
        /* Task code goes here. */
        printf("Task 3\n");
        vTaskDelay(1000 / portTICK_RATE_MS);
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
        printf("Blink LED\n");
        output_io_toggle(2);
    }
    else
    {
        printf("Print Uart\n");
    }
}

void button_callback(int pin)
{
    if (pin == GPIO_NUM_0)
    {
        EventBits_t uxBits;
        BaseType_t xHigherPriorityTaskWoken;

        /* Set bit 0 and bit 4 in xEventGroup. */
        uxBits = xEventGroupSetBitsFromISR(
            xCreatedEventGroup,                            /* The event group being updated. */
            BIT_EVENT_BUTTON_PRESS | BIT_EVENT_UART_RECV, &xHigherPriorityTaskWoken); /* The bits being set. */
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
            BIT_EVENT_BUTTON_PRESS | BIT_EVENT_UART_RECV, /* The bits within the event group to wait for. */
            pdTRUE,        /* BIT_0 & BIT_4 should be cleared before returning. */
            pdFALSE,       /* Don't wait for both bits, either bit will do. */
            portMAX_DELAY); /* Wait a maximum of 100ms for either bit to be set. */
        if(uxBits & BIT_EVENT_BUTTON_PRESS) {
            printf("BUTTON PRESS\n");
            output_io_toggle(2);
        }
        if(uxBits & BIT_EVENT_UART_RECV) {
            printf("UART DATA\n");
        }
    }
}

void app_main(void)
{
    printf("Hello world!\n");

    BaseType_t xReturned;
    TaskHandle_t xHandle1 = NULL;
    TaskHandle_t xHandle2 = NULL;
    TaskHandle_t xHandle3 = NULL;

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

    /* Create then start some timers.  Starting the timers before
    the RTOS scheduler has been started means the timers will start
    running immediately that the RTOS scheduler starts. */

    xTimers[0] = xTimerCreate(/* Just a text name, not used by the RTOS
                              kernel. */
                              "TimerBlink",
                              /* The timer period in ticks, must be
                              greater than 0. */
                              pdMS_TO_TICKS(500),
                              /* The timers will auto-reload themselves
                              when they expire. */
                              pdTRUE,
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
        /* Start the timer.  No block time is specified, and
        even if one was it would be ignored because the RTOS
        scheduler has not yet been started. */
        if (xTimerStart(xTimers[0], 0) != pdPASS)
        {
            /* The timer could not be set into the Active
            state. */
        }
    }

    xTimers[1] = xTimerCreate(/* Just a text name, not used by the RTOS
                          kernel. */
                              "TimerPrint",
                              /* The timer period in ticks, must be
                              greater than 0. */
                              pdMS_TO_TICKS(1000),
                              /* The timers will auto-reload themselves
                              when they expire. */
                              pdTRUE,
                              /* The ID is used to store a count of the
                              number of times the timer has expired, which
                              is initialised to 0. */
                              (void *)1,
                              /* Each timer calls the same callback when
                              it expires. */
                              vTimerCallback);

    if (xTimers[1] == NULL)
    {
        /* The timer was not created. */
    }
    else
    {
        /* Start the timer.  No block time is specified, and
        even if one was it would be ignored because the RTOS
        scheduler has not yet been started. */
        if (xTimerStart(xTimers[1], 0) != pdPASS)
        {
            /* The timer could not be set into the Active
            state. */
        }
    }

    output_io_create(2);
    input_io_create(0, HI_TO_LO);
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
    else
    {
        /* The task was created.  Use the task's handle to delete the task. */
        // vTaskDelete(xHandle1);
    }

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
        vTask1,     /* Function that implements the task. */
        "vTask1",   /* Text name for the task. */
        1024,       /* Stack size in words, not bytes. */
        (void *)1,  /* Parameter passed into the task. */
        4,          /* Priority at which the task is created. */
        &xHandle1); /* Used to pass out the created task's handle. */

    if (xReturned == pdPASS)
    {
    }
    else
    {
        /* The task was created.  Use the task's handle to delete the task. */
        vTaskDelete(xHandle1);
    }

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
        vTask2,     /* Function that implements the task. */
        "vTask2",   /* Text name for the task. */
        1024,       /* Stack size in words, not bytes. */
        (void *)1,  /* Parameter passed into the task. */
        4,          /* Priority at which the task is created. */
        &xHandle2); /* Used to pass out the created task's handle. */

    if (xReturned == pdPASS)
    {
    }
    else
    {
        /* The task was created.  Use the task's handle to delete the task. */
        vTaskDelete(xHandle2);
    }

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
        vTask3,     /* Function that implements the task. */
        "vTask3",   /* Text name for the task. */
        1024,       /* Stack size in words, not bytes. */
        (void *)1,  /* Parameter passed into the task. */
        4,          /* Priority at which the task is created. */
        &xHandle3); /* Used to pass out the created task's handle. */

    if (xReturned == pdPASS)
    {
    }
    else
    {
        /* The task was created.  Use the task's handle to delete the task. */
        vTaskDelete(xHandle3);
    }
}
