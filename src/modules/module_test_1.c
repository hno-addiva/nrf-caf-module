/*
 * Copyright (c) 2022 Addiva Elektronik AB
 *
 * SPDX-License-Identifier: TBD
 */

/*
 * Dummy module with dedicated work queue for processing
 * 
 * - CAF handler processing incoming events from other modules (only module_ready handled for now)
 * - Module initialization triggered from main module (main.c) signalling it's ready
 * - Timer firing an automatic work every minute
 * - Shell command for firing a test work on demand
 * - Settings subsys
 */


#define MODULE test_1

#include <zephyr/kernel.h>
#include <caf/events/module_state_event.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

LOG_MODULE_REGISTER(MODULE, LOG_LEVEL_DBG);

#include "test_1_def.h"

// Module state
enum {
	M_STATE_UNINITIALIZED = 0,
	M_STATE_READY
} module_state = {
	M_STATE_UNINITIALIZED
};

// Configurable parameters
static int foo = 0;

// Work queue
static K_THREAD_STACK_DEFINE(stack_area, 2048);
static struct k_work_q work_q;

// Every work item should have a context struct, where k_work work is a member.
// this allows for other data to be passed to the work handler. Use macros below
// to avoid referencing the k_work item maually.
/*

// Declare the local context for the work with a and struct k_work work member, and preferably with _context suffix, 
struct somework_context {
	struct k_work work;
	// Other parameters as needed
	...
};

// Define the function handling the work, preferably with _task suffix
static void somework_task(struct k_work *work)
{
	struct somework_context *context = CONTAINER_OF(work, struct somework_context, work);
	LOG_DBG("somework_task");
	...
}

// Define individual work items for the work task, with unique names.
static struct somework_context somework = {
	WORK_INIT(somework_task),
	// Other static initializations if needed
};

    // Schedule work, can be called in any function
	work_submit(somework);

*/

#define work_submit(name) \
	k_work_submit_to_queue(&work_q, &name.work)
#define work_init(name, fn) \
	k_work_init(&name.work, fn)
#define WORK_INIT(fn) \
	.work = Z_WORK_INITIALIZER(fn)

/*
 * Basic work item
 */

struct work1_context {
		struct k_work work;
};

static void work1_task(struct k_work *work)
{
	struct work1_context __unused *context = CONTAINER_OF(work, struct work1_context, work);
	LOG_DBG("start");
	k_msleep(1000);
	LOG_DBG("done");
}

struct work1_context work1 = {
	WORK_INIT(work1_task),
};

/*
 * Timer based work to show system timer
 * 
 * could have just used scheduled work for delay.
 */

struct minute_work_context {
	struct k_work work;
	// Other parameters as needed
};

static void minute_work_task(struct k_work *work)
{
	struct minute_work_context __unused *context = CONTAINER_OF(work, struct minute_work_context, work);
	LOG_DBG("start");
	k_msleep(10000);
	LOG_DBG("done");
}

static struct minute_work_context minute_work = {
	WORK_INIT(minute_work_task),
};

// Timer ISR, called in interrupt context. Do as little as possible here
static void minute_timer_isr(struct k_timer *dummy)
{
	work_submit(minute_work);
}

static K_TIMER_DEFINE(minute_timer, minute_timer_isr, NULL);


/*
 * Settings
 */

static int m_settings_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg)
{
	const char *next;
	LOG_DBG("set %s", name);
	if (settings_name_steq(name, "foo", &next) && !next) {
		int rc = read_cb(cb_arg, &foo, sizeof(foo));
		LOG_INF("setting foo = %d (%d)", foo, rc);
		return 0;
	}
	return -1;
}

int m_settings_commit(void)
{
	// Warning: Initial commit is early during startup while main is initializing.
	if (module_state == M_STATE_UNINITIALIZED)
		return 0;

	LOG_INF("Settings committed");
	// Do what is needed to apply/activate new settings
	return -1;
}

// _get is for runtime settings. Not used (NULL)
// _export is for settings_save() of runtime settings. Not used (NULL)
SETTINGS_STATIC_HANDLER_DEFINE(MODULE, STRINGIFY(MODULE), NULL, m_settings_set, m_settings_commit, NULL);

/*
 * CAF Module
 */

static void module_initialize(void)
{
	LOG_DBG("initializing");
	#define K_WORK_MODULE_NAME \
		&(struct k_work_queue_config){.name=STRINGIFY(MODULE)}
	k_work_queue_start(&work_q, stack_area, K_THREAD_STACK_SIZEOF(stack_area),
					CONFIG_SYSTEM_WORKQUEUE_PRIORITY, K_WORK_MODULE_NAME);
					 // TODO: Assign proper thread priority
	k_timer_start(&minute_timer, K_SECONDS(1), K_SECONDS(60));
	module_state = M_STATE_READY;
	module_set_state(MODULE_STATE_READY);
	LOG_DBG("initialized");
}

static bool handle_module_state_event(const struct module_state_event *evt)
{
	// Wait for dependent modules to initialize
	if (check_state(evt, MODULE_ID(main), MODULE_STATE_READY)) {
		// Initialize system parts
		module_initialize();
	}
	return false;
}

static bool app_event_handler(const struct app_event_header *aeh)
{
	if (is_module_state_event(aeh)) {
		return handle_module_state_event(cast_module_state_event(aeh));
	}

	/* Event not handled but subscribed. */
	__ASSERT_NO_MSG(false);

	return false;
}

APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, module_state_event);


/*
 * Shell integration
 */

#if CONFIG_SHELL
#include <zephyr/shell/shell.h>

static int sh_echo(const struct shell *shell, size_t argc, char **argv)
{
	for (int i = 0; i < argc; i++) {
		shell_fprintf(shell, SHELL_NORMAL, "echo '%s'\n", argv[i]);    
	}
	return 0;
}

static int sh_work(const struct shell *shell, size_t argc, char **argv)
{
	work_submit(work1);
	return 0;
}

// This shows the dangers of submitting the same work more than once
static int sh_workx3(const struct shell *shell, size_t argc, char **argv)
{
	work_submit(work1);
	work_submit(work1);
	work_submit(work1);
	return 0;
}

static int sh_foo(const struct shell *shell, size_t argc, char **argv)
{
	// Show current setting of foo
	shell_fprintf(shell, SHELL_NORMAL, "foo = %d\n", foo);
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(module_shell,
	SHELL_CMD_ARG(echo, NULL, "Echo", sh_echo, 0, 0),
	SHELL_CMD_ARG(work, NULL, "work", sh_work, 0, 0),
	SHELL_CMD_ARG(workx3, NULL, "work x3", sh_workx3, 0, 0),
	SHELL_CMD_ARG(foo, NULL, "Foo", sh_foo, 0, 0),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(MODULE, &module_shell, "Test 1 commands", NULL);

#endif
