#ifndef __APP_CONSOLE_H_
#define __APP_CONSOLE_H_

#ifdef __cplusplus
extern "C" {
#endif

void app_console_init();
void app_console_deinit();

void app_console_exec(char *arg);

void app_console_run(void);
int  app_console_run_once(const char* prompt);

// Register system functions
void app_console_register_default();

#ifdef __cplusplus
}
#endif

#endif //__APP_CONSOLE_H_