# Orange Pi OLED Monitor

Orange Pi（香橙派）OLED 状态监视器：128×32 SSD1306 屏幕实时显示系统状态，支持电池电量监测（INA219）、LED 呼吸灯、三键关机/重启控制、低电量蜂鸣报警。

## 功能

- **OLED 四行状态显示**（I2C）
  - 第 0 行：IP 地址
  - 第 1 行：CPU 温度
  - 第 2 行：内存占用率
  - 第 3 行：电池电压与电量百分比
- **电池监测**：INA219 电流/电压传感器，2S 锂电（满电 8.4V，截止 6.0V），线性映射电量
- **LED 呼吸灯**：wiringPi 硬件 PWM（pwm3 → PI13），2 秒正弦呼吸
- **三键控制**（按下-松开触发）
  - KEY1：确认关机（3 秒内再按一次执行 `poweroff`）
  - KEY2：确认重启（3 秒内再按一次执行 `reboot`）
  - KEY3：返回状态监视
  - 确认超时 3 秒自动取消，防误触
- **低电量报警**：电量低于 20% 时蜂鸣器每 10 秒"嘀嘀嘀"三声，恢复正常自动停止

## 硬件接线

| 外设 | 引脚 | 说明 |
|---|---|---|
| OLED (SSD1306 128x32) | `/dev/i2c-4`，地址 `0x3C` | I2C |
| INA219 | `/dev/i2c-3`，地址 `0x42` | 采样电阻 100mΩ，测电池电压/电流 |
| LED | PI13（wPi 2，物理 7） | 高电平点亮，PWM3 硬件 PWM |
| KEY1 | PI1（wPi 6，物理 12） | 按键另一端接 GND |
| KEY2 | PI14（wPi 9，物理 16） | 按键另一端接 GND |
| KEY3 | PH4（wPi 10，物理 18） | 按键另一端接 GND |
| 蜂鸣器 | PI5（wPi 8，物理 15） | 有源蜂鸣器：+ 接电源，- 接 PI5（低电平鸣叫） |

> 注意：本板 wiringPi 库函数 `pullUpDnControl` 实测无效，按键上拉依赖 `pinMode` 的寄存器配置；如遇按键不稳定，建议加 10KΩ 外部上拉电阻到 3.3V。

## 编译与运行

依赖：wiringPi（`sudo apt install wiringpi`）

```sh
make                    # 编译，产物 main
sudo ./main             # 前台运行（需要 root 访问 I2C/GPIO）

# 或安装为 systemd 开机自启服务
sudo bash install.sh -enable    # 安装并启动
sudo bash install.sh -disable   # 停止并卸载
```

## 配置

- `main.c` 顶部：
  - `BATT_LOW_PCT`：低电量报警阈值（默认 20%）
  - `BEEP_INTERVAL`：报警间隔秒数（默认 10）
- `ina219/ina219.h`：`INA219_BAT_FULL` / `INA219_BAT_EMPTY`（满电/截止电压）
- `keys/keys.c`：`CONFIRM_TIMEOUT`（确认超时秒数）
- `buzzer/buzzer.c`：`BEEP_MS` / `GAP_MS`（三声节奏）
- `led/led.c`：`PWM_ARR` / `PWM_DIV`（呼吸灯频率）

## 项目结构

```
├── main.c              主程序（状态采集/显示/低电量报警）
├── Makefile
├── install.sh          systemd 服务安装脚本
├── ssd1306/            OLED 驱动（6x8 ASCII 字库）
├── ina219/             电池电压/电流驱动
├── led/                PWM 呼吸灯
├── keys/               三键轮询与关机/重启确认
└── buzzer/             蜂鸣器驱动
```

## License

[MIT](LICENSE)
