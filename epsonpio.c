/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright (C) 2026 Ciprian Ionescu <me@ciprian-ionescu.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
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

static void pios_start(void)
{
    clk_div8_program_init(pio2, 0, clk_offset, PIN_DCLK, PIN_CLKDIV);
    dsio_tx_program_init(pio0, 0, tx_offset, PIN_DSIO, PIN_DST2);
    dsio_rx_program_init(pio1, 0, rx_offset, PIN_DSIO);

    pio_sm_set_enabled(pio2, 0, true);
    pio_sm_set_enabled(pio1, 0, true);
    pio_sm_set_enabled(pio0, 0, true);
}

static void pios_stop(void)
{
    pio_sm_set_enabled(pio0, 0, false);
    pio_sm_set_enabled(pio1, 0, false);
    pio_sm_set_enabled(pio2, 0, false);

    // don't let leftovers from this session leak into the next one
    tud_cdc_read_flush();

    gpio_set_function(PIN_DSIO, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_DSIO, GPIO_IN);
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

    clk_offset = pio_add_program(pio2, &clk_div8_program);
    tx_offset = pio_add_program(pio0, &dsio_tx_program);
    rx_offset = pio_add_program(pio1, &dsio_rx_program);

    bool connected = false;

    while (1)
    {
        tud_task();

        if (tud_cdc_connected() != connected)
        {
            connected = !connected;
            connected ? pios_start() : pios_stop();
        }

        if (!connected)
            continue;

        while (tud_cdc_available() && !pio_sm_is_tx_fifo_full(pio0, 0))
            pio_sm_put(pio0, 0, tud_cdc_read_char());

        while (!pio_sm_is_rx_fifo_empty(pio1, 0))
            tud_cdc_write_char(pio_sm_get(pio1, 0) >> 24);

        tud_cdc_write_flush();
    }
}
