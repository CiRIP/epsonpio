#include "bsp/board.h"
#include "tusb.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "clk_div.pio.h"
#include "dsio_tx.pio.h"
#include "dsio_rx.pio.h"

#define PIN_DSIO 10
#define PIN_DCLK 11
#define PIN_DST2 12
#define PIN_CLKDIV 13

static uint tx_offset, rx_offset, clk_offset;
static bool pios_active = false;

static void pios_start(void)
{
    clk_offset = pio_add_program(pio2, &clk_div8_program);
    clk_div8_program_init(pio2, 0, clk_offset, PIN_DCLK, PIN_CLKDIV);

    tx_offset = pio_add_program(pio0, &dsio_tx_program);
    dsio_tx_program_init(pio0, 0, tx_offset, PIN_DSIO, PIN_DST2, PIN_CLKDIV);

    rx_offset = pio_add_program(pio1, &dsio_rx_program);
    dsio_rx_program_init(pio1, 0, rx_offset, PIN_DSIO);

    pio_sm_set_enabled(pio2, 0, true);
    pio_sm_set_enabled(pio1, 0, true);
    pio_sm_set_enabled(pio0, 0, true);

    pios_active = true;
}

static void pios_stop(void)
{
    pio_sm_set_enabled(pio0, 0, false);
    pio_sm_set_enabled(pio1, 0, false);
    pio_sm_set_enabled(pio2, 0, false);

    pio_sm_drain_tx_fifo(pio0, 0);
    while (!pio_sm_is_rx_fifo_empty(pio1, 0))
        (void)pio_sm_get(pio1, 0);

    pio_remove_program(pio0, &dsio_tx_program, tx_offset);
    pio_remove_program(pio1, &dsio_rx_program, rx_offset);
    pio_remove_program(pio2, &clk_div8_program, clk_offset);

    gpio_set_function(PIN_DSIO, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_DSIO, GPIO_IN);

    pios_active = false;
}

int main()
{
    board_init();
    tusb_init();

    gpio_init(PIN_DST2);
    gpio_set_dir(PIN_DST2, GPIO_IN);
    gpio_pull_down(PIN_DST2);

    gpio_init(PIN_DCLK);
    gpio_set_dir(PIN_DCLK, GPIO_IN);
    gpio_pull_down(PIN_DCLK);

    gpio_init(PIN_DSIO);
    gpio_set_dir(PIN_DSIO, GPIO_IN);
    gpio_pull_up(PIN_DSIO);

    bool last_connected = false;

    while (1)
    {
        tud_task();

        bool connected = tud_cdc_connected();

        if (connected && !last_connected)
            pios_start();

        if (!connected && last_connected)
            pios_stop();

        last_connected = connected;

        if (!pios_active)
            continue;

        // USB -> TX FIFO
        while (tud_cdc_available() && !pio_sm_is_tx_fifo_full(pio0, 0))
        {
            uint8_t b;
            tud_cdc_read(&b, 1);
            pio_sm_put(pio0, 0, (uint32_t)b);
        }

        // RX FIFO -> USB
        while (!pio_sm_is_rx_fifo_empty(pio1, 0))
        {
            uint32_t word = pio_sm_get(pio1, 0);
            uint8_t b = (uint8_t)(word >> 24);
            tud_cdc_write(&b, 1);
        }

        tud_cdc_write_flush();
    }
}