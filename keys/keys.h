#ifndef KEYS_H
#define KEYS_H

int key_init(int oled_fd); /* 初始化三键（上拉 + 下降沿中断） */
int key_poll(int oled_fd); /* 处理按键，返回 1=确认模式（主循环跳过刷新） */

#endif
