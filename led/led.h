#ifndef LED_H
#define LED_H

int  led_init(void);         /* 初始化 PWM3（PI13），返回 0 成功 */
void led_breath_start(void); /* 启动呼吸线程（非阻塞） */
void led_breath_stop(void);  /* 停止呼吸并关闭 PWM */

#endif
