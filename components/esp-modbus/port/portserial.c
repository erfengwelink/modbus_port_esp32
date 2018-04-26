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
 * File: $Id: portserial.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "mbrtu.h"
//#include "mbconfig.h"

#include <string.h>
#include "driver/uart.h"
#include "soc/dport_access.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

static const char *TAG = "FREE_MODBUS_SERIAL";

#define MB_UART UART_NUM_2
#define MB_UART_TX_PIN 33
#define MB_UART_RX_PIN 34

#define MB_UART_BUF_SIZE (1024)

static QueueHandle_t mb_uart_queue;
static uint8_t *mbDataP = NULL;

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);

static uint8_t mb_serial_read(uint8_t *data, uint8_t size)
{
    mbDataP = data;
    uint8_t remaindBytes = size;
    while (remaindBytes--)
    {
        prvvUARTRxISR();
    };
    mbDataP = NULL;
    return 0;
}

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t *dtmp = (uint8_t *)malloc(MB_UART_BUF_SIZE);
    for (;;)
    {
        //Waiting for UART event.
        if (xQueueReceive(mb_uart_queue, (void *)&event, (portTickType)portMAX_DELAY))
        {
            bzero(dtmp, MB_UART_BUF_SIZE);
            //ESP_LOGI(TAG, "uart[%d] event:", MB_UART);
            switch (event.type)
            {
            //Event of UART receving data
            /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
            case UART_DATA:
                //ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                uart_read_bytes(MB_UART, dtmp, event.size, portMAX_DELAY);

#if 0
                    ESP_LOGI(TAG, "[DATA EVT]:");
                    for(int i = 0; i < event.size; i++)
                        printf("|%02x", dtmp[i]);
                    printf("\r\n");
#endif

                //mb_buffer_in(dtmp, (uint8_t)event.size);
                //virtual_serial_read();
                mb_serial_read(dtmp, event.size);

                uart_flush_input(MB_UART);

                //    ESP_LOGI(TAG, "|%02X", dtmp[i]);
                //uart_write_bytes(MB_UART, (const char*) dtmp, event.size);
                //uart_write_bytes(MB_UART, (const char*)"qqqq", 4);
                break;
            //Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");
                // If fifo overflow happened, you should consider adding flow control for your application.
                // The ISR has already reset the rx FIFO,
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(MB_UART);
                xQueueReset(mb_uart_queue);
                break;
            //Event of UART ring buffer full
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                // If buffer full happened, you should consider encreasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(MB_UART);
                xQueueReset(mb_uart_queue);
                break;
            //Event of UART RX break detected
            case UART_BREAK:
                ESP_LOGI(TAG, "uart rx break");
                break;
            //Event of UART parity check error
            case UART_PARITY_ERR:
                ESP_LOGI(TAG, "uart parity error");
                break;
            //Event of UART frame error
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "uart frame error");
                break;
            //UART_PATTERN_DET
            case UART_PATTERN_DET:
                uart_get_buffered_data_len(MB_UART, &buffered_size);
                int pos = uart_pattern_pop_pos(MB_UART);
                ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                if (pos == -1)
                {
                    // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                    // record the position. We should set a larger queue size.
                    // As an example, we directly flush the rx buffer here.
                    uart_flush_input(MB_UART);
                }
                else
                {
                    /*
                        uart_read_bytes(MB_UART, dtmp, pos, 100 / portTICK_PERIOD_MS);
                        uint8_t pat[PATTERN_CHR_NUM + 1];
                        memset(pat, 0, sizeof(pat));
                        uart_read_bytes(MB_UART, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                        ESP_LOGI(TAG, "read data: %s", dtmp);
                        ESP_LOGI(TAG, "read pat : %s", pat);
                        */
                }
                break;
            //Others
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

/* ----------------------- Start implementation -----------------------------*/
void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
    vMBPortEnterCritical();
    UCHAR len = get_s_usLength() + 1;

    //ESP_LOGI(TAG, "get_usLength:%d xRxEnable: %s | xTxEnable: %s", len, xRxEnable?"on":"off",xTxEnable?"on":"off");

    if (xTxEnable && !xRxEnable)
    {
        for (int i = 0; i < len; i++)
            prvvUARTTxReadyISR();
    }
    else if (!xTxEnable && xRxEnable)
    {
        //set_eRcvState(2);     //STATE_M_RX_RCV
    }
    else
    {
    }
    vMBPortExitCritical();
}

BOOL xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    int lDataBit = UART_DATA_8_BITS;
    int lParity = UART_PARITY_DISABLE;
    switch (ucDataBits)
    {
    case 5:
        lDataBit = UART_DATA_5_BITS;
        break;
    case 6:
        lDataBit = UART_DATA_6_BITS;
        break;
    case 7:
        lDataBit = UART_DATA_7_BITS;
        break;
    case 8:
        lDataBit = UART_DATA_8_BITS;
        break;
    default:
        break;
    }
    switch (eParity)
    {
    case MB_PAR_NONE:
        lParity = UART_PARITY_DISABLE;
        break;
    case MB_PAR_EVEN:
        lParity = UART_PARITY_EVEN;
        break;
    case MB_PAR_ODD:
        lParity = UART_PARITY_ODD;
        break;
    default:
        break;
    }
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = ulBaudRate, //115200,
        .data_bits = lDataBit,   //UART_DATA_8_BITS,
        .parity = lParity,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    uart_param_config(MB_UART, &uart_config);

    //Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);
    //Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(MB_UART, MB_UART_TX_PIN, MB_UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    //Install UART driver, and get the queue.
    uart_driver_install(MB_UART, MB_UART_BUF_SIZE * 2, MB_UART_BUF_SIZE * 2, 20, &mb_uart_queue, 0);

    //Set uart pattern detect function.
    //uart_enable_pattern_det_intr(MB_UART, '+', PATTERN_CHR_NUM, 10000, 10, 10);
    //Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(MB_UART, 20);

    //Create a task to handler UART event from ISR
    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL);

    return TRUE;
}

static INLINE BOOL
xMBPortSerialPutByte(CHAR ucByte) // const
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
    uart_write_bytes(MB_UART, &ucByte, 1);
    return TRUE;
}

static INLINE BOOL
xMBPortSerialGetByte(CHAR *pucByte)
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
    *pucByte = (CHAR) * (mbDataP++);
    return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR(void)
{
    pxMBMasterFrameCBTransmitterEmpty();
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR(void)
{
    pxMBMasterFrameCBByteReceived();
}

BOOL xMBMasterPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
                             eMBParity eParity)
{
    return xMBPortSerialInit(ucPORT, ulBaudRate, ucDataBits, eParity);
}

void vMBMasterPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    vMBPortEnterCritical();
    UCHAR len = get_s_usLength() + 1;

    //ESP_LOGI(TAG, "get_usLength:%d xRxEnable: %s | xTxEnable: %s", len, xRxEnable?"on":"off",xTxEnable?"on":"off");

    if (xTxEnable && !xRxEnable)
    {
        for (int i = 0; i < len; i++)
            prvvUARTTxReadyISR();
    }
    else if (!xTxEnable && xRxEnable)
    {
        //set_eRcvState(2);//STATE_M_RX_RCV
    }
    else
    {
    }
    vMBPortExitCritical();
}

BOOL xMBMasterPortSerialGetByte(CHAR *pucByte)
{
    return xMBPortSerialGetByte(pucByte);
}

BOOL xMBMasterPortSerialPutByte(const CHAR ucByte)
{
    return xMBPortSerialPutByte(ucByte);
}