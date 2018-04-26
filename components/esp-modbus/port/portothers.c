/* ----------------------- System includes ----------------------------------*/
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Modbus includes ----------------------------------*/
//#include <intrinsics.h>

/* ----------------------- Variables ----------------------------------------*/
static BOOL bIsWithinException = FALSE;
static portMUX_TYPE mb_mux = portMUX_INITIALIZER_UNLOCKED;

/* ----------------------- Start implementation -----------------------------*/

void vMBPortSetWithinException(BOOL bInException)
{
    bIsWithinException = bInException;
}

BOOL bMBPortIsWithinException(void)
{
    return bIsWithinException;
}

void vMBPortEnterCritical(void)
{
    taskENTER_CRITICAL(&mb_mux);
}

void vMBPortExitCritical(void)
{
    taskEXIT_CRITICAL(&mb_mux);
}

void vMBPortClose(void)
{
    extern void vMBPortSerialClose(void);
    extern void vMBPortTimerClose(void);
    extern void vMBPortEventClose(void);
    vMBPortSerialClose();
    vMBPortTimerClose();
    vMBPortEventClose();
}