/**
 * ina219.c - INA219 电流/电压传感器驱动 (I2C)
 * 电池：2S 锂电，满电 8.4V；采样电阻 100mΩ
 * 与 ssd1306 同款 wiringPiI2C 读写；INA219 寄存器为大端，
 * wiringPi 的 16 位读写为小端，故读写各交换一次字节
 */

#include "ina219.h"
#include <unistd.h>
#include <wiringPiI2C.h>

static int g_fd = -1;

static uint16_t read_reg(uint8_t reg) {
    return __builtin_bswap16((uint16_t)wiringPiI2CReadReg16(g_fd, reg));
}

static void write_reg(uint8_t reg, uint16_t val) {
    wiringPiI2CWriteReg16(g_fd, reg, __builtin_bswap16(val));
}

int ina219_init(const char *i2c_dev) {
    g_fd = wiringPiI2CSetupInterface(i2c_dev, INA219_I2C_ADDR);
    if (g_fd < 0) return -1;
    write_reg(0x00, 0x0CFF); /* 16V / ±320mV(0.1Ω→3.2A) / 12bit / 连续 */
    write_reg(0x05, 4194);   /* LSB≈97.66µA，Cal=0.04096/(LSB×0.1Ω)=4194 */
    return 0;
}

void ina219_close(void) {
    if (g_fd >= 0) { close(g_fd); g_fd = -1; }
}

/* 总线电压：寄存器 bit15:3，LSB=4mV */
float ina219_bus_voltage(void) {
    return (read_reg(0x02) >> 3) * 4.0f / 1000.0f;
}

/* 电流：带符号 16 位，LSB=97.66µA（即 100/1024 µA） */
float ina219_current(void) {
    return (int16_t)read_reg(0x04) * (100.0f / 1024.0f) / 1000.0f;
}

/* 电量：满电 8.4V / 截止 6.0V 线性映射并限幅 */
int ina219_battery_percent(void) {
    float v = ina219_bus_voltage();
    if (v < 0.1f) return -1;
    int pct = (int)((v - INA219_BAT_EMPTY) * 100.0f /
                    (INA219_BAT_FULL - INA219_BAT_EMPTY));
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    return pct;
}
