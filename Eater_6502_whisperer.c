#include <stdio.h>
#include "pico/stdlib.h"

#define CLOCK 17
#define RW_PIN 0

const char ADDR_PINS[] = {16, 15, 14, 13, 12,11,10,9,8,7,6,5,4,3,2,1}; // MSB to LSB
const char DATA_PINS[] = {18, 19, 20, 21, 22, 23, 24, 25};
const char OTHER_PINS[] = {26, 26, 27, 28, 29};

void clock_isr(uint gpio, uint32_t events)
{
    char output[30];

    
    if (gpio_get(RW_PIN)){
        printf("R | ");
    } else {
        printf("W | ");
    }

    printf("Addr: ");
    unsigned int addr = 0;
    for (int i = 0; i < 16; i++)
    {
        bool status = gpio_get(ADDR_PINS[i]);
        printf("%d", status);
        addr = addr << 1 | status;
    }
    printf(" - %04x", addr);

    printf(" | Data: ");
    unsigned int data = 0;
    for (int i = 0; i < 8; i++)
    {
        bool status = gpio_get(DATA_PINS[i]);
        printf("%d", status);
        data = data << 1 | status;
    }
    printf(" - %02x", data);

    printf(" | Other: ");
    for (int i = 0; i < 5; i++)
    {
        printf("%d", gpio_get(OTHER_PINS[i]));
    }
    putchar('\n');
}

int main()
{
    // Set all pins to input
    for (int i = 0; i < 16; i++)
    {
        gpio_init(ADDR_PINS[i]);
        gpio_set_dir(ADDR_PINS[i], GPIO_IN);
    }
    for (int i = 0; i < 8; i++)
    {
        gpio_init(DATA_PINS[i]);
        gpio_set_dir(DATA_PINS[i], GPIO_IN);
    }
    for (int i = 0; i < 5; i++)
    {
        gpio_init(OTHER_PINS[i]);
        gpio_set_dir(OTHER_PINS[i], GPIO_IN);
    }

    // Set RW pin to output
    gpio_init(RW_PIN);
    gpio_set_dir(RW_PIN, GPIO_OUT);

    // Set interrupt on rising edge of the clock
    gpio_init(CLOCK);
    gpio_set_dir(CLOCK, GPIO_IN);
    gpio_set_irq_enabled_with_callback(CLOCK, GPIO_IRQ_EDGE_RISE, true, &clock_isr);

    // Begin serial at 115200 baud
    stdio_init_all();

    // while (1)
    // {
    // for (int i = ADDR_PIN_MIN; i <= ADDR_PIN_MAX; i++)
    //     {
    //         printf("%d", gpio_get(i));
    //     }
    //     putchar('\n');
    // }

    while (1)
    {
        tight_loop_contents();
    }

    return 0;
}
