/*
 * Copyright (c) 2022 Addiva Elektronik AB
 *
 * SPDX-License-Identifier: TBD
 */

#include <zephyr/kernel.h>

#define MODULE test_1
#include <caf/events/module_state_event.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MODULE, LOG_LEVEL_DBG);

#include "module_test_1.h"
#include "test_1_def.h"

// Work queue
static K_THREAD_STACK_DEFINE(stack_area, 2048);
static struct k_work_q work_q;

// Every work item should have a context struct, where k_work work is a member.
// this allows for other data to be passed to the work handler. Use macros below
// to avoid referencing the k_work item maually.
/*
static struct somework_context {
        struct k_work work;
        ...
} work1;

static void somework_task(struct k_work *work)
{
    struct somework_context *context = CONTAINER_OF(work, struct somework_context, work);
    LOG_DBG("somework_task");
    ...

}
    work_init(somework, womework_task);

    work_submit(somework);

*/

#define work_submit(context) \
    k_work_submit_to_queue(&work_q, &context.work)
#define work_init(context, fn) \
    k_work_init(&context.work, fn)

static struct work1_context {
        struct k_work work;
} work1;

static void work1_task(struct k_work *work)
{
    struct work1_context __unused *context = CONTAINER_OF(work, struct work1_context, work);
    LOG_DBG("work1 task");
 }

// Timer based work to show system timer, could have just used scheduled
//  work for delay.
static struct minute_work_context {
    struct k_work work;
} minute_work;

static void minute_work_task(struct k_work *work)
{
    struct minute_work_context __unused *context = CONTAINER_OF(work, struct minute_work_context, work);
    LOG_DBG("minute work task");
// Timer ISR, called in interrupt context. Do as little as possible here
static void minute_timer_isr(struct k_timer *dummy)
{
    work_submit(minute_work);
}

static K_TIMER_DEFINE(minute_timer, minute_timer_isr, NULL);


#define K_WORK_NAME(_name) \
    &(struct k_work_queue_config){.name=_name}    
static void module_initialize(void)
{
    LOG_DBG("initialize");
    k_work_queue_start(&work_q, stack_area, K_THREAD_STACK_SIZEOF(stack_area),
                    CONFIG_SYSTEM_WORKQUEUE_PRIORITY, K_WORK_NAME("test_1"));
                     // TODO: Assign proper priority
    work_init(work1, work1_task);
    work_init(minute_work, minute_work_task);
    k_timer_start(&minute_timer, K_SECONDS(1), K_SECONDS(60));
    module_set_state(MODULE_STATE_READY);
    LOG_DBG("initialized");
}

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
        // Initialize system parts
        module_initialize();
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

SHELL_STATIC_SUBCMD_SET_CREATE(module_shell,
	SHELL_CMD_ARG(echo, NULL, "Echo", sh_echo, 0, 0),
	SHELL_CMD_ARG(work, NULL, "work", sh_work, 0, 0),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(MODULE, &module_shell, "Test 1 commands", NULL);

#endif
