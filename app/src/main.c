#include <stdint.h>
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
K_MSGQ_DEFINE(knob_msgq, 4, 2, 1);

#define NUM_CHANNELS 8

static void knob_cb(struct input_event *evt)
{
	printk("Knob input detected, code: %d, value: %d\n", evt->code, evt->value);

	if (k_msgq_put(&knob_msgq, &evt->value, K_NO_WAIT) != 0) {
		LOG_ERR("Failed to put new input event");
	}
}

INPUT_CALLBACK_DEFINE(knob_dev, knob_cb);

void display_line(const struct device *dev, const char *line, const int line_no)
{
    // TODO probably need a mutex here
    // OR put the display in a separate thread
    int rc;

    rc = auxdisplay_cursor_position_set(dev, AUXDISPLAY_POSITION_ABSOLUTE, 0, line_no);
    if (rc != 0) {
        LOG_ERR("Failed to move cursor: %d", rc);
    }

    rc = auxdisplay_write(dev, line, 16);
    if (rc != 0) {
        LOG_ERR("Failed to write data: %d", rc);
    }
}
int main(void)
{
    int rc;
	const struct device *const display_dev = DEVICE_DT_GET(DT_NODELABEL(auxdisplay_0));
	uint8_t data[17];

	if (!device_is_ready(display_dev)) {
		LOG_ERR("Auxdisplay device is not ready.");
		return 0;
	}

	rc = auxdisplay_cursor_set_enabled(display_dev, false);

	int curr_channel = 0;
	uint32_t direction = 0;

	snprintk(data, 17, "Source Device %d ", curr_channel + 1);
	display_line(display_dev, data, 0);

	while (true) {
		k_msgq_get(&knob_msgq, &direction, K_FOREVER);
		curr_channel = (curr_channel + direction) % NUM_CHANNELS;

		snprintk(data, 17, "Source Device %d ", curr_channel + 1);
		display_line(display_dev, data, 0);

		// TODO
		// cancel existing utility thread if running
		// start utility thread with 500ms delay
		// utility thread will cancel existing ble audio thread if running
		// then start new ble audio thread on new channel
	}
	return 0;
}
