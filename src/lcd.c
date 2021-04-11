#include <zephyr.h>
#include <drivers/gpio.h>
#include <logging/log.h>
#include "ili932x.h"

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

#define TFTLCD_DELAY 0xFF


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
write_data8(uint8_t data)
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
read_data8(void)
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

static void
write_reg16(uint16_t reg)
{
    RS_COMMAND;
    DATA_OUTPUT;
    CS_ACTIVE;
    write_data8(reg >> 8);
    write_data8(reg & 0xFF);
    CS_IDLE;
}

static void
write_data16(uint16_t data)
{
    RS_DATA;
    DATA_OUTPUT;
    CS_ACTIVE;
    write_data8(data >> 8);
    write_data8(data & 0xFF);
    CS_IDLE;
}

static uint16_t
read_data16()
{
    uint16_t data;
    RS_DATA;
    DATA_INPUT;
    CS_ACTIVE;
    data = read_data8();
    data = data << 8;
    data = data | read_data8();
    CS_IDLE;
    return data;
}

static void
write16(uint16_t reg, uint16_t data)
{
    write_reg16(reg);
    write_data16(data);
}

static uint16_t
read16(uint16_t reg)
{
    write_reg16(reg);
    return read_data16();
}

static uint16_t
lcd_read_id(void)
{
    uint16_t id = read16(0x0000); // Driver Code register address
    printk("id: 0x%04x\n", id);
    return id;
}

static const uint16_t ILI932x_regValues[] = {
  ILI932X_START_OSC        , 0x0001, // Start oscillator
  TFTLCD_DELAY             , 50,     // 50 millisecond delay
  ILI932X_DRIV_OUT_CTRL    , 0x0100,
  ILI932X_DRIV_WAV_CTRL    , 0x0700,
  ILI932X_ENTRY_MOD        , 0x1030,
  ILI932X_RESIZE_CTRL      , 0x0000,
  ILI932X_DISP_CTRL2       , 0x0202,
  ILI932X_DISP_CTRL3       , 0x0000,
  ILI932X_DISP_CTRL4       , 0x0000,
  ILI932X_RGB_DISP_IF_CTRL1, 0x0,
  ILI932X_FRM_MARKER_POS   , 0x0,
  ILI932X_RGB_DISP_IF_CTRL2, 0x0,
  ILI932X_POW_CTRL1        , 0x0000,
  ILI932X_POW_CTRL2        , 0x0007,
  ILI932X_POW_CTRL3        , 0x0000,
  ILI932X_POW_CTRL4        , 0x0000,
  TFTLCD_DELAY             , 200,
  ILI932X_POW_CTRL1        , 0x1690,
  ILI932X_POW_CTRL2        , 0x0227,
  TFTLCD_DELAY             , 50,
  ILI932X_POW_CTRL3        , 0x001A,
  TFTLCD_DELAY             , 50,
  ILI932X_POW_CTRL4        , 0x1800,
  ILI932X_POW_CTRL7        , 0x002A,
  TFTLCD_DELAY             , 50,
  ILI932X_GAMMA_CTRL1      , 0x0000,
  ILI932X_GAMMA_CTRL2      , 0x0000,
  ILI932X_GAMMA_CTRL3      , 0x0000,
  ILI932X_GAMMA_CTRL4      , 0x0206,
  ILI932X_GAMMA_CTRL5      , 0x0808,
  ILI932X_GAMMA_CTRL6      , 0x0007,
  ILI932X_GAMMA_CTRL7      , 0x0201,
  ILI932X_GAMMA_CTRL8      , 0x0000,
  ILI932X_GAMMA_CTRL9      , 0x0000,
  ILI932X_GAMMA_CTRL10     , 0x0000,
  ILI932X_GRAM_HOR_AD      , 0x0000,
  ILI932X_GRAM_VER_AD      , 0x0000,
  ILI932X_HOR_START_AD     , 0x0000,
  ILI932X_HOR_END_AD       , 0x00EF,
  ILI932X_VER_START_AD     , 0X0000,
  ILI932X_VER_END_AD       , 0x013F,
  ILI932X_GATE_SCAN_CTRL1  , 0xA700, // Driver Output Control (R60h)
  ILI932X_GATE_SCAN_CTRL2  , 0x0003, // Driver Output Control (R61h)
  ILI932X_GATE_SCAN_CTRL3  , 0x0000, // Driver Output Control (R62h)
  ILI932X_PANEL_IF_CTRL1   , 0X0010, // Panel Interface Control 1 (R90h)
  ILI932X_PANEL_IF_CTRL2   , 0X0000,
  ILI932X_PANEL_IF_CTRL3   , 0X0003,
  ILI932X_PANEL_IF_CTRL4   , 0X1100,
  ILI932X_PANEL_IF_CTRL5   , 0X0000,
  ILI932X_PANEL_IF_CTRL6   , 0X0000,
  ILI932X_DISP_CTRL1       , 0x0133, // Main screen turn on
};

static void
lcd_9325_init()
{
    uint16_t i = 0, a, d;
    while(i < sizeof(ILI932x_regValues) / sizeof(uint16_t)) {
        a = ILI932x_regValues[i++];
        d = ILI932x_regValues[i++];
        if(a == TFTLCD_DELAY)
        {
            k_sleep(K_MSEC(d));
        }
        else
        {
            write16(a, d);
        }
    }
    //setRotation(rotation);
    //setAddrWindow(0, 0, TFTWIDTH-1, TFTHEIGHT-1);
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
        lcd_9325_init();
        for (int i = 0; i < 200; i++)
        {
            write16(0x0020, i);
            write16(0x0021, i);
            write16(0x0022, i << 3);
        }
        return 0;
    }

    printk("LCD ID is wrong: 0x%04X\n", id);
    return -1;
}