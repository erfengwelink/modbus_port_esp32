#if 0
    main architecture：

                    +-----------+
                    | esp32     |
                    +-------+---+
                            |   [rs485]
                           \|/
                +-----------+--+
                |data converter|
                +--------------+
    +--------+        |         +--------+
    |outer AC|  <-----+-------->|inner AC|  (1)
    +--------+                  +--------+
        .             .              .
        .             .              .
        .             .              .
    +--------+                  +--------+
    |outer AC|                  |inner AC|  (n)
    +--------+                  +--------+

#endif

#include "app_ac_dev.h"
#include "app_modbus.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "MBAPP";

#define DATA_CONVERT_ADDR 0x01
#define GLOBAL_OFFSET 0
//#define MB_LOG(...)
#define MB_LOG(...) ESP_LOGW(__VA_ARGS__)
#define APP_MB_WRITE true
#define APP_MB_READ false

typedef int (*get_p_buf)(int i);

typedef enum _md_param_t_ {
    //following just for read-only cmd && means bit-len.
    //MODE
    //SPEED
    //EXTRA0
    //auxi
    //EXTRA1
    MD_DESCRETE_GP_R,
    MD_TEMP_GP_R,

    // following just for write-only cmd  && means word-len.
    MD_COOL_SYS_W,
    MD_MODE_SET_W,
    MD_SPEED_SET_W,
    MD_TEMP_SET_W,
    MD_PWR_ON_TIMER_W,
    MD_PWR_OFF_TIMER_W,
    MD_AUXI_SET_W,

    MD_PARA_MAX
} mdPara_t;

typedef struct
{
    mdPara_t param;
    mideaFuncType_t func;
    int offsets;
    int addrStart;
    int addrEnd;
    bool rw; //read or write
    get_p_buf pbuf_cb;
} appCtrl_t;

typedef struct
{
    mideaDevType_t dev;
    mideaFuncType_t func;
    int maxNum;
    int basicAddr;
    int segOffset;
} mideaRegMap_t;

typedef enum {
    MD_DESC_PWR,
    MD_DESC_MODE,
    MD_DESC_SPEED,
    MD_DESC_AUXI_ECON_RUN,
    MD_DESC_AUXI_ELECTRIC_PAVING,
    MD_DESC_AUXI_SWING,
    MD_DESC_AUXI_AERATION,
    MD_DESC_AUXI_FRESH,
    MD_DESC_AUXI_HUMIDIFY,
    MD_DESC_AUXI_ADD_OXYGEN,
    MD_DESC_AUXI_DRY,
    MD_DESC_MAX
} descParam_t;

typedef struct
{
    descParam_t param;
    uint8_t bufIndex;
    uint8_t validBits;
    uint8_t startBit;
    readDescreteResult_t res;
} descIndex_t;

typedef enum {
    MD_INPUT_SET_TEMP,
    MD_INPUT_CUR_TEMP,
    MD_INPUT_OUT_ONLINE_0_3,
    MD_INPUT_IN_ONLINE_0_15,
    MD_INPUT_IN_ONLINE_16_31,
    MD_INPUT_IN_ONLINE_32_47,
    MD_INPUT_IN_ONLINE_48_63,
} inputParam_t;

typedef struct
{
    inputParam_t param;
    uint8_t bufIndex;
    uint8_t validBits;
    uint8_t startBit;
    readMideaInputFunc_t res;
} inputIndex_t;

/* 
description:    
|in/out device|register function|total number of ac|basic register address|invalid register total number of per ac| 
*/
const static mideaRegMap_t mMapTable[] = {
    {INNER_DEV, MIDEA_READ_DISCRETE_REG, 64, 10000, 128},
    {INNER_DEV, MIDEA_READ_INPUT_REG, 64, 30000, 32},
    {INNER_DEV, MIDEA_WRITE_HOLDING_REG, 64, 40000, 32},

    {MULTI_INNER_DEV, MIDEA_WRITE_HOLDING_REG, 64, 40000, 32},

    {OUTER_DEV, MIDEA_READ_DISCRETE_REG, 4, 18192, 128},
    {OUTER_DEV, MIDEA_READ_INPUT_REG, 4, 32048, 32},
};

const static appCtrl_t appCtrlTable[] = {
    //power w
    {MD_MODE_SET_W, MIDEA_WRITE_HOLDING_REG, W_MODE_SET, 7, 1, APP_MB_WRITE, NULL},
    //mode w
    {MD_MODE_SET_W, MIDEA_WRITE_HOLDING_REG, W_MODE_SET, 0, 6, APP_MB_WRITE, NULL},
    //speed w
    {MD_SPEED_SET_W, MIDEA_WRITE_HOLDING_REG, W_FAN_SPEED_SET, 0, 3, APP_MB_WRITE, NULL},
    //temp w
    {MD_TEMP_SET_W, MIDEA_WRITE_HOLDING_REG, W_TEMP_SET, 0, 0, APP_MB_WRITE, NULL},
    //swing
    //{MD_AUXI_SET_W, MIDEA_WRITE_HOLDING_REG, W_AUXI_FUNC, 2, 1, APP_MB_WRITE, NULL},
    //auxi setting w
    {MD_AUXI_SET_W, MIDEA_WRITE_HOLDING_REG, W_AUXI_FUNC, 0, 4, APP_MB_WRITE, NULL},

    //power r
    //mode r
    //setting r
    //speed r
    {MD_DESCRETE_GP_R, MIDEA_READ_DISCRETE_REG, 0, 9, 12, APP_MB_READ, get_p_disc_buf},
    //temp r
    {MD_TEMP_GP_R, MIDEA_READ_INPUT_REG, 5, 0, 0, APP_MB_READ, get_p_reg_in_buf},
};

const static descIndex_t descTab[] = {
    {MD_DESC_PWR, 0, 1, 8, MD_PWR_OFF_R},
    {MD_DESC_MODE, 0, 5, 1, MD_CURR_WIND_MODE_R},
    {MD_DESC_SPEED, 1, 4, 1, MD_CURR_HIGH_LEV_SPEED_R},

    {MD_DESC_AUXI_ECON_RUN, 3, 1, 1, MD_ECON_RUN_OFF_R},
    {MD_DESC_AUXI_ELECTRIC_PAVING, 3, 1, 2, MD_ELECTRIC_PAVING_OFF_R},
    {MD_DESC_AUXI_SWING, 3, 1, 3, MD_SWING_OFF_R},
    {MD_DESC_AUXI_AERATION, 3, 1, 4, MD_AERATION_OFF_R},
    {MD_DESC_AUXI_FRESH, 3, 1, 5, MD_FRESH_OFF_R},
    {MD_DESC_AUXI_HUMIDIFY, 3, 1, 6, MD_HUMIDIFY_OFF_R},
    {MD_DESC_AUXI_ADD_OXYGEN, 3, 1, 7, MD_ADD_OXYGEN_OFF_R},
    {MD_DESC_AUXI_DRY, 3, 1, 8, MD_DRY_OFF_R},

};

const static inputIndex_t inputTab[] = {
    {MD_INPUT_SET_TEMP, 4, 0, 0, USER_SET_TEMP},
    {MD_INPUT_CUR_TEMP, 5, 0, 0, INDOOR_TEMP},

    {MD_INPUT_OUT_ONLINE_0_3, 18, 4, 0, OUT_MACHINE_0_3_ONLINE},
    {MD_INPUT_IN_ONLINE_0_15, 19, 16, 0, IN_MACHINE_0_15_ONLINE},
    {MD_INPUT_IN_ONLINE_16_31, 20, 16, 0, IN_MACHINE_16_31_ONLINE},
    {MD_INPUT_IN_ONLINE_32_47, 21, 16, 0, IN_MACHINE_32_47_ONLINE},
    {MD_INPUT_IN_ONLINE_48_63, 22, 16, 0, IN_MACHINE_48_63_ONLINE},
    
};

static int get_dest_addr(const int index, const int acNo, const int offset)
{
    return mMapTable[index].basicAddr +
           acNo * mMapTable[index].segOffset + offset + GLOBAL_OFFSET;
}

static int find_table_index(const mideaDevType_t dev, const mideaFuncType_t cmd)
{
    for (int i = 0; i < sizeof(mMapTable) / sizeof(mideaRegMap_t); i++)
    {
        if (dev == mMapTable[i].dev)
        {
            if (cmd == mMapTable[i].func)
            {
                return i;
            }
        }
    }
    return -1;
}

static int acnum2addr(
    const mideaDevType_t dev,
    const int acNo,
    const int offset,
    const mideaFuncType_t cmd)
{
    int addr = 0;
    int index = -1;
    index = find_table_index(dev, cmd);

    if (index > -1)
    {
        addr = get_dest_addr(index, acNo, offset);
        MB_LOG(TAG, "%s:addr:%d\r\n", __func__, addr);
    }
    else
    {
        MB_LOG(TAG, "%s: ERR>>> no found index in table!\r\n", __func__);
    }

    return addr;
}

static void online_bits_dump(const int data)
{
    printf(" ' '---> offline\r\n");
    printf(" '*'---> online\r\n");
    printf(" 0 1 2 3 4 5 6 7 8 9 A B C D E F \r\n");
    printf(" - - - - - - - - - - - - - - - - \r\n");
    for(int i = 0; i < 16; i++)
    {
        if(data & (0x01<<i))
        {
            printf(" *");
        }
        else
        {
            printf("  ");
        }
    }
    printf("\r\n");
}

static int input_reg_unpack(inputParam_t param, int *pdata)
{
    readMideaInputFunc_t res = INPUT_REG_INVALID;
    inputIndex_t entry = inputTab[param];
    int data = get_p_reg_in_buf(entry.bufIndex);
    int isWordVal = entry.validBits ? 0 : 1;
    if (isWordVal)
    {
        *pdata = data;
        res = entry.res;
    }
    else // for bit operate.
    {
        *pdata = data;
        res = entry.res;
        online_bits_dump(data);
        //online_bits_dump(7);
    }
    return res;
}

static int discrete_reg_unpack(descParam_t param, int *pdata)
{
    descIndex_t entry = descTab[param];
    int start = entry.startBit - 1;
    int end = entry.startBit + entry.validBits - 1;
    int data = get_p_disc_buf(entry.bufIndex);
    readDescreteResult_t result = entry.res;
    MB_LOG(TAG, "%s:data:%2X | start:%d | end %d", __func__, data, start, end);
    if (end > 8)
    {
        MB_LOG(TAG, "end > 8 return -1");
        return -1;
    }
    if (1 == entry.validBits)
    {
        if (data & (0x01 << start))
        {
            result += 1;
            MB_LOG("$", "[[[[ bit param on ]]]");
        }
        else
        {
            MB_LOG("$", "[[[[ bit param off ]]]");
        }
        *pdata = result;
        return 0;
    }
    else
    {
        for (int i = start; i < end; i++)
        {
            if (data & (0x01 << i))
            {
                *pdata = result + i;
                MB_LOG(TAG, "data ::%d || i :: %d | pdata: %d", data, i, *pdata);
                return 0;
            }
        }
    }
    *pdata = MD_COIL_PARAM_INVALID_R;
    return -1;
}

static int load_midea_param(const mideaReqParam_t req, const int acNo, uint16_t wdata)
{
    int ret = -1;
    MB_LOG(TAG, "req = %d", req);
    int regAddr = 0;
    uint16_t sendData = wdata;
    mideaFuncType_t func = appCtrlTable[req].func;
    int singleOffset = appCtrlTable[req].offsets;
    bool rw = appCtrlTable[req].rw;
    int num = 0; //

    assert(req < MD_PARAM_MAX);
    switch (func)
    {
    case MIDEA_READ_DISCRETE_REG:
        assert(APP_MB_READ == rw);
        num = mMapTable[0].segOffset;
        regAddr = acnum2addr(INNER_DEV, acNo, singleOffset, func);
        ret = app_coil_discrete_input_read(DATA_CONVERT_ADDR, BRAND_MIDEA, func, regAddr, num);
        MB_LOG(TAG, " $$app_coil_discrete_input_read$$ret = %d", ret);
        break;
    case MIDEA_READ_INPUT_REG:
        assert(APP_MB_READ == rw);
        num = mMapTable[1].segOffset;
        regAddr = acnum2addr(INNER_DEV, acNo, singleOffset, func);
        ret = app_input_register_read(DATA_CONVERT_ADDR, BRAND_MIDEA, func, regAddr, num); //single or multi-coils
        MB_LOG(TAG, " $$app_input_register_read$$ret = %d", ret);
        break;
    case MIDEA_WRITE_HOLDING_REG:
        assert(APP_MB_WRITE == rw);
        regAddr = acnum2addr(INNER_DEV, acNo, singleOffset, func);
        ret = app_register_single_write(DATA_CONVERT_ADDR, BRAND_MIDEA, func, regAddr, sendData);
        MB_LOG(TAG, " $$$$ret = %d", ret);
        break;
    default:
        return -1;
        break;
    }
    MB_LOG(TAG, "acNo :%d || offset:%d | FUNC:%2X ret = %d ", acNo, singleOffset, func, ret);
    return ret;
}

int app_ac_set_power(const int addr, const int devIDs, const int ison)
{
    uint16_t wValue = (uint16_t)get_p_hold_buf(W_MODE_SET - 1);

    if (!ison)
    {
        wValue &= ~MD_POWER_STATE;
    }
    else
    {
        wValue |= MD_POWER_STATE;
    }
    return load_midea_param(MD_PWR_SET, devIDs, wValue); //app_register_single_write(0x01, BRAND_MIDEA, func, regAddr, wValue);
}

int app_ac_set_mode(const int addr, const int devIDs, const ac_mode_t mode)
{
    assert(mode < ACMODE_MAX);
    uint16_t wValue = (uint16_t)get_p_hold_buf(W_MODE_SET - 1) & ~MD_MODE_MASK;

    switch (mode)
    {
    case ACMODE_FAN:
        wValue |= MD_WIND_MODE;
        break;
    case ACMODE_DRY:
        wValue |= MD_DEHUMI_MODE;
        break;
    case ACMODE_HEAT:
        wValue |= MD_HEAT_MODE;
        break;
    case ACMODE_COOL:
        wValue |= MD_COOL_MODE;
        break;
    case ACMODE_AUTO:
        wValue |= MD_AUTO_MODE;
        break;
    case ACMODE_LOCK:
        wValue |= MD_MODE_LOCK;
        break;
    //case ACMODE_POWER:
    //    wValue |= MD_POWER_STATE;
    //    break;
    default:
        break;
    }
    wValue &= MD_MODE_MASK;
    return load_midea_param(MD_MODE_SET, devIDs, wValue);
}

// AC temperature need 10, such as 26.0 degree 260, range is 160 ~ 300
int app_ac_set_temp(const int addr, const int devIDs, const int temp)
{
    uint16_t wValue = temp / 10;
    if (temp >= 160 && temp <= 320)
    {
        return load_midea_param(MD_TEMP_SET, devIDs, wValue);
    }
    else
    {
        MB_LOG(TAG, "invalid temp value");
        return -1;
    }
}

// AC speed level
int app_ac_set_speed(const int addr, const int devIDs, const ac_speed_t speed)
{
    assert(speed < AC_SPEED_MAX);
    uint16_t wValue = (uint16_t)get_p_hold_buf(W_FAN_SPEED_SET - 1);

    switch (speed)
    {
    case AC_SPEED_AUTO:
        // no support
        break;
    case AC_SPEED_LOW:
        wValue |= MD_LOW_WIND_SPEED;
        break;
    case AC_SPPED_MID:
        wValue |= MD_MID_WIND_SPEED;
        break;
    case AC_SPEED_HIGH:
        wValue |= MD_HIGH_WIND_SPEED;
        break;
    case AC_SPEED_TURBO:
        // no support
        break;
    default:
        break;
    }
    wValue &= MD_WIND_SPEED_MASK;
    if (wValue)
    {
        return load_midea_param(MD_SPEED_SET, devIDs, wValue);
    }
    else
    {
        return -1;
    }
}

int app_ac_set_auxisetting(const int addr, const int devIDs, MDAuxiSet_t psettings, const int cnt)
{
    uint16_t wValue = (uint16_t)get_p_hold_buf(W_AUXI_FUNC - 1);

    switch (psettings)
    {
    case MD_AUXI_ECONOMIC_RUN:
        wValue |= MD_AUXI_ECONOMIC_RUN;
        break;
    case MD_AUXI_HEAT:
        wValue |= MD_AUXI_HEAT;
        break;
    case MD_AUXI_SWING:
        wValue |= MD_AUXI_SWING;
        break;
    case MD_AUXI_AIR_FRESH:
        wValue |= MD_AUXI_AIR_FRESH;
        break;
    default:
        MB_LOG(TAG, "invalid psettings value return -1");
        return -1;
    }
    wValue &= MD_AUXI_MASK;
    return load_midea_param(MD_AUXI_SET, devIDs, wValue);
}

int app_ac_set_swing(const int addr, const int devIDs, const ac_swing_t hs, const ac_swing_t vs)
{
    uint16_t wValue = (uint16_t)get_p_hold_buf(W_AUXI_FUNC - 1);

    switch (hs)
    {
    case AC_SWG_H_V_STOP:
        wValue &= (~MD_AUXI_SWING);
        break;
    case AC_SWG_H_V_RUN:
        wValue |= MD_AUXI_SWING;
        break;
    default:
        break;
    }
    return load_midea_param(MD_AUXI_SET, devIDs, wValue);
}

static int update_target_ac_read_data(mideaReqParam_t type, int devIDs)
{
    return load_midea_param(type, devIDs, 0);
}

int app_ac_get_param(const int addr, const int devIDs, const brandType_t brand, int *pmode, int *ptemp, int *pspeed)
{
    int updated = -1;
    int pwr = -1;

#if 1
    updated = update_target_ac_read_data(MD_DESCRETE_GET, devIDs);
    if (0 == updated) //update ok
    {
        discrete_reg_unpack(MD_DESC_PWR, &pwr);
        MB_LOG(TAG, "pwr:%d", pwr);
        discrete_reg_unpack(MD_DESC_MODE, pmode);
        MB_LOG(TAG, "mode:%d", *pmode);

        discrete_reg_unpack(MD_DESC_SPEED, pspeed);
        MB_LOG(TAG, "pspeed:%d", *pspeed);
    }
    else
    {
    }

    int temp = 0;
    updated = update_target_ac_read_data(MD_TEMP_GET, devIDs);
    if (0 == updated) //update ok
    {
        input_reg_unpack(MD_INPUT_CUR_TEMP, &temp);
        *ptemp = (temp & (0x01 << 15) ? temp - 65536 : temp) * 2 + 40;
        MB_LOG(TAG, "temp : %d ptemp:%d", temp, *ptemp);
    }
    else
    {
        MB_LOG(TAG, "update_target_ac_read_data ->MD_TEMP_GET failed!!!");
    }
#endif

    return 0;
}

int app_ac_get_param_ex(const int addr,
                        const int devIDs,
                        const brandType_t brand,
                        int *pmode,
                        int *ptemp,
                        int *pspeed,
                        int *pswing,
                        int *psetting)
{
    int ret = -1;
    int updated = -1;
    int pwr = -1;
    int economic = -1;
    int epaving = -1;
    int swing = -1;
    int aeration = -1;
    int fresh = -1;
    int hum = -1;
    int oxy = -1;
    int dry = -1;

    updated = update_target_ac_read_data(MD_DESCRETE_GET, devIDs);
    if (0 == updated) //update ok
    {
        ret = discrete_reg_unpack(MD_DESC_PWR, &pwr);
        MB_LOG(TAG, "pwr:%d ret = %d", pwr, ret);

        ret = discrete_reg_unpack(MD_DESC_MODE, pmode);
        MB_LOG(TAG, "mode:%d ret = %d", *pmode, ret);

        ret = discrete_reg_unpack(MD_DESC_SPEED, pspeed);
        MB_LOG(TAG, "pspeed:%d ret = %d", *pspeed, ret);

        ret = discrete_reg_unpack(MD_DESC_AUXI_ECON_RUN, &economic);
        MB_LOG(TAG, "economic:%d ret = %d", economic, ret);

        ret = discrete_reg_unpack(MD_DESC_AUXI_ELECTRIC_PAVING, &epaving);
        MB_LOG(TAG, "epaving:%d ret = %d", epaving, ret);

        ret = discrete_reg_unpack(MD_DESC_AUXI_SWING, &swing);
        MB_LOG(TAG, "swing:%d ret = %d", swing, ret);

        ret = discrete_reg_unpack(MD_DESC_AUXI_AERATION, &aeration);
        MB_LOG(TAG, "aeration:%d ret = %d", aeration, ret);

        ret = discrete_reg_unpack(MD_DESC_AUXI_FRESH, &fresh);
        MB_LOG(TAG, "fresh:%d ret = %d", fresh, ret);

        ret = discrete_reg_unpack(MD_DESC_AUXI_HUMIDIFY, &hum);
        MB_LOG(TAG, "hum:%d ret = %d", hum, ret);

        ret = discrete_reg_unpack(MD_DESC_AUXI_ADD_OXYGEN, &oxy);
        MB_LOG(TAG, "oxy:%d ret = %d", oxy, ret);

        ret = discrete_reg_unpack(MD_DESC_AUXI_DRY, &dry);
        MB_LOG(TAG, "dry:%d ret = %d", dry, ret);

        *pswing = swing;
    }
    else
    {
        MB_LOG(TAG, "update_target_ac_read_data ->MD_DESCRETE_GET failed!!!");
    }

#if 1
    int temp = 0;
    updated = update_target_ac_read_data(MD_TEMP_GET, devIDs);
    if (0 == updated) //update ok
    {
        input_reg_unpack(MD_INPUT_CUR_TEMP, &temp);
        *ptemp = (temp & (0x01<<15) ? temp - 65536 : temp)*2 + 40;
        MB_LOG(TAG, "temp : %d ptemp:%d", temp, *ptemp);
    }
    else
    {
        MB_LOG(TAG, "update_target_ac_read_data ->MD_TEMP_GET failed!!!");
    }
#endif

    return ret;
}

int read_all_stuff_online_dev(int n, int devIDs)
{
    int bits = 0;
    int updated = update_target_ac_read_data(MD_TEMP_GET, devIDs);
    if (0 == updated) //update ok
    {
        input_reg_unpack(MD_INPUT_CUR_TEMP + n, &bits);

        MB_LOG(TAG, "bits : %d ", bits);
    }
    else
    {
        MB_LOG(TAG, "update_target_ac_read_data ->MD_TEMP_GET failed!!!");
    }
    return 0;
}
