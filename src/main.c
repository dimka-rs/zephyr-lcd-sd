#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <stdio.h>
#include <logging/log.h>
#include <disk/disk_access.h>
#include <fs/fs.h>
#include <ff.h>
#include "lcd.h"


#define LOG_MODULE_NAME lcd_sd
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define STACKSIZE 	2048
#define PRIORITY	7
#define LED_PIN 	17
#define RUN_LED_BLINK_IVAL_MSEC 500

K_HEAP_DEFINE(fs_heap, 1000);

/* SD + FS */

static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};

/*
*  Note the fatfs library is able to mount only strings inside _VOLUME_STRS
*  in ffconf.h
*/
static const char *disk_mount_pt = "/SD:";


static int lsdir(const char *path, int depth, int current)
{
	int res;
	struct fs_dir_t dirp;
	static struct fs_dirent entry;

	fs_dir_t_init(&dirp);

	/* Verify fs_opendir() */
	res = fs_opendir(&dirp, path);
	if (res) {
		printk("Error opening dir %s [%d]\n", path, res);
		return res;
	}

	char *int_dir = k_heap_alloc(&fs_heap, 100, K_NO_WAIT);
	char *int_dir_p;
	memcpy(int_dir, path, strlen(path));
	int_dir[strlen(path)] = '/';
	int_dir_p = &int_dir[0] + strlen(path) + 1;

	for (;;) {
		/* Verify fs_readdir() */
		res = fs_readdir(&dirp, &entry);

		/* entry.name[0] == 0 means end-of-dir */
		if (res || entry.name[0] == 0) {
			break;
		}

		/* tree hierarchy */
		printk("+");
		for (int i = 0; i < current; i++)
		{
			printk("-");
		}
		if (entry.type == FS_DIR_ENTRY_DIR) {
			printk("[DIR ] %s\n", entry.name);
			memcpy(int_dir_p, entry.name, strlen(entry.name));
			int_dir[strlen(path)+strlen(entry.name)+1] = 0;
			if (depth > 0)
			{
				lsdir(int_dir, depth-1, current+1);
			}
		} else {
			printk("[FILE] %s (size = %zu)\n",
				entry.name, entry.size);
		}
	}

	/* Verify fs_closedir() */
	fs_closedir(&dirp);
	k_heap_free(&fs_heap, int_dir);
	return res;
}

void test_sd(void)
{
	/* raw disk i/o */
	do {
		static const char *disk_pdrv = "SD";
		uint64_t memory_size_mb;
		uint32_t block_count;
		uint32_t block_size;

		if (disk_access_init(disk_pdrv) != 0) {
			LOG_ERR("Storage init ERROR!");
			break;
		}

		if (disk_access_ioctl(disk_pdrv,
				DISK_IOCTL_GET_SECTOR_COUNT, &block_count)) {
			LOG_ERR("Unable to get sector count");
			break;
		}
		LOG_INF("Block count %u", block_count);

		if (disk_access_ioctl(disk_pdrv,
				DISK_IOCTL_GET_SECTOR_SIZE, &block_size)) {
			LOG_ERR("Unable to get sector size");
			break;
		}
		printk("Sector size %u\n", block_size);

		memory_size_mb = (uint64_t)block_count * block_size;
		printk("Memory Size(MB) %u\n", (uint32_t)(memory_size_mb >> 20));
	} while (0);

	mp.mnt_point = disk_mount_pt;

	int res = fs_mount(&mp);

	if (res == FR_OK) {
		printk("Disk mounted.\n");
		printk("\nListing dir %s\n", disk_mount_pt);
		lsdir(disk_mount_pt, 3, 0);
		if (fs_unmount(&mp))
		{
			LOG_ERR("Error unmounting %s\n", mp.mnt_point);
		}
	} else {
		printk("Error mounting disk.\n");
	}
}

void
test_lcd(void)
{
	lcd_init();
}

void main(void)
{

	LOG_INF("START");
	printk("Starting\n");

	//test_sd();

	test_lcd();

	for (;;) {
		k_sleep(K_MSEC(RUN_LED_BLINK_IVAL_MSEC));
	}
}

