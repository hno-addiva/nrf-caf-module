/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/logging/log.h>

#define MODULE main

LOG_MODULE_REGISTER(MODULE);

#include <zephyr/drivers/hwinfo.h>
void main(void)
{
	LOG_INF("Starting");
	LOG_INF("End");
}
