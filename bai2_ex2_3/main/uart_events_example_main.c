/* UART Events Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/timers.h"
#include "uart_iot.h"
#include "output_iot.h"

#define SHELL_CHANGE_PERIOR_STR_FULL         "period="
#define SHELL_CHANGE_PERIOR_STR         "period"

extern const char *TAG;

/**
 * This example shows how to use the UART driver to handle special UART events.
 *
 * It also reads data from UART0 directly, and echoes it to console.
 *
 * - Port: UART0
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: on
 * - Pin assignment: TxD (default), RxD (default)
 */

extern QueueHandle_t uart0_queue;
TimerHandle_t xTimers;

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
}

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(uart0_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            bzero(dtmp, RD_BUF_SIZE);
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                    uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                    char *temp = (char *)malloc(event.size + 1);
                    strcpy(temp, (char*) dtmp);
                    if(strstr((char*) temp, SHELL_CHANGE_PERIOR_STR_FULL)) {
                        // Extract the first token
                        char *token = strtok((char*) temp, "=");
                        // loop through the string to extract all other tokens
                        while (token != NULL)
                        {
                            if(!strstr((char*) token, SHELL_CHANGE_PERIOR_STR)) {
                                int x = atoi(token);
                                if (x > 0) {
                                    if (xTimerChangePeriod(xTimers, x / portTICK_PERIOD_MS, 500) == pdPASS) {
                                        /* The command was successfully sent. */
                                        ESP_LOGI(TAG, "Change Timer period successfully");
                                    }
                                    else {
                                        /* The command could not be sent, even after waiting for 100 ticks
                                        to pass.  Take appropriate action here. */
                                        ESP_LOGI(TAG, "Change Timer period failed");
                                    }
                                } else {
                                    ESP_LOGI(TAG, "Period is negative or zero");
                                }
                            }
                            token = strtok(NULL, " ");
                        }
                    }
                    uart_write_bytes(EX_UART_NUM, (const char*) dtmp, event.size);
                    free(temp);
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    ESP_LOGI(TAG, "uart rx break");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:
                    uart_get_buffered_data_len(EX_UART_NUM, &buffered_size);
                    int pos = uart_pattern_pop_pos(EX_UART_NUM);
                    ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                    if (pos == -1) {
                        // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                        // record the position. We should set a larger queue size.
                        // As an example, we directly flush the rx buffer here.
                        uart_flush_input(EX_UART_NUM);
                    } else {
                        uart_read_bytes(EX_UART_NUM, dtmp, pos, 100 / portTICK_PERIOD_MS);
                        uint8_t pat[PATTERN_CHR_NUM + 1];
                        memset(pat, 0, sizeof(pat));
                        uart_read_bytes(EX_UART_NUM, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                        ESP_LOGI(TAG, "read data: %s", dtmp);
                        ESP_LOGI(TAG, "read pat : %s", pat);
                    }
                    break;
                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

        output_io_create(2);

    /* Create then start some timers.  Starting the timers before
    the RTOS scheduler has been started means the timers will start
    running immediately that the RTOS scheduler starts. */

    xTimers = xTimerCreate(/* Just a text name, not used by the RTOS
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

    if (xTimers == NULL)
    {
        /* The timer was not created. */
    }
    else
    {
        /* Start the timer.  No block time is specified, and
        even if one was it would be ignored because the RTOS
        scheduler has not yet been started. */
        if (xTimerStart(xTimers, 0) != pdPASS)
        {
            /* The timer could not be set into the Active
            state. */
        }
    }

    uart_set_callback(uart_event_task);
    uart_create(EX_UART_NUM);
}
