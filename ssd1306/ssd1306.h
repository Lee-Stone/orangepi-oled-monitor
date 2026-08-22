#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

#define SSD1306_I2C_ADDR    0x3C
#define SSD1306_WIDTH       128
#define SSD1306_HEIGHT      32

int  ssd1306_init(const char *i2c_dev);
void ssd1306_close(int fd);
void ssd1306_clear(int fd);
void ssd1306_fill(int fd);
void ssd1306_set_pos(int fd, uint8_t x, uint8_t y);
void ssd1306_show_char(int fd, uint8_t x, uint8_t y, char ch);
void ssd1306_show_str(int fd, uint8_t x, uint8_t y, const char *str);

#endif

