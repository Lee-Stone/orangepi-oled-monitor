#ifndef BUZZER_H
#define BUZZER_H

int  buzzer_init(void);   /* 初始化 PI5 输出，返回 0 成功 */
void buzzer_beep_3(void); /* 短促鸣叫三声（嘀嘀嘀） */
void buzzer_off(void);    /* 关闭 */

#endif
