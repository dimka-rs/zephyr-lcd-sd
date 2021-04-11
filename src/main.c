#include <logging/log.h>
#include <zephyr.h>
#include <device.h>
#include "lcd.h"
#include "sd.h"

#define STACKSIZE 	2048
#define PRIORITY	7

#define LOG_MODULE_NAME main
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

void main(void)
{

	LOG_INF("START");

	//sd_init();
	lcd_init();

	for (;;) {
		k_sleep(K_SECONDS(1));
	}
}

