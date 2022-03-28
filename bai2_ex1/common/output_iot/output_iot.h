#ifndef OUTPUT_IOT_H
#define OUTPUT_IOT_H
#include <esp_log.h>
#include <hal/gpio_types.h>

void output_io_create(gpio_num_t gpio_num);
void output_io_set_level(gpio_num_t gpio_num, int level);
void output_io_toggle(gpio_num_t gpio_num);

#endif