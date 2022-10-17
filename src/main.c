/*
 * Copyright (c) 2022 Addiva Elektronik AB
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define MODULE main

#include <zephyr/zephyr.h>
#include <app_event_manager.h>
#include <caf/events/module_state_event.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/settings/settings.h>
#include <zephyr/shell/shell.h>

LOG_MODULE_REGISTER(MODULE);

uint8_t device_id[8];

void main(void)
{
	LOG_INF("Starting");

	// Prepare whats needed before application starts
	hwinfo_get_device_id(device_id, sizeof(device_id));
	LOG_HEXDUMP_INF(device_id, sizeof(device_id), "Device ID");

	// Initialize system services, storage, settings etc.
	settings_subsys_init();
	settings_load();

	// Start CAF and signal that main have completed the system setup
	if (app_event_manager_init()) {
		LOG_ERR("Application Event Manager not initialized");
	} else {
		module_set_state(MODULE_STATE_READY);
	}

	LOG_INF("Done");
}

// settings shell is missing a reload command to load new settings
// after written
static int cmd_reload(const struct shell *shell, size_t argc,
                         char **argv)
{
	LOG_INF("Reloading settings");
	settings_load();
}

SHELL_CMD_REGISTER(reload, NULL, "Reload settings", cmd_reload);