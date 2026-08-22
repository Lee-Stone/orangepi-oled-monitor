/**
 * led.c - LED 呼吸灯（wiringPi 硬件 PWM）
 * 引脚 wPi 2 = PI13（物理引脚 7），高电平点亮
 * 占空比 = CCR/ARR：pwmSetRange 设 ARR，pwmWrite 设 CCR；2s 正弦呼吸
 */

#include "led.h"
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>

#define LED_PIN 2          /* wPi 2 = PI13 = PWM3 */
#define PWM_ARR 1000       /* 周期（range） */
#define PWM_DIV 100        /* 时钟分频 */

static pthread_t g_thread;
static volatile int g_run = 0;

int led_init(void) {
    if (wiringPiSetup() < 0) return -1;
    pinMode(LED_PIN, PWM_OUTPUT);
    pwmSetRange(LED_PIN, PWM_ARR);
    pwmSetClock(LED_PIN, PWM_DIV);
    pwmWrite(LED_PIN, 0);
    return 0;
}

static void *breath(void *arg) {
    (void)arg;
    while (g_run) {
        for (int i = 0; i < 100; i++) {      /* 100 步 × 20ms = 2s 一个呼吸 */
            if (!g_run) break;
            pwmWrite(LED_PIN, (int)(PWM_ARR * sin(M_PI * i / 100.0)));
            usleep(20000);
        }
    }
    return NULL;
}

void led_breath_start(void) {
    if (g_run) return;
    g_run = 1;
    pthread_create(&g_thread, NULL, breath, NULL);
}

void led_breath_stop(void) {
    if (!g_run) return;
    g_run = 0;
    pthread_join(g_thread, NULL);
    pwmWrite(LED_PIN, 0);
}
