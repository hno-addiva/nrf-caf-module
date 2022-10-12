/*
 * Copyright (c) 2022 Addiva Elektronik AB
 *
 * SPDX-License-Identifier: TBD
 */

#include <zephyr/kernel.h>

#define MODULE test_1
#include <caf/events/module_state_event.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MODULE);

#include "module_test_1.h"
#include "test_1_def.h"

#if CONFIG_CAF_BUTTON
static bool handle_button_event(const struct button_event *evt)
{
	if (evt->pressed) {
		switch (evt->key_id) {
		case BUTTON_ID_...:
            ...
			break;
		case BUTTON_ID_...:
            ...
			break;
		default:
			break;
		}
	}

	return false;
}
#endif

static bool handle_module_state_event(const struct module_state_event *evt)
{
    // Wait for dependent modules to initialize
	if (check_state(evt, MODULE_ID(main), MODULE_STATE_READY)) {
        // Initialize
        module_set_state(MODULE_STATE_READY);
    }
	return false;
}

static bool app_event_handler(const struct app_event_header *aeh)
{
    #if CONFIG_CAF_BUTTON
	if (is_button_event(aeh)) {
		return handle_button_event(cast_button_event(aeh));
	}
    #endif

	if (is_module_state_event(aeh)) {
        return handle_module_state_event(cast_module_state_event(aeh));
	}

	/* Event not handled but subscribed. */
	__ASSERT_NO_MSG(false);

	return false;
}

APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, module_state_event);
#if CONFIG_CAF_BUTTON
APP_EVENT_SUBSCRIBE(MODULE, button_event);
#endif

// Shell integration
#if CONFIG_SHELL
#include <zephyr/shell/shell.h>

static int test_1_echo(const struct shell *shell, size_t argc,
		char **argv)
{
    for (int i = 0; i < argc; i++) {
	    shell_fprintf(shell, SHELL_NORMAL, "echo '%s'\n", argv[i]);    
    }
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_test_1,
	SHELL_CMD_ARG(echo, NULL, "Echo", test_1_echo, 0, 0),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(MODULE, &sub_test_1, "Test_1 commands", NULL);

#endif
