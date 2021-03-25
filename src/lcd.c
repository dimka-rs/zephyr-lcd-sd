#include <zephyr.h>
#include <drivers/gpio.h>

#define DATA_WIDTH 8
typedef struct
{
    int data[DATA_WIDTH];
    int rst;
    int cs;
    int rs;
    int wr;
    int rd;
} lcd_t;

lcd_t lcd0 = {
    .data = {19, 20, 13, 14, 15, 16, 17, 18},
    .rst = 30,
    .cs  = 29,
    .rs = 28,
    .wr = 4,
    .rd = 3,
};

static const struct device *gpio0;

static int
set_data_pins(gpio_flags_t flags)
{
    for (int i = 0; i < DATA_WIDTH; i++)
    {
        int ret = gpio_pin_configure(gpio0, lcd0.data[i], flags);
        if (ret < 0)
        {
            printk("Failed to configure pin %d: %d\n", lcd0.data[i], ret);
            return ret;
        }
    }

    return 0;
}

static void
write_data(uint8_t data)
{
    set_data_pins(GPIO_OUTPUT);

    // set high byte
    for (int i = 0; i < DATA_WIDTH; i++)
    {
        gpio_pin_set(gpio0, lcd0.data[i], data & (1 << i));
    }

    // strobe
    gpio_pin_set(gpio0, lcd0.wr, 0);
    k_sleep(K_MSEC(10));
    gpio_pin_set(gpio0, lcd0.wr, 1);
}

static uint8_t
read_data(void)
{
    uint8_t data = 0;

    set_data_pins(GPIO_INPUT);

    // strobe
    gpio_pin_set(gpio0, lcd0.rd, 0);
    k_sleep(K_MSEC(10));
    // get high byte
    for (int i = 0; i < DATA_WIDTH; i++)
    {
        if (gpio_pin_get(gpio0, lcd0.data[i]))
        {
            data |= (1 << i);
        }
    }
    gpio_pin_set(gpio0, lcd0.rd, 1);

    return data;
}

static uint16_t
lcd_read_id(void)
{
    uint16_t id;

    gpio_pin_set(gpio0, lcd0.cs, 0);

    gpio_pin_set(gpio0, lcd0.rs, 0); // idx reg/int status
    write_data(0); // set id register address
    write_data(0); // set id register address
    //gpio_pin_set(gpio0, lcd0.rs, 1); // maybe change read type?
    id = read_data();
    id <<= 8;
    id |= read_data();

    gpio_pin_set(gpio0, lcd0.cs, 1);
    return id;
}

int
lcd_init(void)
{
    gpio0 = device_get_binding("GPIO_0");
    if (gpio0 == NULL)
    {
        printk("Failed to get GPIO_0\n");
        return -1;
    }

    set_data_pins(GPIO_OUTPUT_HIGH);
    gpio_pin_configure(gpio0, lcd0.cs, GPIO_OUTPUT_HIGH);
    gpio_pin_configure(gpio0, lcd0.rd, GPIO_OUTPUT_HIGH);
    gpio_pin_configure(gpio0, lcd0.rs, GPIO_OUTPUT_HIGH);
    gpio_pin_configure(gpio0, lcd0.wr, GPIO_OUTPUT_HIGH);
    gpio_pin_configure(gpio0, lcd0.rst, GPIO_OUTPUT_LOW);

    // reset
    k_sleep(K_MSEC(100));
    gpio_pin_set(gpio0, lcd0.rst, 1);

    uint16_t id = lcd_read_id();
    //FIXME: Is it compatible with ili9341?
    if (id == 0x5408)
    {
        printk("LCD 5408 init OK\n");
        return 0;
    }

    printk("LCD ID is wrong: %04X\n", id);
    return -1;
}