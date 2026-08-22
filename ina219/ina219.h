#ifndef INA219_H
#define INA219_H

#include <stdint.h>

#define INA219_I2C_ADDR   0x42    /* A1=高 / A0=低（i2cdetect -y 3 实测） */
#define INA219_BAT_FULL   8.4f    /* 2S 锂电池满电电压 (V) */
#define INA219_BAT_EMPTY  6.0f    /* 2S 锂电池截止电压 (V) */

int   ina219_init(const char *i2c_dev);
void  ina219_close(void);
float ina219_bus_voltage(void);     /* 总线电压 (V) */
float ina219_current(void);         /* 电流 (A) */
int   ina219_battery_percent(void); /* 电量 0-100，-1=不可用 */

#endif
