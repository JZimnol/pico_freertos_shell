# Raspberry Pi Pico FreeRTOS Shell

This FreeRTOS module allows you to add an interactive shell with custom commands
to your application.

<p align="center">
  <img src=docs/example.gif />
</p>

## Basic usage

**Basic usage can be found [here](https://github.com/JZimnol/pico_shell_example)**.

## Notes

- For now, only the `VT100`'s terminal escape sequences are supported. Supported
  escape sequences are:
  - colored output (use terminal that supports colored output, e.g.
    `picocom/putty`)
  - left arrow
  - right arrow
  - down arrow (if command history is enabled)
  - up arrow (if command history is enabled)
  - delete
  - home
  - end
- The following special characters are supported:
  - tab (autocompletion for commands/subcommands)
  - ctrl+c
  - ctrl+h / backspace
  - ctrl+d / enter
- A few compile time options have been defined. Please refer to the main
  [CMakeLists.txt](CMakeLists.txt) file for a list of available compile time
  CMake options.
- The module consist of two separate FreeRTOS tasks: `shell input/output and
  message buffering task` and `command handler task`. That's why if a few
  messages are printed using printf/puts inside any command handler, they MIGHT
  be separated by some other application's messages.
- This module has been tested for `pico-sdk == 1.5.1`
  - may not work with other versions, dunno, didn't test it
- Probably not every corner case has been handled regarding printing messages to
  serial output.
  **Creating both pull requests and issues on GitHub is welcomed!**

# Example

## File structure

Assume the following file structure:

```
your_project/
├── CMakeLists.txt
├── deps/
|   ├── pico_freertos_shell/
|   |   ├── cmake_preinit/
|   |   ├── include/
|   |   ├── ...
|   ...
├── FreeRTOS_Kernel_import.cmake
├── main.c
└── pico_sdk_import.cmake
```

The following files should have the following contents:

### your_project/CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.13)

# set pico board (works both for pico and pico_w)
set(PICO_BOARD pico_w)

# initialize SDK based on PICO_SDK_PATH
# initialize FreeRTOS kernel based on FREERTOS_KERNEL_PATH
# note: this must happen before project()
set(PICO_SDK_PATH ...)
set(FREERTOS_KERNEL_PATH ...)
include(pico_sdk_import.cmake)
include(FreeRTOS_Kernel_import.cmake)

# initialize Pico FreeRTOS Shell based on PICO_FREERTOS_SHELL_PATH
# note: this must happen before project() and pico_sdk_init()
set(PICO_FREERTOS_SHELL_PATH "${CMAKE_CURRENT_LIST_DIR}/deps/pico_freertos_shell"
    CACHE PATH "Path to the pico_freertos_shell root directory")
include(${PICO_FREERTOS_SHELL_PATH}/pico_freertos_shell_import.cmake)

project(your_app)
pico_sdk_init()

# create FreeRTOS target if needed

# add subdirectory containing Pico FreeRTOS Shell sources
add_subdirectory(${PICO_FREERTOS_SHELL_PATH})

add_executable(your_app
               main.c)
target_link_libraries(your_app PUBLIC
                      pico_stdlib
                      # link with FreeRTOS here e.g.
                      FreeRTOS
                      pico_freertos_shell_lib)

# link the Pico FreeRTOS Shell modue with the FreeRTOS library, e.g.
pico_freertos_shell_link_freertos(FreeRTOS)

# enable serial output (e.g. USB or UART - no difference for app)
pico_enable_stdio_usb(example_app 1)
pico_enable_stdio_uart(example_app 0)

# rest of the file if needed...
```

### your_project/main.c

```c
#include <stdlib.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <task.h>

#include <pico_freertos_shell/commands.h>
#include <pico_freertos_shell/init.h>

/* other includes if needed */

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* add custom commands with handlers if needed */

static int m_some_variable;

static void variable_set_cmd_handler(int argc, char **argv) {
    if (argc != 1) {
        printf("Invalid number of arguments\n");
        return;
    }

    m_some_variable = atoi(argv[0]);
    printf("Set variable to %d\n", m_some_variable);
}

static void variable_read_cmd_handler(int argc, char **argv) {
    if (argc != 0) {
        printf("Invalid number of arguments\n");
        return;
    }

    printf("Variable is %d\n", m_some_variable);
}

static void helloworld_cmd_handler(int argc, char **argv) {
    printf("Hello World!\n");
    if (argc > 0) {
        printf("My arguments:\n");
        for (size_t i = 0; i < argc; i++) {
            printf("argv[%d] = %s\n", i, argv[i]);
        }
    }
}

static const pfs_command_t VARIABLE_SUBCOMMANDS[] = {
        PFS_COMMAND_INITIALIZER(set,
                                "set the variable to a value",
                                PFS_COMMAND_HANDLER(variable_set_cmd_handler)),
        PFS_COMMAND_INITIALIZER(read,
                                "read the value of the variable",
                                PFS_COMMAND_HANDLER(
                                        variable_read_cmd_handler))};

static const pfs_command_t SHELL_COMMANDS[] = {
        PFS_COMMAND_INITIALIZER(
                variable,
                "set or read a variable",
                PFS_SUBCOMMANDS(VARIABLE_SUBCOMMANDS,
                                ARRAY_SIZE(VARIABLE_SUBCOMMANDS))),
        PFS_COMMAND_INITIALIZER(helloworld,
                                "prints 'Hello World!' and a list of arguments",
                                PFS_COMMAND_HANDLER(helloworld_cmd_handler))};

static int example_app_register_commands(void) {
    return pfs_commands_register(SHELL_COMMANDS, ARRAY_SIZE(SHELL_COMMANDS));
}

#define MAIN_APP_STACK_SIZE (1000U)
static StackType_t main_app_task_stack[MAIN_APP_STACK_SIZE];
static StaticTask_t main_app_task_buffer;

static void main_app_task(__unused void *params) {
    int x = 0;
    while (1) {
        printf("This is a message using printf: %d\n", x++);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        puts("This is a message using puts");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

int main() {
    stdio_init_all();

    if (example_app_register_commands()) {
        printf("Failed to register commands\n");
    }

    pfs_init();

    xTaskCreateStatic(main_app_task, "MainAppTask", MAIN_APP_STACK_SIZE, NULL,
                      tskIDLE_PRIORITY + 2UL, main_app_task_stack,
                      &main_app_task_buffer);

    vTaskStartScheduler();
}
```

## Compiling and running

### Compiling

Create the build directory and build the project within it.

```shell
# these commands may vary depending on the OS
mkdir build/
cd build
cmake .. && make -j
```

**NOTE:** please refer to the main [CMakeLists.txt](CMakeLists.txt) file for a
list of available compile time CMake options.

### Running

Flash the board using generated binary file. Open your serial terminal (e.g.
minicom, picocom, putty) and a shell prompt should appear. Type `help` or
`helptree` for a list of available commads. Try some example commands:
- `helloworld arg1 "arg with a space"`
- `helptree variable`
- `variable read`
- `variable set 123`
