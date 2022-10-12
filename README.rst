.. _nRF CAF Module:

nRF CAF Module "test_1"
#######################

Overview
********

A simple CAF module with worker thread, timer event and shell integratin

Building and Running
********************

Nothing special

Sample Output
=============

~~~
*** Booting Zephyr OS build v3.1.99-ncs1  ***


[00:00:00.401,519] <inf> main: Starting
[00:00:00.401,550] <inf> main: Device ID
                               fa 70 c6 79 38 f1 5c 6f                          |.p.y8.\o
[00:00:00.401,611] <inf> app_event_manager: e:module_state_event module:main state:READY
[00:00:00.401,611] <dbg> test_1: module_initialize: initializing
[00:00:00.401,733] <dbg> test_1: module_initialize: initialized
[00:00:00.401,763] <inf> app_event_manager: e:module_state_event module:test_1 state:READY
[00:00:00.401,824] <inf> main: End
[00:00:01.401,733] <dbg> test_1: minute_work_task: start
[00:00:11.401,855] <dbg> test_1: minute_work_task: done
uart:~$ help
Please press the <Tab> button to see all available commands.
You can also use the <Tab> button to prompt or auto-complete all commands or its subcommands.
You can try to call commands with <-h> or <--help> parameter for more information.

Shell supports following meta-keys:
  Ctrl + (a key from: abcdefklnpuw)
  Alt  + (a key from: bf)
Please refer to shell documentation for more details.

Available commands:
  app_event_manager  :Application Event Manager commands
  clear              :Clear screen.
  device             :Device commands
  devmem             :Read/write physical memory"devmem address [width [value]]
  help               :Prints the help message.
  history            :Command history.
  hwinfo             :HWINFO commands
  kernel             :Kernel commands
  log                :Commands for controlling logger
  nrf_clock_control  :Clock control commands
  resize             :Console gets terminal screen size or assumes default in
                      case the readout fails. It must be executed after each
                      terminal width change to ensure correct text display.
  shell              :Useful, not Unix-like shell commands.
  test_1             :Test 1 commands
uart:~$ test_1
test_1 - Test 1 commands
Subcommands:
  echo  :Echo
  work  :work
[...]
[00:01:01.401,733] <dbg> test_1: minute_work_task: start
[00:01:11.401,855] <dbg> test_1: minute_work_task: done
[...]
uart:~$ test_1 work
[00:01:30.646,545] <dbg> test_1: work1_task: start
[00:01:31.646,697] <dbg> test_1: work1_task: done
[...]
[00:02:01.401,733] <dbg> test_1: minute_work_task: start
[00:02:11.401,855] <dbg> test_1: minute_work_task: done
~~~
