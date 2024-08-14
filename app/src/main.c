/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/auxdisplay.h>
#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>

#include <app/drivers/blink.h>

#include <app_version.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

const struct device *const knob_dev = DEVICE_DT_GET(DT_NODELABEL(knob_0));
K_MSGQ_DEFINE(mouse_msgq, MOUSE_REPORT_COUNT, 2, 1);

#define BLINK_PERIOD_MS_STEP 100U
#define BLINK_PERIOD_MS_MAX  1000U

static void knob_cb(struct input_event *evt)
{
	printk("Knob input detected, code: %d, value: %d\n", evt->code, evt->value);

	/*
	if (k_msgq_put(&mouse_msgq, tmp, K_NO_WAIT) != 0) {
		LOG_ERR("Failed to put new input event");
	}
	*/
}

INPUT_CALLBACK_DEFINE(knob_dev, knob_cb);

int main(void)
{
    int rc;
	const struct device *const dev = DEVICE_DT_GET(DT_NODELABEL(auxdisplay_0));
	uint8_t data[64];

	if (!device_is_ready(dev)) {
		LOG_ERR("Auxdisplay device is not ready.");
		return 0;
	}

	rc = auxdisplay_cursor_set_enabled(dev, false);

	if (rc != 0) {
		LOG_ERR("Failed to enable cursor: %d", rc);
	}

	snprintk(data, sizeof(data), "Hi Liana how areyou doing?");
	rc = auxdisplay_write(dev, data, strlen(data));

	if (rc != 0) {
		LOG_ERR("Failed to write data: %d", rc);
	}

	int curr_channel = 1;


	while (1) {
        rc = auxdisplay_channel_select(dev, channel);
        if (rc != 0) {
            LOG_ERR("Failed to select channel: %d", rc);
        }
        channel = (channel + 1) % 2;
        k_sleep(K_MSEC(1000));
    }

	return 0;
}
