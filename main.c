/**
 * main.c - Orange Pi OLED Terminal
 * 第0行: IP地址  第1行: CPU温度  第2行: 内存占用率
 * wiringPi + SSD1306 (128x32), I2C-4, 地址 0x3C
 */

#include "ssd1306/ssd1306.h"
#include "ina219/ina219.h"
#include "led/led.h"
#include "keys/keys.h"
#include "buzzer/buzzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

#define BATT_LOW_PCT 20   /* 低电量报警阈值(%) */
#define BEEP_INTERVAL 10  /* 低电量报警间隔(秒) */

static int g_fd = -1;
static time_t last_beep = 0;

/* 退出/被杀时清屏 */
static void handle_signal(int sig) {
    (void)sig;
    if (g_fd >= 0) {
        ssd1306_clear(g_fd);
        ssd1306_close(g_fd);
    }
    led_breath_stop();
    buzzer_off();
    exit(0);
}
#include <arpa/inet.h>

/* 获取本机 IPv4 地址（跳过 loopback） */
static void get_ip(char *buf, size_t len) {
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        snprintf(buf, len, "IP:none");
        return;
    }
    for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET)
            continue;
        if (!strcmp(ifa->ifa_name, "lo"))
            continue;
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr,
                  ip, sizeof(ip));
        snprintf(buf, len, "IP:%s", ip);
        freeifaddrs(ifaddr);
        return;
    }
    freeifaddrs(ifaddr);
    snprintf(buf, len, "IP:none");
}

/* 获取 CPU 温度（毫摄氏度转摄氏度） */
static void get_temp(char *buf, size_t len) {
    FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (!fp) {
        snprintf(buf, len, "Temp: N/A");
        return;
    }
    int mtemp = 0;
    if (fscanf(fp, "%d", &mtemp) == 1) {
        snprintf(buf, len, "Temp: %.1fC", mtemp / 1000.0);
    } else {
        snprintf(buf, len, "Temp: N/A");
    }
    fclose(fp);
}

/* 获取内存占用率百分比 */
static void get_mem(char *buf, size_t len) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        snprintf(buf, len, "Mem:N/A");
        return;
    }
    unsigned long total = 0, avail = 0;
    char key[64], unit[16];
    unsigned long val;
    while (fscanf(fp, "%63s %lu %15s", key, &val, unit) == 3) {
        if (!strcmp(key, "MemTotal:")) total = val;
        else if (!strcmp(key, "MemAvailable:")) { avail = val; break; }
    }
    fclose(fp);
    if (!total) {
        snprintf(buf, len, "Mem:N/A");
        return;
    }
    snprintf(buf, len, "Mem:%d%%", (int)((total - avail) * 100 / total));
}

/* 获取电池电压与电量（INA219，/dev/i2c-2） */
static void get_bat(char *buf, size_t len) {
    int pct = ina219_battery_percent();
    if (pct < 0) {
        snprintf(buf, len, "Bat:N/A");
        return;
    }
    snprintf(buf, len, "Bat:%.2fV %d%%", ina219_bus_voltage(), pct);
}

int main(void) {
    int fd = ssd1306_init("/dev/i2c-3");
    if (fd < 0) {
        fprintf(stderr, "FAILED! Board connected? (i2cdetect -y 3)\n");
        return EXIT_FAILURE;
    }
    g_fd = fd;

    /* INA219 电池监测（未接则第 3 行显示 Bat:N/A） */
    if (ina219_init("/dev/i2c-2") < 0)
        fprintf(stderr, "INA219 not found on /dev/i2c-2 (i2cdetect -y 2)\n");

    /* LED 呼吸灯（wiringPi 硬件 PWM pwm3 → PI13） */
    if (led_init() < 0)
        fprintf(stderr, "LED PWM init failed\n");
    else
        led_breath_start();

    /* 三键：KEY1 关机 / KEY2 重启 / KEY3 返回监视 */
    key_init(fd);

    /* 蜂鸣器：低电量报警 */
    buzzer_init();

    /* 捕获退出信号，清屏 */
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGHUP, handle_signal);

    char ip[32], temp[32], mem[32], bat[32];
    int tick = 0;

    while (1) {
        int r = key_poll(fd);
        if (r == 1) { usleep(100000); continue; }        /* 确认模式：快速响应按键 */
        if (r == 2 || ++tick % 10 == 0) {                /* KEY3 或每秒：刷新状态 */
            get_ip(ip, sizeof(ip));
            get_temp(temp, sizeof(temp));
            get_mem(mem, sizeof(mem));
            get_bat(bat, sizeof(bat));

            ssd1306_show_str(fd, 0, 0, ip);
            ssd1306_show_str(fd, 0, 1, temp);
            ssd1306_show_str(fd, 0, 2, mem);
            ssd1306_show_str(fd, 0, 3, bat);

            /* 低电量报警：低于阈值时每 10 秒嘀三声，恢复正常自动停止 */
            int pct = ina219_battery_percent();
            if (pct >= 0 && pct < BATT_LOW_PCT && time(NULL) - last_beep >= BEEP_INTERVAL) {
                last_beep = time(NULL);
                buzzer_beep_3();
            }
        }
        usleep(100000);
    }

    ssd1306_close(fd);
    return EXIT_SUCCESS;
}
