/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/hwinfo.h>

#define MODULE main

LOG_MODULE_REGISTER(MODULE);

uint8_t device_id[8];

void main(void)
{
	LOG_INF("Starting");
	hwinfo_get_device_id(device_id, sizeof(device_id));
	LOG_HEXDUMP_INF(device_id, sizeof(device_id), "Device ID");

	LOG_INF("End");
}
