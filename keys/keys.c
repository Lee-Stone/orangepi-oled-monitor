/**
 * keys.c - 三键控制（寄存器轮询，按下-松开触发）
 * KEY1=PI1(wPi6) 关机确认 | KEY2=PI14(wPi9) 重启确认 | KEY3=PH4(wPi10) 返回状态监视
 * 按键接 GND（按下=低电平）；wiringPi 库函数上拉（pinMode + pullUpDnControl）
 */

#include "keys.h"
#include "ssd1306/ssd1306.h"
#include <wiringPi.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define KEY1 6   /* PI1  */
#define KEY2 9   /* PI14 */
#define KEY3 10  /* PH4  */

#define CONFIRM_TIMEOUT 5 /* 秒 */

typedef enum { MODE_NORMAL, MODE_SHUTDOWN, MODE_REBOOT } Mode;
static Mode mode = MODE_NORMAL;
static time_t confirm_at;
static volatile int f1, f2, f3;

/* 轮询线程：按下（下降沿）记录，松开（上升沿）触发；20ms 去抖确认 */
static void *key_thread(void *arg) {
    (void)arg;
    int last1 = digitalRead(KEY1), last2 = digitalRead(KEY2), last3 = digitalRead(KEY3);
    int down1 = 0, down2 = 0, down3 = 0;
    while (1) {
        int v1 = digitalRead(KEY1), v2 = digitalRead(KEY2), v3 = digitalRead(KEY3);
        if (last1 && !v1) down1 = 1;                        /* 按下 */
        if (!last1 && v1 && down1) { usleep(20000); if (digitalRead(KEY1)) { down1 = 0; f1 = 1; } }
        if (last2 && !v2) down2 = 1;
        if (!last2 && v2 && down2) { usleep(20000); if (digitalRead(KEY2)) { down2 = 0; f2 = 1; } }
        if (last3 && !v3) down3 = 1;
        if (!last3 && v3 && down3) { usleep(20000); if (digitalRead(KEY3)) { down3 = 0; f3 = 1; } }
        last1 = v1; last2 = v2; last3 = v3;
        usleep(10000);
    }
    return NULL;
}

int key_init(int oled_fd) {
    (void)oled_fd;
    wiringPiSetup();
    /* 库函数上拉输入（pinMode 的 sysfs 报错可忽略，GPIO 方向与上拉由寄存器完成） */
    pinMode(KEY1, INPUT); pullUpDnControl(KEY1, PUD_UP);
    pinMode(KEY2, INPUT); pullUpDnControl(KEY2, PUD_UP);
    pinMode(KEY3, INPUT); pullUpDnControl(KEY3, PUD_UP);
    pthread_t th;
    pthread_create(&th, NULL, key_thread, NULL);
    return 0;
}

static void show_msg(int fd, const char *l1, const char *l2) {
    ssd1306_clear(fd);
    ssd1306_show_str(fd, 0, 0, l1);
    ssd1306_show_str(fd, 0, 1, l2);
}

int key_poll(int oled_fd) {
    if (f3) { f3 = 0; mode = MODE_NORMAL; ssd1306_clear(oled_fd); return 2; } /* 2=已回正常，立即刷新状态 */

    if (f1) {
        f1 = 0;
        if (mode == MODE_NORMAL) {
            mode = MODE_SHUTDOWN; confirm_at = time(NULL);
            show_msg(oled_fd, "Are you sure you want", "to shut down?");
        } else if (mode == MODE_SHUTDOWN) {
            if (system("poweroff") != 0) { /* 关机失败则忽略 */ }
        } else {
            mode = MODE_NORMAL; ssd1306_clear(oled_fd);
        }
    }

    if (f2) {
        f2 = 0;
        if (mode == MODE_NORMAL) {
            mode = MODE_REBOOT; confirm_at = time(NULL);
            show_msg(oled_fd, "Are you sure you want", "to reboot?");
        } else if (mode == MODE_REBOOT) {
            if (system("reboot") != 0) { /* 重启失败则忽略 */ }
        } else {
            mode = MODE_NORMAL; ssd1306_clear(oled_fd);
        }
    }

    if (mode != MODE_NORMAL) {
        if (time(NULL) - confirm_at > CONFIRM_TIMEOUT) {
            mode = MODE_NORMAL;
            ssd1306_clear(oled_fd);
            return 0;
        }
        return 1;
    }
    return 0;
}
