#include <stdio.h>
#include "pico/stdlib.h"
#include "incbin.h"

#define INCBIN_PREFIX g_
#define INCBIN_STYLE INCBIN_STYLE_SNAKE

#define CLOCK 17
#define RW_PIN 0

// Masks for the address and data pins
const uint32_t ADDR_MASK = 0b00000000000000011111111111111110;
const uint32_t DATA_MASK = 0b00000011111111000000000000000000;
const uint32_t CLOCK_MASK = 0b00000000000000100000000000000000;
const uint32_t RW_MASK = 0b00000000000000000000000000000001;

const char ADDR_PINS[] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1}; // MSB to LSB
// const char DATA_PINS[] = {18, 19, 20, 21, 22, 23, 24, 25};
const char DATA_PINS[] = {25, 24, 23, 22, 21, 20, 19, 18}; // MSB to LSB

// #define ROM_SIZE 2 ^ 15 // hex 8000
// #define ROM_START 0x8000
#define ROM_START 0x8000
INCBIN(rom, "rom.bin");
#define ROM_SIZE g_rom_size
#define rom g_rom_data

// #define RAM_SIZE 2 ^ 15 // hex 8000
// #define RAM_START 0x0000
#define RAM_SIZE 0x8000
#define RAM_START 0x0000
static uint8_t ram[RAM_SIZE];

uint16_t address = 0;
uint8_t data = 0;
bool rw = false;

uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void read()
{
    // Address is only for printing
    gpio_set_dir_out_masked(DATA_MASK);
    // gpio_put_masked(DATA_MASK, reverse(data) << 17);
    // gpio_put_masked(DATA_MASK, data << 17);
    for (int i = 0; i < 8; i++)
    {
        gpio_put(DATA_PINS[i], (data >> i) & 1);
    }
    
}

void print_read()
{
    if (address >= ROM_START && address <= ROM_START + ROM_SIZE)
        printf("ROM ");
    else if (address >= RAM_START && address <= RAM_START + RAM_SIZE)
        printf("RAM ");
    else
        printf("INVALID ");
    printf("R | Addr: %016b - %04x | Data: %08b - %02x\n", address, address, data, data);
}

void write()
{
    // Address is only for printing
    gpio_set_dir_in_masked(DATA_MASK);
    // uint8_t data = (gpio_get_all() && DATA_MASK) >> 17;
    uint8_t data = 0;
    for (int i = 0; i < 8; i++)
    {
        data = data << 1 | gpio_get(DATA_PINS[i]);
    }
}

void print_write()
{
    if (address >= ROM_START && address <= ROM_START + ROM_SIZE)
        printf("ROM ");
    else if (address >= RAM_START && address <= RAM_START + RAM_SIZE)
        printf("RAM ");
    else
        printf("INVALID ");
    printf("W | Addr: %016b - %04x | Data: %08b - %02x\n", address, address, data, data);
}

void evaluate_ram()
{

    // Read the address bus
    address = 0;
    for (int i = 0; i < 16; i++)
    {
        address = address << 1 | gpio_get(ADDR_PINS[i]);
    }

    rw = gpio_get(RW_PIN); // High is read, low is write

    // Read or write to the ROM or RAM
    if (address >= ROM_START && address < ROM_START + ROM_SIZE)
    // Inside ROM
    {
        if (rw) // Read the ROM
        {
            data = rom[address - ROM_START];
            read();
        }
        else // Write to the ROM
        {
            write(); // Discard the data because we can't write to ROM
        }
    }
    else if (address >= RAM_START && address < RAM_START + RAM_SIZE)
    // Inside RAM
    {
        if (rw) // Read the RAM
        {
            data = ram[address - RAM_START];
            read();
        }
        else // Write to the RAM
        {
            write(address);
            ram[address - RAM_START] = data;
        }
    }
    else // Inside invalid memory
    {
        if (rw) // Read the invalid memory
        {
            data = 0;
            read(address, 0);
        }
        else // Write to the invalid memory
            write(address);
    }
}

void print_to_serial(uint gpio, uint32_t events)
{
    rw = gpio_get(RW_PIN); // High is read, low is write

    if (rw)
        print_read();
    else
        print_write();
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

    gpio_init_mask(ADDR_MASK || DATA_MASK);
    gpio_set_dir_in_masked(ADDR_MASK || DATA_MASK);

    // Set RW pin to output
    gpio_init(RW_PIN);
    gpio_set_dir(RW_PIN, GPIO_OUT);

    // Set interrupt on rising edge of the clock
    gpio_init(CLOCK);
    gpio_set_dir(CLOCK, GPIO_IN);

    // Begin serial at 115200 baud
    stdio_init_all();

    gpio_set_irq_enabled_with_callback(CLOCK, GPIO_IRQ_EDGE_RISE, true, &print_to_serial);

    while (1)
    {
        evaluate_ram();
        sleep_us(50);
    }

    return 0;
}
