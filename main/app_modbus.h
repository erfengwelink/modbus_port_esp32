#ifndef _APP_MODBUS_H_
#define _APP_MODBUS_H_

#include "mb.h"
#include "mbport.h"
#include "mbutils.h"
#include "app_ac_dev.h"

/* -----------------------Master Defines -------------------------------------*/
#define M_DISCRETE_INPUT_START 10000
#define M_DISCRETE_INPUT_NDISCRETES 128*16
#define M_COIL_START 0
#define M_COIL_NCOILS 128*16
#define M_REG_INPUT_START 30000
#define M_REG_INPUT_NREGS 32*16
#define M_REG_HOLDING_START 40000
#define M_REG_HOLDING_NREGS 32*16
/* master mode: holding register's all address */
#define M_HD_RESERVE 0
/* master mode: input register's all address */
#define M_IN_RESERVE 0
/* master mode: coil's all address */
#define M_CO_RESERVE 0
/* master mode: discrete's all address */
#define M_DI_RESERVE 0

#ifdef __cplusplus
extern "C" {
#endif

int get_p_reg_in_buf(int i);

int get_p_hold_buf(int i);

int get_p_coil_buf(int i);

int get_p_disc_buf(int i);

void SysMonitor(void *parameter);

void modebus_task(void *parameter);

//  0x01
int app_coil_read(const uint8_t addr, const brandType_t brand, const int func, const int index, const int num); //single or multi-coils

//  0x02
int app_coil_discrete_input_read(const uint8_t addr, const brandType_t brand, const int func, const int index, const int num); //single or multi-coils

//  0x03
int app_holding_register_read(const uint8_t addr, const brandType_t brand, const int func, const int index, const int num); //single or multi-coils

//  0x04
int app_input_register_read(const uint8_t addr, const brandType_t brand, const int func, const int index, const int num); //single or multi-coils

//  0x05
int app_coil_single_write(const uint8_t addr, const brandType_t brand, const int func, const int index, const USHORT sendData); //single

//  0x06
int app_register_single_write(const uint8_t addr, const brandType_t brand, const int func, const int index, const USHORT sendData); //single

//  0x0f
int app_coil_multi_write(const uint8_t addr, const brandType_t brand, const int func, const int index, const int num, UCHAR *sendData); //single or multi-coils

// 0x10
int app_register_multi_write(const uint8_t addr, const brandType_t brand, const int func, const int index, const int num, USHORT *sendData); //single or multi-coils

// 0x17
int app_register_multi_write_read(const uint8_t addr, const brandType_t brand, const int func, const int index, const int num, USHORT *sendData); //single or multi-coils

#ifdef __cplusplus
}
#endif

#endif
