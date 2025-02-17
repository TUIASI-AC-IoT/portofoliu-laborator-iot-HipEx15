#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#define GPIO_OUTPUT_IO 4
#define GPIO_INPUT_IO 2

#define GPIO_OUTPUT_PIN_SEL (1ULL<<GPIO_OUTPUT_IO)
#define GPIO_INPUT_PIN_SEL (1ULL<<GPIO_INPUT_IO)

static int button_pressed = 0;

static void gpio_button_task(void* arg)
{
    static int last_button_state = 0;
    int button_state;

    for (;;) {
        button_state = gpio_get_level(GPIO_INPUT_IO);

        if (button_state == 1 && last_button_state == 0) {
            button_pressed++;
            printf("Button is pressed ! Value is: %d\n", button_pressed);
        }

        last_button_state = button_state;
        vTaskDelay( 100 / portTICK_PERIOD_MS ) ;
    }
}

void app_main() 
{
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};

    //GPIO4
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //GPIO2
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //disable pull-down mode
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //start gpio task
    xTaskCreate(gpio_button_task, "gpio_button_task", 2048, NULL, 10, NULL);

    int cnt = 0;

    while(1) 
    {
        printf("cnt: %d\n", cnt++);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_OUTPUT_IO, 1);

        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_OUTPUT_IO, 0);

        vTaskDelay(250 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_OUTPUT_IO, 1);

        vTaskDelay(750 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_OUTPUT_IO, 0);
    }
}