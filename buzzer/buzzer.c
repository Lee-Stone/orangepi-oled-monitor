/**
 * buzzer.c - 有源蜂鸣器驱动（PI5 = wPi 8，实测低电平鸣叫）
 * 接法：蜂鸣器 + 接电源，- 接 PI5 → 引脚 LOW 鸣叫、HIGH 静音
 * 嘀嘀嘀三声：每声 100ms，间隔 100ms
 */

#include "buzzer.h"
#include <wiringPi.h>
#include <unistd.h>

#define BUZZER  8      /* PI5 = wPi 8 = 物理引脚 15 */

#define BEEP_MS 100    /* 每声时长 */
#define GAP_MS  100    /* 三声之间的间隔 */

int buzzer_init(void) {
    wiringPiSetup();
    pinMode(BUZZER, OUTPUT);
    digitalWrite(BUZZER, HIGH);   /* 默认静音 */
    return 0;
}

void buzzer_beep_3(void) {
    for (int i = 0; i < 3; i++) {
        digitalWrite(BUZZER, LOW);     /* 响 */
        usleep(BEEP_MS * 1000);
        digitalWrite(BUZZER, HIGH);    /* 停 */
        if (i < 2) usleep(GAP_MS * 1000);
    }
}

void buzzer_off(void) {
    digitalWrite(BUZZER, HIGH);   /* 静音 */
}
