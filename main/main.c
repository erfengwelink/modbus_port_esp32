#include "app_ac_dev.h"
#include "app_modbus.h"
#include "app_console.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main()
{

    xTaskCreate(modebus_task,"modebus_task",1024*80,NULL,5,NULL);

    app_console_init();
    app_console_run();
}