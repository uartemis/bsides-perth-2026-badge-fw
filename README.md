# uartemis_fw
```
          __  __   ___    ____  ______               _
         / / / /  /   |  / __ \/_  __/___  ____ ___ (_)____
        / / / /  / /| | / /_/ / / / / _ \/ __ `__ \/ / ___/
       / /_/ /  / ___ |/ _, _/ / / /  __/ / / / / / (__  )
       \____/  /_/  |_/_/ |_| /_/  \___/_/ /_/ /_/_/____/
                           ______________________
                      ____/_____________________/|
                     /_________________________/ |
                     |                         | |
                     |  CCTV   .               | |
                     |                  _______| |
                     |_________________|  ((O))|/
                              \________|_______|
                                 /    \
                      _..---.._.'      |
                   .-'                 |
                 .'       .------.     |
                /       .'        \    |
               /       /    __     |   |
              ;       / _.-'  /   /    ;
              |      /-'   .-'  .'    /
              |    .'   .-'   .'    .'
              \_.-'  .-'   .-'   _.'
            .-'   .-'  _.-'__..-'
         .-'   .-'_.--'      ||   ||
      .-'  _.-'             ||   ||
     '----'              ___||   ||___
                        / / /     \ \ \
                 =========================

              B I R D S  A R E N ' T  R E A L
```
## What this is

A STM32CubeMX project for the BSides Perth 2026 badge MCU. The pin configuration here was recovered off the original hardware. The firmware was dumped over the onboard CH340, decompiled, and the live GPIO/peripheral setup read straight out of the running chip (clock tree, USART1 on PA9/PA10, I2C1 on PB8/PB9 for the LED driver, and the handful of enable lines the board holds high). That config was then transcribed back into CubeMX so the board can be rebuilt, modified, and reflashed from scratch. 

## Hardware

- **MCU:** STM32G031K8T7 (Cortex-M0+, 64 KB flash, 8 KB RAM), HSI @ 16 MHz, no crystal
- **LEDs:** AW21036 36-channel driver on I2C1 (addr 0x34), driver in `Core/Src/aw21036.c`
- **Serial:** USART1 → onboard CH340, PA9 (TX) / PA10 (RX), 115200 8N1
- **Enable:** PA3 held high (board power/peripheral enable)

## Build & flash

```sh
make # -> build/uartemis_fw.bin
stm32flash -b 115200 -w build/uartemis_fw.bin /dev/ttyUSB0
```

Open a serial terminal at 115200. Hit enter to see a message displayed over serial.

The LEDs will display a circular chasing rainbow. Use the sample and send buttons to speed up/slow down the chase.

## Layout

- `Core/Src/main.c` - init, LED identity pattern, rainbow chase
- `Core/Src/aw21036.c` / `.h` - LED driver (reset, scaling, global current, latch)
- `uartemis_fw.ioc` - the CubeMX project; re-import to change the pinout
