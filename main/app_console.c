#if 1

#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"

#include "esp_vfs_fat.h"

#include "app_console.h"



static const char* TAG = "CMD";

#define CONFIG_STORE_HISTORY 0
#define CONFIG_CONSOLE_UART_NUM 0

#if CONFIG_STORE_HISTORY

#define MOUNT_PATH "/data"
#define HISTORY_PATH MOUNT_PATH "/history.txt"

static void initialize_filesystem()
{
    static wl_handle_t wl_handle;
    const esp_vfs_fat_mount_config_t mount_config = {
            .max_files = 4,
            .format_if_mount_failed = true
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount(MOUNT_PATH, "storage", &mount_config, &wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (0x%x)", err);
        return;
    }
}
#endif // CONFIG_STORE_HISTORY

static int free_mem(int argc, char** argv)
{
    printf("Free: %d\n", esp_get_free_heap_size());
#if 0
    printf("max heap: %d\n", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    printf("min heap: %d\n", esp_get_minimum_free_heap_size());

    if (argc > 2) {
        heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
    }
#endif
    return 0;
}

static void register_free()
{
    const esp_console_cmd_t cmd = {
        .command = "free",
        .help = "Get the total size of heap memory available",
        .hint = NULL,
        .func = &free_mem,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

#include "app_ac_dev.h"
static int restart(int argc, char** argv)
{
    ESP_LOGI(__func__, "Restarting");
    int addr = 1;
    int dev = 0;
    int mode = 0;
    int speed = 0;
    int temp = 0;
    int swing = 0;
    int set = 0;

    assert(4 == argc);
    //uint16_t rw_data = 0;
    dev = atoi(argv[2]);
    int smode = atoi(argv[3]);
    int stemp = atoi(argv[3]);
    int ison = atoi(argv[3]);
    speed = atoi(argv[3]);
    int cmd = atoi(argv[1]);
    int inout = atoi(argv[2]);
    int acoffset = atoi(argv[3]);

    switch(cmd){
        case 0:
        app_ac_set_power(addr, dev, ison?1:0);
        break;
        case 1:
        app_ac_set_temp(addr, dev, stemp);
        break;
        case 2:
        app_ac_set_mode(addr, dev, smode);
        break;
        case 3:
        app_ac_set_speed(addr, dev, speed);
        break;
        case 4:
        app_ac_set_auxisetting(addr, dev, 0x01<<ison, 0);
        break;
        case 5:
        app_ac_set_speed(addr, dev, ison?0x11:0);

        break;
        case 6:
        
        app_ac_get_param(addr, dev, BRAND_MIDEA, &mode, &temp, &speed);
        
        ESP_LOGI(TAG, "app_ac_get_param_ex :: mode:%d | temp = %d | speed = %d",
            mode, temp, speed);
        break;
        case 7:
        
        app_ac_get_param_ex(1, dev, BRAND_MIDEA, &mode, &temp, &speed, &swing, &set);
        
        ESP_LOGI(TAG, "app_ac_get_param_ex :: mode:%d | temp = %d | speed = %d | swing = %d | set = %d", 
        mode, temp, speed, swing, set);

        break;
        default:
        read_all_stuff_online_dev(inout, acoffset);
        break;
    }
    //app_ac_get_param_ex(1, inout, BRAND_MIDEA, &mode, &temp, &speed, &swing, &set);
    //ESP_LOGI(TAG, "app_ac_get_param_ex :: mode:%d | temp = %d | speed = %d | swing = %d | set = %d", 
    //mode, temp, speed, swing, set);
    //esp_restart();

    return 0;
}

static void register_restart()
{
    const esp_console_cmd_t cmd = {
        .command = "restart",
        .help = "Restart the program",
        .hint = NULL,
        .func = &restart,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

void app_console_register_default()
{
    register_free();
    register_restart();

}

static void initialize_console()
{
    /* Disable buffering on stdin and stdout */
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(CONFIG_CONSOLE_UART_NUM,
            256, 0, 0, NULL, 0) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_CONSOLE_UART_NUM);

    /* Initialize the console */
    esp_console_config_t console_config = {
            .max_cmdline_args = 8,
            .max_cmdline_length = 512,
#if CONFIG_LOG_COLORS
            .hint_color = atoi(LOG_COLOR_CYAN)
#endif
    };
    ESP_ERROR_CHECK( esp_console_init(&console_config) );

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);

#if CONFIG_STORE_HISTORY
    /* Load command history from filesystem */
    linenoiseHistoryLoad(HISTORY_PATH);
#endif
}

void app_console_init()
{
#if CONFIG_STORE_HISTORY
    initialize_filesystem();
#endif

    initialize_console();

    /* Register commands */
    esp_console_register_help_command();
	
	/* Register command commands */
	app_console_register_default();
}

void app_console_deinit()
{
	
}

void app_console_exec(char * args)
{
    int ret;
    esp_err_t err = esp_console_run(args, &ret);
    if (err == ESP_ERR_NOT_FOUND) {
        printf("Unrecognized command\n");
    } else if (err == ESP_OK && ret != ESP_OK) {
        printf("Command returned non-zero error code: 0x%x\n", ret);
    } else if (err != ESP_OK) {
        printf("Internal error: 0x%x\n", err);
    }
}

int app_console_run_once(const char* prompt)
{
    /* Get a line using linenoise.
     * The line is returned when ENTER is pressed.
     */
    char *line = linenoise(prompt);
    if (line == NULL) { /* Ignore empty lines */
        //WELINK.IO Add.
        return -1;
    }

    /* Add the command to the history */
    linenoiseHistoryAdd(line);
#if CONFIG_STORE_HISTORY
    /* Save command history to filesystem */
    linenoiseHistorySave(HISTORY_PATH);
#endif

    /* Try to run the command */
    int ret;
    esp_err_t err = esp_console_run(line, &ret);
    if (err == ESP_ERR_NOT_FOUND) {
//        if (app_uart_exe(line) < 0) {
            printf("Unrecognized command\n");
//        }
    } else if (err == ESP_OK && ret != ESP_OK) {
        printf("Command returned non-zero error code: 0x%x\n", ret);
    } else if (err != ESP_OK) {
        printf("Internal error: 0x%x\n", err);
    }
    /* linenoise allocates line buffer on the heap, so need to free it */
    linenoiseFree(line);

    return 0;
}

void app_console_run()
{
    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    const char* prompt = LOG_COLOR_I "CMD> " LOG_RESET_COLOR;

    printf("\n"
           "Type 'help' to get the list of commands.\n");

    /* Figure out if the terminal supports escape sequences */
    int probe_status = linenoiseProbe();
    if (probe_status) { /* zero indicates success */
        //printf("\n"
        //       "Your terminal application does not support escape sequences.\n"
        //       "Line editing and history features are disabled.\n"
        //       "On Windows, try using Putty instead.\n");
        linenoiseSetDumbMode(1);
#if CONFIG_LOG_COLORS
        /* Since the terminal doesn't support escape sequences,
         * don't use color codes in the prompt.
         */
        prompt = "CMD> ";
#endif //CONFIG_LOG_COLORS
    }

    /* Main loop */
    while(true) {
        /* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
        char* line = linenoise(prompt);
        if (line == NULL) { /* Ignore empty lines */
            //WELINK.IO Add.
            vTaskDelay(10);
            continue;
        }
        /* Add the command to the history */
        linenoiseHistoryAdd(line);
#if CONFIG_STORE_HISTORY
        /* Save command history to filesystem */
        linenoiseHistorySave(HISTORY_PATH);
#endif

        /* Try to run the command */
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND) {
//			if(app_uart_exe(line) < 0) {
				printf("Unrecognized command\n");
//			}
        } else if (err == ESP_OK && ret != ESP_OK) {
            printf("Command returned non-zero error code: 0x%x\n", ret);
        } else if (err != ESP_OK) {
            printf("Internal error: 0x%x\n", err);
        }
        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }
}

#endif
