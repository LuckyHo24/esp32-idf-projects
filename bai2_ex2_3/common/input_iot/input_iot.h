#ifndef INPUT_IOT_H
#define INPUT_IOT_H
#include <esp_log.h>
#include <hal/gpio_types.h>

typedef enum {
    HI_TO_LO = 0,
    LO_TO_HI,
    ANY_EDGE
} interrupt_type_edge_t;

typedef void (*input_callback_t) (int);

void input_io_create(gpio_num_t gpio_num, interrupt_type_edge_t type);
void input_io_get_level(gpio_num_t gpio_num);
void input_set_callback(void *cb);

#endif