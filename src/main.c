/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define MODULE main

#include <zephyr/zephyr.h>
#include <app_event_manager.h>
#include <caf/events/module_state_event.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/hwinfo.h>

LOG_MODULE_REGISTER(MODULE);

uint8_t device_id[8];

void main(void)
{
	LOG_INF("Starting");
	hwinfo_get_device_id(device_id, sizeof(device_id));
	LOG_HEXDUMP_INF(device_id, sizeof(device_id), "Device ID");


	if (app_event_manager_init()) {
		LOG_ERR("Application Event Manager not initialized");
	} else {
		module_set_state(MODULE_STATE_READY);
	}
	LOG_INF("End");
}
