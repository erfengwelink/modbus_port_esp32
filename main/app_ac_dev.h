#ifndef _APP_AC_DEV_H_
#define _APP_AC_DEV_H_

#include "mbconfig.h"
#include "stdint.h"

typedef enum ac_mode_t {
    ACMODE_FAN = 1,
    ACMODE_DRY,
    ACMODE_HEAT,
    ACMODE_COOL,
    ACMODE_AUTO,
    ACMODE_LOCK,
    //ACMODE_SLEEP,
    ACMODE_POWER,
    ACMODE_MAX
} ac_mode_t;

typedef enum ac_swing_t {
    AC_SWG_H_V_STOP = 0x00,
    AC_SWG_H_V_RUN = 0x11,
    AC_SWG_H_V_RESET = 0x22,
    AC_SWG_H_RUN = 0x10,
    AC_SWG_V_RUN = 0x01
} ac_swing_t;

typedef enum ac_speed_t {
    AC_SPEED_AUTO = 0,
    AC_SPEED_LOW,
    AC_SPPED_MID,
    AC_SPEED_HIGH,
    AC_SPEED_TURBO,
    AC_SPEED_MAX
} ac_speed_t;

typedef enum ac_setting_id_t {
    AC_SETTING_NULL = 0,
    AC_SETTING_SLEEP,
    AC_SETTING_MUTE,
    AC_SETTING_ECO,
    AC_SETTING_TURBO,
    AC_SETTING_MAX
} ac_setting_id_t;

typedef enum {
    BRAND_MIDEA,
    BRAND_HAIER,
} brandType_t;

typedef enum {
    INNER_DEV,
    MULTI_INNER_DEV,
    OUTER_DEV,
    MULTI_OUTER_DEV
} mideaDevType_t;

typedef enum {
    MIDEA_READ_COIL_REG = 0x01,
    MIDEA_READ_DISCRETE_REG = 0x02,
    MIDEA_READ_INPUT_REG = 0x04,
    MIDEA_WRITE_HOLDING_REG = 0x10
} mideaFuncType_t;

typedef enum {
    HAIER_READ_HOLDING_REG = 0x03, //read holding
    HAIER_WRITE_SINGLE_REG = 0x06, //write single
    HAIER_WRITE_HOLDING_REG = 0x10
} haierFuncType_t;

// read Descrete result
typedef enum {
    //MODE
    MD_CURR_WIND_MODE_R,
    MD_CURR_DRY_MODE_R,
    MD_CURR_HEAT_MODE_R,
    MD_CURR_COOL_MODE_R,
    MD_CURR_AUTO_MODE_R,
    MD_CURR_LOCK_MODE_R,
    MD_IGNOR_0,

    MD_PWR_OFF_R,
    MD_PWR_ON_R,
    //SPEED
    MD_CURR_HIGH_LEV_SPEED_R,
    MD_CURR_MID_LEV_SPEED_R,
    MD_CURR_LOW_LEV_SPEED_R,
    MD_CURR_WEAK_LEV_SPEED_R,
    MD_IGNOR_1,
    MD_IGNOR_2,
    MD_IGNOR_3,
    MD_CURR_FIXED_LEV_SPEED_R,
    //EXTRA0
    MD_COMPRESSOR,
    MD_OUTER_HIGH_LEV_SPEED_R,
    MD_OUTER_LOW_LEV_SPEED_R,
    MD_FOUR_WAY_VALVE_R,
    MD_CRANK_CASE_R,
    MD_OIL_RETURN_R,
    MD_IGNOR_4,
    MD_IGNOR_5,

    //auxi
    MD_ECON_RUN_OFF_R,
    MD_ECON_RUN_ON_R,

    MD_ELECTRIC_PAVING_OFF_R,
    MD_ELECTRIC_PAVING_ON_R,

    MD_SWING_OFF_R,
    MD_SWING_ON_R,

    MD_AERATION_OFF_R,
    MD_AERATION_ON_R,

    MD_FRESH_OFF_R,
    MD_FRESH_ON_R,

    MD_HUMIDIFY_OFF_R,
    MD_HUMIDIFY_ON_R,

    MD_ADD_OXYGEN_OFF_R,
    MD_ADD_OXYGEN_ON_R,

    MD_DRY_OFF_R,
    MD_DRY_ON_R,
    //EXTRA0
    MD_HORIZ_SWING_R,
    MD_ADD_WATER_R,
    MD_WET_PIT_PUMP_R,
    MD_IGNOR_6,
    MD_COOL_MODE_LOCK_R,
    MD_HEAT_MODE_LOCK_R,
    MD_CONTROLLER_LOCK_R,
    MD_TELECONTROLLER_LOCK_R,

    MD_COIL_PARAM_INVALID_R
} readDescreteResult_t;

// read input register
typedef enum {
    SYS_STATE = 1,
    DEV_INFO_1,
    DEV_INFO_2,
    USER_SET_TEMP,
    INDOOR_TEMP,
    EVAPORATOR_TUBE_TEMP, // evaporator tube temp
    EVAPORATOR_MID_TEMP,
    CONDENSER_TEMP, // condenser temp
    INPUT_REG_RESERVE_0,
    INPUT_REG_RESERVE_1,
    TIMER_POW_ON,
    TIMER_POW_OFF,
    DEV_PW,
    INPUT_REG_RESERVE_2,
    DEV_FAULT_CONDITON,
    DEV_PROTECT_CONDITOION,

    OUT_MACHINE_0_3_ONLINE,
    IN_MACHINE_0_15_ONLINE,
    IN_MACHINE_16_31_ONLINE,
    IN_MACHINE_32_47_ONLINE,
    IN_MACHINE_48_63_ONLINE,

    OUT_MACHINE_0_3_FAULT_CONDITON,
    OUT_MACHINE_0_3_RUN_CONDITON,

    IN_MACHINE_0_15_FAULT_CONDITON,
    IN_MACHINE_16_31_FAULT_CONDITON,
    IN_MACHINE_32_47_FAULT_CONDITON,
    IN_MACHINE_48_63_FAULT_CONDITON,

    IN_MACHINE_0_15_RUN_CONDITON,
    IN_MACHINE_16_31_RUN_CONDITON,
    IN_MACHINE_32_47_RUN_CONDITON,
    IN_MACHINE_48_63_RUN_CONDITON,
    INPUT_REG_MAX,
    INPUT_REG_INVALID,
} readMideaInputFunc_t;

typedef enum {
    W_COOL_SYS = 1,
    W_MODE_SET,
    W_FAN_SPEED_SET,
    W_TEMP_SET,
    W_TIMER_ON,
    W_TIMER_OFF,
    W_AUXI_FUNC,
    W_HOLD_REG_MAX = 32
} writeMideaHoldingFunc_t;

/* followings are haier func address */
/* W_ --> WRITE */
/* R_ --> READ  */
typedef enum {
    R_W_RUNNING_STATE = 1,
    R_W_RUNNING_MODE,
    W_WIND_SPEED_SET,
    W_WIND_SPEED_GET,
    R_W_TEMP_SET,
    R_W_SWING_LOCATION,
    R_W_CENTRAL_CONTROLLER,
    R_W_SWING_STATE,
    R_W_WIRE_CONTROL_PROHIBITE_SET,
    R_INNER_MACHINE_TEMP,
    R_FAULT_CODE,
    R_OUTER_ADDR
} readHaierHoldingFunc_t;

typedef enum {
    MD_W_ALL_COOL_SYS_OFF,
    MD_W_ALL_SUMMER_MODE_1_ON,
    MD_W_ALL_SUMMER_MODE_2_ON,
    MD_W_ALL_SUMMER_MODE_3_ON,
    MD_W_ALL_WINTER_MODE_1_ON,
    MD_W_ALL_WINTER_MODE_2_ON,
    MD_W_ALL_WINTER_MODE_3_ON
} MDcoolSysSwitch_t;

typedef enum {
    MD_WIND_MODE = 0x01 << 0,
    MD_DEHUMI_MODE = 0x01 << 1,
    MD_HEAT_MODE = 0x01 << 2,
    MD_COOL_MODE = 0x01 << 3,
    MD_AUTO_MODE = 0x01 << 4,
    MD_MODE_LOCK = 0x01 << 5,
    MD_POWER_STATE = 0x01 << 7,
    MD_MODE_MASK = MD_WIND_MODE | MD_DEHUMI_MODE | MD_HEAT_MODE |
                   MD_COOL_MODE | MD_AUTO_MODE | MD_MODE_LOCK
} MDModeSets_t;

typedef enum {
    MD_HIGH_WIND_SPEED = 0x01 << 0,
    MD_MID_WIND_SPEED = 0x01 << 1,
    MD_LOW_WIND_SPEED = 0x01 << 2,
    MD_WIND_SPEED_MASK = MD_HIGH_WIND_SPEED | MD_MID_WIND_SPEED | MD_LOW_WIND_SPEED
} MDWindSpeedSet_t;

//auxiliary func
typedef enum {
    MD_AUXI_ECONOMIC_RUN = 0x01 << 0,
    MD_AUXI_HEAT = 0x01 << 1,
    MD_AUXI_SWING = 0x01 << 2,
    MD_AUXI_AIR_FRESH = 0x01 << 3,
    MD_AUXI_MASK = MD_AUXI_ECONOMIC_RUN | MD_AUXI_HEAT |
                   MD_AUXI_SWING | MD_AUXI_AIR_FRESH
} MDAuxiSet_t;

typedef enum {
    MD_PWR_SET,
    MD_MODE_SET,
    MD_SPEED_SET,
    MD_TEMP_SET,
    MD_AUXI_SET,
    MD_DESCRETE_GET,
    MD_TEMP_GET,
    MD_PARAM_MAX,
} mideaReqParam_t;

typedef enum {
    RET_NONE = -1,
    RET_ALL_OK = 0,
    RET_RSP_FAIL,
    RET_CB_TIMEOUT,
    RET_ERR,
    RET_MAX
} ret_t;

#ifdef __cplusplus
extern "C" {
#endif

// AC open power 1/ON, 0/OFF
ret_t app_ac_set_power(const int addr, const int devIDs, const int ison);
// AC mode control ADDR:485 address, NUM: each room AC ID.
ret_t app_ac_set_mode(const int addr, const int devIDs, const ac_mode_t mode);
// AC temperature need 10, such as 26.0 degree 260, range is 160 ~ 300
ret_t app_ac_set_temp(const int addr, const int devIDs, const int temp);
// AC speed level
ret_t app_ac_set_speed(const int addr, const int devIDs, const ac_speed_t speed);
// AC swing H dir and V dir.
ret_t app_ac_set_swing(const int addr, const int devIDs, const ac_swing_t hs, const ac_swing_t vs);
ret_t app_ac_set_setting(const int addr, const int devIDs, ac_setting_id_t *psettings, const int cnt);

ret_t app_ac_set_auxisetting(const int addr, const int devIDs, MDAuxiSet_t psettings, const int cnt);

// AC get current parameters if NULL skip value.
ret_t app_ac_get_param(const int addr, const int devIDs, const brandType_t brand, int *pmode, int *ptemp, int *pspeed);
ret_t app_ac_get_param_ex(const int addr, const int devIDs, const brandType_t brand, int *pmode, int *ptemp, int *pspeed, int *pswing, int *psetting);

ret_t read_all_stuff_online_dev(int n, int devIDs);

#ifdef __cplusplus
}
#endif

#endif