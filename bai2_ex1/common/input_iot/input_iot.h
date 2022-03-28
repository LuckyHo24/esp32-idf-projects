#ifndef INPUT_IOT_H
#define INPUT_IOT_H
#include <esp_log.h>
#include <hal/gpio_types.h>

typedef void (*input_callback_t) (int);

void input_io_create(gpio_num_t gpio_num, gpio_int_type_t type);
int input_io_get_level(gpio_num_t gpio_num);
void input_set_callback(void *cb);

#endif