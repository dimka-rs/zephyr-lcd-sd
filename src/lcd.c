#include <zephyr.h>
#include <drivers/gpio.h>
#include <logging/log.h>

#define LOG_MODULE_NAME lcd
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define DATA_WIDTH 8

#define CS_ACTIVE  gpio_pin_set(gpio0, lcd0.cs, 0)
#define CS_IDLE    gpio_pin_set(gpio0, lcd0.cs, 1)
#define RD_ACTIVE  gpio_pin_set(gpio0, lcd0.rd, 0)
#define RD_IDLE    gpio_pin_set(gpio0, lcd0.rd, 1)
#define WR_ACTIVE  gpio_pin_set(gpio0, lcd0.wr, 0)
#define WR_IDLE    gpio_pin_set(gpio0, lcd0.wr, 1)
#define RS_COMMAND gpio_pin_set(gpio0, lcd0.rs, 0)
#define RS_DATA    gpio_pin_set(gpio0, lcd0.rs, 1)
#define RST_ACTIVE gpio_pin_set(gpio0, lcd0.rst, 0)
#define RST_IDLE   gpio_pin_set(gpio0, lcd0.rst, 1)

#define DATA_INPUT  set_data_pins(GPIO_INPUT)
#define DATA_OUTPUT set_data_pins(GPIO_OUTPUT)

typedef struct
{
    gpio_pin_t data[DATA_WIDTH];
    gpio_pin_t rst;
    gpio_pin_t cs;
    gpio_pin_t rs;
    gpio_pin_t wr;
    gpio_pin_t rd;
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
    // set high byte
    for (int i = 0; i < DATA_WIDTH; i++)
    {
        gpio_pin_set(gpio0, lcd0.data[i], data & (1 << i));
    }

    WR_ACTIVE;
    k_sleep(K_MSEC(1));
    WR_IDLE;
    k_sleep(K_MSEC(1));
}

static uint8_t
read_data(void)
{
    uint8_t data = 0;

    RD_ACTIVE;
    k_sleep(K_MSEC(1));
    for (int i = 0; i < DATA_WIDTH; i++)
    {
        if (gpio_pin_get(gpio0, lcd0.data[i]))
        {
            data |= (1 << i);
        }
    }
    RD_IDLE;
    k_sleep(K_MSEC(1));

    return data;
}

static uint16_t
lcd_read_id(void)
{
    uint16_t id;

    RS_COMMAND;
    DATA_OUTPUT;
    CS_ACTIVE;
    write_data(0); // 0x00
    write_data(0); // Driver Code register address
    CS_IDLE;

    RS_DATA;
    DATA_INPUT;
    CS_ACTIVE;
    id = read_data();
    id = id << 8;
    id = id | read_data();
    CS_IDLE;

    printk("id: 0x%04x\n", id);
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

    gpio_pin_configure(gpio0, lcd0.cs,  GPIO_OUTPUT_HIGH);
    gpio_pin_configure(gpio0, lcd0.rs,  GPIO_OUTPUT_HIGH);
    gpio_pin_configure(gpio0, lcd0.rd,  GPIO_OUTPUT_HIGH);
    gpio_pin_configure(gpio0, lcd0.wr,  GPIO_OUTPUT_HIGH);
    gpio_pin_configure(gpio0, lcd0.rst, GPIO_OUTPUT_HIGH);
    set_data_pins(GPIO_OUTPUT_HIGH);

    // reset
    RST_ACTIVE;
    k_sleep(K_MSEC(10));
    RST_IDLE;

    uint16_t id = lcd_read_id();
    //My shield uses ILI9325
    if (id == 0x9325)
    {
        printk("LCD 0x%04X init OK\n", id);
        return 0;
    }

    printk("LCD ID is wrong: 0x%04X\n", id);
    return -1;
}