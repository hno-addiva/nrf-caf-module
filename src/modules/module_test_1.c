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
 */


#define MODULE test_1

#include <zephyr/kernel.h>
#include <caf/events/module_state_event.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(MODULE, LOG_LEVEL_DBG);

#include "test_1_def.h"

// Work queue
static K_THREAD_STACK_DEFINE(stack_area, 2048);
static struct k_work_q work_q;

// Every work item should have a context struct, where k_work work is a member.
// this allows for other data to be passed to the work handler. Use macros below
// to avoid referencing the k_work item maually.
/*
struct somework_context {
	struct k_work work;
	// Other parameters as needed
	...
} somework;

static void somework_task(struct k_work *work)
{
	struct somework_context *context = CONTAINER_OF(work, struct somework_context, work);
	LOG_DBG("somework_task");
	...

}

static struct somework_context somework = {
	.work = Z_WORK_INITIALIZER(some_task),
	// Other static initializations if needed
};

	work_init(somework, somework_task);

	work_submit(somework);

*/

#define work_submit(context) \
	k_work_submit_to_queue(&work_q, &context.work)
#define work_init(context, fn) \
	k_work_init(&context.work, fn)

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
	.work = Z_WORK_INITIALIZER(work1_task),
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
	.work = Z_WORK_INITIALIZER(minute_work_task),
};

// Timer ISR, called in interrupt context. Do as little as possible here
static void minute_timer_isr(struct k_timer *dummy)
{
	work_submit(minute_work);
}

static K_TIMER_DEFINE(minute_timer, minute_timer_isr, NULL);



static void module_initialize(void)
{
	LOG_DBG("initializing");
	#define K_WORK_NAME(_name) \
		&(struct k_work_queue_config){.name=_name}
	k_work_queue_start(&work_q, stack_area, K_THREAD_STACK_SIZEOF(stack_area),
					CONFIG_SYSTEM_WORKQUEUE_PRIORITY, K_WORK_NAME("test_1"));
					 // TODO: Assign proper thread priority
	k_timer_start(&minute_timer, K_SECONDS(1), K_SECONDS(60));
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


// Shell integration
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

SHELL_STATIC_SUBCMD_SET_CREATE(module_shell,
	SHELL_CMD_ARG(echo, NULL, "Echo", sh_echo, 0, 0),
	SHELL_CMD_ARG(work, NULL, "work", sh_work, 0, 0),
	SHELL_CMD_ARG(workx3, NULL, "work x3", sh_workx3, 0, 0),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(MODULE, &module_shell, "Test 1 commands", NULL);

#endif
