/* aw21036.h - driver for the BSides badge LED array (Awinic AW21036 over I2C1).
 *
 * Reverse engineered from the firmware:
 *   - I2C 7-bit address 0x34, on I2C1 (SCL=PB8, SDA=PB9).
 *   - 36-channel driver = 12 RGB LEDs; the badge populates 10 of them.
 *   - 8-bit PWM per channel (0..255), brightness-scaled in software.
 *   - LED -> channel mapping (see aw21036.c): slots 0..5 = ch 0..5,
 *     extras = ch 10,11,9,8.
 *
 * REGISTER ADDRESSES and VALUES below were recorded from a live I2C capture of
 * the firmware driving the AW21036 at 0x34. The init is reproduced in
 * aw21036_init(). Per frame the firmware writes BR(0x01,36B) then 0x49=0x00
 * (latch) - that is aw21036_show().
 */
#ifndef AW21036_H
#define AW21036_H

#include <stdint.h>
#include "main.h"

#define AW21036_ADDR7   0x34u          /* 7-bit; HAL wants it <<1 (0x68) */

/* ---- registers (CONFIRMED from firmware init) ---------------------------- */
#define AW21036_REG_GCR     0x00u      /* Global control: bit0 = CHIPEN       */
#define AW21036_REG_BR0     0x01u      /* PWM/brightness ch0; ch N at 0x01+N  */
#define AW21036_REG_UPDATE  0x49u      /* write 0x00 to latch BR/SL to output */
#define AW21036_REG_SL0     0x4Au      /* scaling/current ch0; ch N at 0x4A+N */
#define AW21036_REG_GCCR    0x6Eu      /* global current 0..255               */
#define AW21036_REG_CFG70   0x70u      /* config, firmware writes 0x80        */
#define AW21036_REG_CFG78   0x78u      /* config, firmware writes 0x00 at init */
#define AW21036_REG_CFG7C   0x7Cu      /* config, firmware writes 0x07        */
#define AW21036_REG_CFG90   0x90u      /* 3-byte config, firmware writes FF FF FF */
#define AW21036_REG_RESET   0x7Fu      /* software reset: write 0x00          */

#define AW21036_NCH         36         /* output channels                     */
#define AW21036_NLED        12         /* RGB LEDs (3 channels each)           */

/* Logical LEDs the badge uses. 0..5 = the six BSIDES slot LEDs,
 * 6..9 = the extra LEDs. */
#define BADGE_NLED          10

/* Init: reset state, set global current, scaling, enable. Returns HAL status.
 * global_current 0..255 (max brightness headroom). */
HAL_StatusTypeDef aw21036_init(I2C_HandleTypeDef *hi2c, uint8_t global_current);

/* Set one logical badge LED (0..BADGE_NLED-1) in the local framebuffer. */
void aw21036_set(uint8_t led, uint8_t r, uint8_t g, uint8_t b);

/* Push the framebuffer to the chip (writes all 36 PWM regs + latch). */
HAL_StatusTypeDef aw21036_show(I2C_HandleTypeDef *hi2c);

#endif /* AW21036_H */
