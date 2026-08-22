TARGET  = main
CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Issd1306 -I.
LDLIBS  = -lwiringPi -lpthread -lm

SRCS    = main.c ssd1306/ssd1306.c ina219/ina219.c led/led.c keys/keys.c buzzer/buzzer.c
OBJS    = main.o ssd1306/ssd1306.o ina219/ina219.o led/led.o keys/keys.o buzzer/buzzer.o

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)
	@echo "==> Build OK: ./$(TARGET)"

main.o: main.c ssd1306/ssd1306.h ina219/ina219.h led/led.h keys/keys.h buzzer/buzzer.h
	$(CC) $(CFLAGS) -c -o $@ $<

ssd1306/ssd1306.o: ssd1306/ssd1306.c ssd1306/ssd1306.h
	$(CC) $(CFLAGS) -c -o $@ $<

ina219/ina219.o: ina219/ina219.c ina219/ina219.h
	$(CC) $(CFLAGS) -c -o $@ $<

led/led.o: led/led.c led/led.h
	$(CC) $(CFLAGS) -c -o $@ $<

keys/keys.o: keys/keys.c keys/keys.h
	$(CC) $(CFLAGS) -c -o $@ $<

buzzer/buzzer.o: buzzer/buzzer.c buzzer/buzzer.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	sudo ./$(TARGET)
