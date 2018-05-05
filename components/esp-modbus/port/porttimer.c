/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: porttimer.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
//#include "mbconfig.h"

#include "soc/timer_group_struct.h"
#include "driver/timer.h"

#define TIMER_INTR_SEL TIMER_INTR_LEVEL                  // Timer level interrupt
#define TIMER_GROUP TIMER_GROUP_0                        // Test on timer group 0
#define TIMER_DIVIDER 2                                  // Hardware timer clock divider
#define TIMER_INTERVAL0_SEC (0.00005)                    // [50us interval] test interval for timer 0 [1000usec/1.0msec(0.001),100usec/0.1msec(0.0001),8.5usec/0.0085msec(0.00001)]
#define TIMER_CPU_CLOCK 160000000L                       // CPU Clock 160Mhz
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER)     // used to calculate counter value
#define TIMER_SCALE160 (TIMER_CPU_CLOCK / TIMER_DIVIDER) // used to calculate counter value

/* ----------------------- static functions ---------------------------------*/
//static void prvvTIMERExpiredISR( void );
static void prvvMasterTIMERExpiredISR(void *p);

/* ----------------------- Start implementation -----------------------------*/

static void init_timer_group_0(USHORT N50us)
{
    int timer_group = TIMER_GROUP;
    int timer_idx = TIMER_0;

    // Configure timer
    timer_config_t config;
    config.alarm_en = 1;
    config.auto_reload = 1;
    config.counter_dir = TIMER_COUNT_UP;
    config.divider = TIMER_DIVIDER;
    config.intr_type = TIMER_INTR_SEL;
    config.counter_en = TIMER_PAUSE;
    timer_init(timer_group, timer_idx, &config);

    timer_pause(timer_group, timer_idx);
    timer_set_alarm_value(timer_group, timer_idx, N50us * TIMER_INTERVAL0_SEC * TIMER_SCALE);

    timer_enable_intr(timer_group, timer_idx);
    timer_isr_register(timer_group, timer_idx,
                       prvvMasterTIMERExpiredISR, (void *)timer_idx, ESP_INTR_FLAG_LOWMED, NULL);
}

static void timer_value_update(USHORT val)
{
    int timer_group = TIMER_GROUP;
    int timer_idx = TIMER_0;

    timer_pause(timer_group, timer_idx);
    timer_set_alarm_value(timer_group, timer_idx, val * TIMER_INTERVAL0_SEC * TIMER_SCALE);
}

BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
    init_timer_group_0(usTim1Timerout50us);
    return TRUE;
}

inline void
vMBPortTimersEnable()
{
    /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
    timer_start(TIMER_GROUP, TIMER_0);
}

inline void
vMBPortTimersDisable()
{
    /* Disable any pending timers. */
    timer_pause(TIMER_GROUP, TIMER_0);
}

/* Create an ISR which is called whenever the timer has expired. This function
 * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
 * the timer has expired.
 */
/*
static void prvvTIMERExpiredISR( void )
{
    TIMERG0.hw_timer[0].update 	= 1;
    TIMERG0.int_clr_timers.t0 	= 1;
    ( void )pxMBPortCBTimerExpired(  );
    TIMERG0.hw_timer[0].config.alarm_en = 1;
}
*/

/*  master device   APIs */
BOOL xMBMasterPortTimersInit(USHORT usTim1Timerout50us)
{
    return xMBPortTimersInit(usTim1Timerout50us);
}

INLINE
void vMBMasterPortTimersT35Enable()
{
    USHORT timer_tick = 2500;//1000;
    /* Set current timer mode, don't change it.*/
    vMBMasterSetCurTimerMode(MB_TMODE_T35);
    timer_value_update(timer_tick);
    vMBPortTimersEnable();
}

INLINE
void vMBMasterPortTimersDisable()
{
    vMBPortTimersDisable();
}

INLINE
void vMBMasterPortTimersConvertDelayEnable()
{
    USHORT timer_tick = MB_MASTER_DELAY_MS_CONVERT * 20;
    /* Set current timer mode, don't change it.*/
    vMBMasterSetCurTimerMode(MB_TMODE_CONVERT_DELAY);
    timer_value_update(timer_tick);
    vMBPortTimersEnable();
}

INLINE
void vMBMasterPortTimersRespondTimeoutEnable()
{
    USHORT timer_tick = MB_MASTER_TIMEOUT_MS_RESPOND * 20;
    /* Set current timer mode, don't change it.*/
    vMBMasterSetCurTimerMode(MB_TMODE_RESPOND_TIMEOUT);
    timer_value_update(timer_tick);
    vMBPortTimersEnable();
}

static void prvvMasterTIMERExpiredISR(void *p)
{
    TIMERG0.hw_timer[0].update = 1;
    TIMERG0.int_clr_timers.t0 = 1;
    (void)pxMBMasterPortCBTimerExpired();
    TIMERG0.hw_timer[0].config.alarm_en = 1;
}