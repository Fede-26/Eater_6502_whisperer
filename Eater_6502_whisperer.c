#include <stdio.h>
#include "pico/stdlib.h"

#define CLOCK 17
#define RW_PIN 0

// Masks for the address and data pins
const uint32_t ADDR_MASK =  0b00000000000000011111111111111110;
const uint32_t DATA_MASK =  0b00000011111111000000000000000000;
const uint32_t OTHER_MASK = 0b00111100000000000000000000000000;
const uint32_t CLOCK_MASK = 0b00000000000000100000000000000000;
const uint32_t RW_MASK =    0b00000000000000000000000000000001;

const char ADDR_PINS[] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1}; // MSB to LSB
const char DATA_PINS[] = {18, 19, 20, 21, 22, 23, 24, 25};
const char OTHER_PINS[] = {26, 27, 28, 29};

// #define ROM_SIZE 2 ^ 15 // hex 8000
// #define ROM_START 0x8000
#define ROM_SIZE 10
#define ROM_START 11
static uint8_t rom[ROM_SIZE];

// #define RAM_SIZE 2 ^ 15 // hex 8000
// #define RAM_START 0x0000
#define RAM_SIZE 10
#define RAM_START 0x0000
static uint8_t ram[RAM_SIZE];


void print_pins()
{
    char output[30];

    if (gpio_get(RW_PIN))
    {
        printf("R | ");
    }
    else
    {
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

void clock_isr(uint gpio, uint32_t events)
{

    // Read the address bus
    uint16_t address = 0;
    for (int i = 0; i < 16; i++)
    {
        address = address << 1 | gpio_get(ADDR_PINS[i]);
    }


    // Read or write to the ROM or RAM
    if (address >= ROM_START && address < ROM_START + ROM_SIZE)
    {
        if (gpio_get(RW_PIN))
        {
            // Read
            printf("ROM ");
            uint8_t data = rom[address - ROM_START];
            // Set pins to output
            gpio_set_dir_out_masked(DATA_MASK);
            gpio_put_masked(DATA_MASK, data << 17);
        }
        else
        {
            // Write
            printf("ROM ");
            uint8_t data = 0;
            gpio_set_dir_in_masked(DATA_MASK);
            data = (gpio_get_all() && DATA_MASK) >> 17;
            rom[address - ROM_START] = data;
        }
    }
    else if (address >= RAM_START && address < RAM_START + RAM_SIZE)
    {
        if (gpio_get(RW_PIN))
        {
            // Read
            printf("RAM ");
            uint8_t data = ram[address - RAM_START];
            gpio_set_dir_out_masked(DATA_MASK);
            gpio_put_masked(DATA_MASK, data << 17);
        }
        else
        {
            // Write
            printf("RAM ");
            uint8_t data = 0;
            gpio_set_dir_in_masked(DATA_MASK);
            data = (gpio_get_all() && DATA_MASK) >> 17;
            ram[address - RAM_START] = data;
        }
    }
    
    print_pins();
}


int main()
{
    // Set all pins to input
    // for (int i = 0; i < 16; i++)
    // {
    //     gpio_init(ADDR_PINS[i]);
    //     gpio_set_dir(ADDR_PINS[i], GPIO_IN);
    // }
    // for (int i = 0; i < 8; i++)
    // {
    //     gpio_init(DATA_PINS[i]);
    //     gpio_set_dir(DATA_PINS[i], GPIO_IN);
    // }
    // for (int i = 0; i < 5; i++)
    // {
    //     gpio_init(OTHER_PINS[i]);
    //     gpio_set_dir(OTHER_PINS[i], GPIO_IN);
    // }

    // Set all pins to input
    gpio_init_mask(ADDR_MASK && DATA_MASK && OTHER_MASK );
    gpio_set_dir_in_masked(ADDR_MASK && DATA_MASK && OTHER_MASK);

    // Set RW pin to output
    gpio_init(RW_PIN);
    gpio_set_dir(RW_PIN, GPIO_OUT);

    // Set interrupt on rising edge of the clock
    gpio_init(CLOCK);
    gpio_set_dir(CLOCK, GPIO_IN);

    // Begin serial at 115200 baud
    stdio_init_all();

    // Read the ROM via serial
    for (int i = 0; i < ROM_SIZE; i++)
    {
        rom[i] = getchar();
    }

    sleep_ms(1000);

    gpio_set_irq_enabled_with_callback(CLOCK, GPIO_IRQ_EDGE_RISE, true, &clock_isr);

    while (1)
    {
        tight_loop_contents();
    }

    return 0;
}
