/* aw21036.c - see aw21036.h. Reproduces the badge's LED behaviour on HAL I2C. */
#include "aw21036.h"

/* Framebuffer: 36 PWM channels (12 RGB LEDs). Index = channel*? no: we store
 * per channel so we can push it as one auto-incremented block to BR0..BR35. */
static uint8_t fb[AW21036_NCH];

/* Map a logical badge LED (0..9) to its AW21036 channel group (R channel).
 * Straight from the firmware (FUN_08006034 + remap table 05 04 03 02 01 00
 * 0a 0b 09 08): slots 0..5 -> ch 0..5, extras 0..3 -> ch 10,11,9,8.
 * Returned value is the R channel; G=+1, B=+2 (each group = 3 channels). */
static uint8_t led_to_group(uint8_t led)
{
    static const uint8_t grp[BADGE_NLED] = { 0, 1, 2, 3, 4, 5, 10, 11, 9, 8 };
    return grp[led] * 3u;
}

static HAL_StatusTypeDef wr(I2C_HandleTypeDef *hi2c, uint8_t reg,
                            const uint8_t *buf, uint16_t n)
{
    return HAL_I2C_Mem_Write(hi2c, AW21036_ADDR7 << 1, reg,
                             I2C_MEMADD_SIZE_8BIT, (uint8_t *)buf, n, 100);
}
static HAL_StatusTypeDef wr1(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t v)
{
    return wr(hi2c, reg, &v, 1);
}

HAL_StatusTypeDef aw21036_init(I2C_HandleTypeDef *hi2c, uint8_t global_current)
{
    HAL_StatusTypeDef s;
    uint8_t sl[AW21036_NCH];
    uint8_t cfg90[3] = { 0xFF, 0xFF, 0xFF };   /* exact bytes from the capture */

    /* Exact init sequence captured from the firmware driving the real chip. */
    if ((s = wr1(hi2c, AW21036_REG_RESET, 0x00)) != HAL_OK) return s; /* reset */
    if ((s = wr1(hi2c, AW21036_REG_CFG7C, 0x07)) != HAL_OK) return s;
    if ((s = wr1(hi2c, AW21036_REG_GCR,   0x01)) != HAL_OK) return s; /* CHIPEN; "no answer at 0x34" if NAK */

    for (int i = 0; i < AW21036_NCH; i++) sl[i] = 0xFF;             /* SL: full scaling (confirmed) */
    if ((s = wr(hi2c, AW21036_REG_SL0, sl, AW21036_NCH)) != HAL_OK) return s;
    if ((s = wr1(hi2c, AW21036_REG_GCCR, global_current)) != HAL_OK) return s; /* firmware used 0x80 */
    if ((s = wr(hi2c, AW21036_REG_CFG90, cfg90, 3))       != HAL_OK) return s;
    if ((s = wr1(hi2c, AW21036_REG_CFG70, 0x80))          != HAL_OK) return s;
    if ((s = wr1(hi2c, AW21036_REG_CFG78, 0x00))          != HAL_OK) return s;

    for (int i = 0; i < AW21036_NCH; i++) fb[i] = 0;                /* PWM off */
    if ((s = wr(hi2c, AW21036_REG_BR0, fb, AW21036_NCH)) != HAL_OK) return s;
    return wr1(hi2c, AW21036_REG_UPDATE, 0x00);                     /* latch */
}

void aw21036_set(uint8_t led, uint8_t r, uint8_t g, uint8_t b)
{
    if (led >= BADGE_NLED) return;
    uint8_t c = led_to_group(led);
    fb[c + 0] = r;
    fb[c + 1] = g;
    fb[c + 2] = b;
}

HAL_StatusTypeDef aw21036_show(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef s = wr(hi2c, AW21036_REG_BR0, fb, AW21036_NCH);
    if (s != HAL_OK) return s;
    return wr1(hi2c, AW21036_REG_UPDATE, 0x00);   /* latch */
}

/* ---- example use ----------------------------------------------------------
 *   aw21036_init(&hi2c1, 0x80);            // half global current
 *   aw21036_set(0, 255, 0, 0);             // slot 0 red
 *   aw21036_set(6, 0, 255, 0);             // first extra LED green
 *   aw21036_show(&hi2c1);
 * The firmware pushes a frame like this every tick after brightness-scaling.
 * --------------------------------------------------------------------------- */
