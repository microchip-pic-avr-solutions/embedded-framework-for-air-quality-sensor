#include "AQ11.h"
#include <stdio.h>
#include <string.h>

/* Local helper macros to make intent explicit */
#define AQ11_DATA_BUF_LEN_1    1u
#define AQ11_DATA_BUF_LEN_2    2u

/* forward declarations for static helpers */
static AQ11_Status_t AQ11_read_bytes(const uint8_t reg,
                                     uint8_t * const buf,
                                     const uint8_t buf_len);

/* --------------------------------------------------------------------------
   Generic sensor interface implementation (kept function bodies, minimal change)
   -------------------------------------------------------------------------- */

void ENS161_ctor(struct ENS161 * const self,
                 uint16_t default_tvoc, uint16_t default_co2,
                 uint16_t ID)
{
    /* vtbl must be constant; use file-scope static const */
    static struct AQSensorVtbl const vtbl =
    {
        &ENS161_init,
        &ENS161_get_ID,
        &ENS161_get_tvoc,
        &ENS161_get_co2
    };

    if (self != NULL)
    {
        AQSensor_ctor(&self->parent, default_tvoc, default_co2, ID);
        self->parent.vptr = &vtbl;
    }
}

void ENS161_init(void * const pVoid)
{
    (void)pVoid;
    AQ11_init();
}

void ENS161_get_ID(void * const pVoid)
{
    struct ENS161 * const self_ = (struct ENS161 *)pVoid;
    if (self_ != NULL)
    {
        uint16_t dev_id = 0u;
        if (AQ11_get_deviceID(&dev_id) == AQ11_OK)
        {
            self_->parent.ID = dev_id;
        }
        else
        {
            /* keep previous value if read failed */
        }
    }
}

void ENS161_get_tvoc(void * const pVoid)
{
    struct ENS161 * const self_ = (struct ENS161 *)pVoid;
    if (self_ != NULL)
    {
        uint16_t tvoc = 0u;
        if (AQ11_get_tvoc(&tvoc) == AQ11_OK)
        {
            self_->parent.TVOC = tvoc;
        }
    }
}

void ENS161_get_co2(void * const pVoid)
{
    struct ENS161 * const self_ = (struct ENS161 *)pVoid;
    if (self_ != NULL)
    {
        uint16_t co2 = 0u;
        if (AQ11_get_co2(&co2) == AQ11_OK)
        {
            self_->parent.CO2 = co2;
        }
    }
}

/* --------------------------------------------------------------------------
   Sensor-specific functions
   -------------------------------------------------------------------------- */

/* Note: Generic_Comms.WriteRead / Write are left unchanged as requested.
   They appear to provide no explicit error return; therefore functions using
   them perform basic plausibility checks on the returned buffer data and
   return AQ11_ERROR_COMM if data is clearly invalid. */

AQ11_Status_t AQ11_interrupt(uint8_t * const value)
{
    if (value == NULL)
    {
        return AQ11_ERROR_NULL_PTR;
    }

    /* Assume AQINT_GetValue returns uint8_t; keep direct call */
    *value = AQINT_GetValue();

    return AQ11_OK;
}

AQ11_Status_t AQ11_get_measure(float * const temperature, float * const humidity)
{
    uint8_t data_buf[AQ11_DATA_BUF_LEN_2] = {0u};
    uint8_t write_reg;

    if ((temperature == NULL) || (humidity == NULL))
    {
        return AQ11_ERROR_NULL_PTR;
    }

    /* Read temperature */
    write_reg = (uint8_t)AIRQUALITY11_REG_DATA_T;
    if (AQ11_read_bytes(write_reg, data_buf, AQ11_DATA_BUF_LEN_2) != AQ11_OK)
    {
        return AQ11_ERROR_COMM;
    }

    {
        uint16_t raw = ((uint16_t)data_buf[0] << 8u) | (uint16_t)data_buf[1];
        float temp_calc = ((float)raw / AIRQUALITY11_TEMP_CALC_MLT) - AIRQUALITY11_TEMP_CALC_KC;
        /* Validate plausible range */
        if ((temp_calc < (AIRQUALITY11_TEMP_MIN - 10.0f)) || (temp_calc > (AIRQUALITY11_TEMP_MAX + 10.0f)))
        {
            return AQ11_ERROR_COMM;
        }
        *temperature = temp_calc;
    }

    /* Read humidity */
    write_reg = (uint8_t)AIRQUALITY11_REG_DATA_RH;
    if (AQ11_read_bytes(write_reg, data_buf, AQ11_DATA_BUF_LEN_2) != AQ11_OK)
    {
        return AQ11_ERROR_COMM;
    }

    {
        uint16_t raw = ((uint16_t)data_buf[0] << 8u) | (uint16_t)data_buf[1];
        float hum_calc = (float)raw / AIRQUALITY11_HUM_CALC_MLT;
        if ((hum_calc < (AIRQUALITY11_HUM_MIN - 10.0f)) || (hum_calc > (AIRQUALITY11_HUM_MAX + 10.0f)))
        {
            return AQ11_ERROR_COMM;
        }
        *humidity = hum_calc;
    }

    return AQ11_OK;
}

AQ11_Status_t AQ11_get_tvoc(uint16_t * const tvoc)
{
    uint8_t data_buf[AQ11_DATA_BUF_LEN_2] = {0u};
    uint8_t write_reg;

    if (tvoc == NULL)
    {
        return AQ11_ERROR_NULL_PTR;
    }

    write_reg = (uint8_t)AIRQUALITY11_REG_DATA_ETVOC;
    if (AQ11_read_bytes(write_reg, data_buf, AQ11_DATA_BUF_LEN_2) != AQ11_OK)
    {
        return AQ11_ERROR_COMM;
    }

    /* little-endian conversion in original code was (low,high) -> (high<<8 | low) */
    *tvoc = (uint16_t)(((uint16_t)data_buf[1] << 8u) | (uint16_t)data_buf[0]);

    return AQ11_OK;
}

AQ11_Status_t AQ11_get_co2(uint16_t * const eco2)
{
    uint8_t data_buf[AQ11_DATA_BUF_LEN_2] = {0u};
    uint8_t write_reg;

    if (eco2 == NULL)
    {
        return AQ11_ERROR_NULL_PTR;
    }

    write_reg = (uint8_t)AIRQUALITY11_REG_DATA_ECO2;
    if (AQ11_read_bytes(write_reg, data_buf, AQ11_DATA_BUF_LEN_2) != AQ11_OK)
    {
        return AQ11_ERROR_COMM;
    }

    *eco2 = (uint16_t)(((uint16_t)data_buf[1] << 8u) | (uint16_t)data_buf[0]);

    return AQ11_OK;
}

AQ11_Status_t AQ11_get_aq(uint16_t * const aqi)
{
    uint8_t data_buf[AQ11_DATA_BUF_LEN_2] = {0u};
    uint8_t write_reg;

    if (aqi == NULL)
    {
        return AQ11_ERROR_NULL_PTR;
    }

    write_reg = (uint8_t)AIRQUALITY11_REG_DATA_AQI_S;
    if (AQ11_read_bytes(write_reg, data_buf, AQ11_DATA_BUF_LEN_2) != AQ11_OK)
    {
        return AQ11_ERROR_COMM;
    }

    *aqi = (uint16_t)(((uint16_t)data_buf[1] << 8u) | (uint16_t)data_buf[0]);

    return AQ11_OK;
}

AQ11_Status_t AQ11_get_UBA(uint8_t * const uba)
{
    uint8_t data_buf[AQ11_DATA_BUF_LEN_1] = {0u};
    uint8_t write_reg;

    if (uba == NULL)
    {
        return AQ11_ERROR_NULL_PTR;
    }

    write_reg = (uint8_t)AIRQUALITY11_REG_DATA_AQI_UBA;
    if (AQ11_read_bytes(write_reg, data_buf, AQ11_DATA_BUF_LEN_1) != AQ11_OK)
    {
        return AQ11_ERROR_COMM;
    }

    *uba = data_buf[0] & (uint8_t)AIRQUALITY11_AQI_UBA_BITMASK;

    return AQ11_OK;
}

AQ11_Status_t AQ11_get_status(uint8_t * const status)
{
    uint8_t data_buf[AQ11_DATA_BUF_LEN_1] = {0u};
    uint8_t write_reg;

    if (status == NULL)
    {
        return AQ11_ERROR_NULL_PTR;
    }

    write_reg = (uint8_t)AIRQUALITY11_REG_DEVICE_STATUS;
    if (AQ11_read_bytes(write_reg, data_buf, AQ11_DATA_BUF_LEN_1) != AQ11_OK)
    {
        return AQ11_ERROR_COMM;
    }

    *status = data_buf[0];

    return AQ11_OK;
}

AQ11_Status_t AQ11_get_deviceID(uint16_t * const deviceID)
{
    uint8_t data_buf[AQ11_DATA_BUF_LEN_2] = {0u};
    uint8_t write_reg;

    if (deviceID == NULL)
    {
        return AQ11_ERROR_NULL_PTR;
    }

    write_reg = (uint8_t)AIRQUALITY11_REG_PART_ID;
    if (AQ11_read_bytes(write_reg, data_buf, AQ11_DATA_BUF_LEN_2) != AQ11_OK)
    {
        return AQ11_ERROR_COMM;
    }

    /* original combined as (high<<8) | low where data_buf[1] was high */
    *deviceID = (uint16_t)(((uint16_t)data_buf[1] << 8u) | (uint16_t)data_buf[0]);

    if (*deviceID != (uint16_t)AIRQUALITY11_PART_ID)
    {
        return AQ11_ERROR_ID_MISMATCH;
    }

    return AQ11_OK;
}

AQ11_Status_t AQ11_set_measure(float temperature, float humidity)
{
    /* validate parameters first */
    if ((temperature < AIRQUALITY11_TEMP_MIN) || (temperature > AIRQUALITY11_TEMP_MAX) ||
        (humidity < AIRQUALITY11_HUM_MIN) || (humidity > AIRQUALITY11_HUM_MAX))
    {
        return AQ11_ERROR_OUT_OF_RANGE;
    }

    {
        uint8_t temp_buf[3] = {0u};
        uint16_t temp_raw;
        float temp_work = (temperature + AIRQUALITY11_TEMP_CALC_KC) * AIRQUALITY11_TEMP_CALC_MLT;

        /* clamp to 16-bit unsigned before splitting - MISRA: explicit cast */
        if (temp_work < 0.0f)
        {
            temp_raw = 0u;
        }
        else if (temp_work > 65535.0f)
        {
            temp_raw = 65535u;
        }
        else
        {
            temp_raw = (uint16_t)temp_work;
        }

        temp_buf[0] = (uint8_t)AIRQUALITY11_REG_TEMP_IN;
        temp_buf[1] = (uint8_t)((temp_raw >> 8u) & 0xFFu);
        temp_buf[2] = (uint8_t)(temp_raw & 0xFFu);

        Generic_Comms.Write((uint8_t)AIRQUALITY11_DEVICE_ADDRESS_0, temp_buf, (uint8_t)3u);
        __delay_ms(10);
    }

    {
        uint8_t hum_buf[2] = {0u};
        uint16_t hum_raw;
        float hum_work = humidity * AIRQUALITY11_HUM_CALC_MLT;

        if (hum_work < 0.0f)
        {
            hum_raw = 0u;
        }
        else if (hum_work > 65535.0f)
        {
            hum_raw = 65535u;
        }
        else
        {
            hum_raw = (uint16_t)hum_work;
        }

        hum_buf[0] = (uint8_t)AIRQUALITY11_REG_RH_IN;
        hum_buf[1] = (uint8_t)(hum_raw & 0xFFu);

        Generic_Comms.Write((uint8_t)AIRQUALITY11_DEVICE_ADDRESS_0, hum_buf, (uint8_t)2u);
        __delay_ms(10);
    }

    return AQ11_OK;
}

AQ11_Status_t AQ11_set_cmd(uint8_t cmd)
{
    uint8_t data_buf[2] = {0u};
    data_buf[0] = (uint8_t)AIRQUALITY11_REG_COMMAND;
    data_buf[1] = cmd;

    Generic_Comms.Write((uint8_t)AIRQUALITY11_DEVICE_ADDRESS_0, data_buf, (uint8_t)2u);
    __delay_ms(10);

    return AQ11_OK;
}

AQ11_Status_t AQ11_set_config(uint8_t config)
{
    uint8_t data_buf[2] = {0u};
    data_buf[0] = (uint8_t)AIRQUALITY11_REG_CONFIG;
    data_buf[1] = config;

    Generic_Comms.Write((uint8_t)AIRQUALITY11_DEVICE_ADDRESS_0, data_buf, (uint8_t)2u);
    __delay_ms(10);

    return AQ11_OK;
}

AQ11_Status_t AQ11_set_opmode(uint8_t op)
{
    uint8_t data_buf[2] = {0u};
    data_buf[0] = (uint8_t)AIRQUALITY11_REG_OPMODE;
    data_buf[1] = op;

    Generic_Comms.Write((uint8_t)AIRQUALITY11_DEVICE_ADDRESS_0, data_buf, (uint8_t)2u);
    __delay_ms(10);

    return AQ11_OK;
}

AQ11_Status_t AQ11_defaultconfig(uint8_t temperature, uint8_t humidity)
{
    uint16_t device_ID = 0u;
    AQ11_Status_t st;

    st = AQ11_get_deviceID(&device_ID);
    if (st != AQ11_OK)
    {
        (void)printf("Error with configuration, wrong device ID\n\r");
        return st;
    }

    /* reset */
    (void)AQ11_set_opmode(AIRQUALITY11_RESET);
    __delay_ms(100);

    (void)AQ11_set_opmode(AIRQUALITY11_IDLE);
    __delay_ms(10);

    /* set measure; guard return */
    st = AQ11_set_measure((float)temperature, (float)humidity);
    if (st != AQ11_OK)
    {
        return st;
    }
    __delay_ms(10);

    (void)AQ11_set_opmode(AIRQUALITY11_STANDARD);
    __delay_ms(10);

    return AQ11_OK;
}

AQ11_Status_t AQ11_init(void)
{
    uint16_t device_ID = 0u;
    AQ11_Status_t st;

    AQSS_SetHigh();
    AQPWM_SetHigh();
    __delay_ms(1000);

    st = AQ11_get_deviceID(&device_ID);
    if (st != AQ11_OK)
    {
        /* device id mismatch or comm failed */
        (void)printf("Device ID read failed or mismatch: %u\n", (unsigned int)device_ID);
        return st;
    }

    (void)printf("Device ID: %04x\n", (unsigned int)device_ID);
    (void)printf("Air Quality 11 - ENS161 Sensor Initialization started\n");

    /* Set to standard opmode */
    (void)AQ11_set_opmode((uint8_t)AIRQUALITY11_STANDARD);
    (void)printf("Set OPMode to Standard Mode \n");
    (void)printf("Sensor ENS161 Initializing done\n\n\n");

    __delay_ms(3000);

    return AQ11_OK;
}

/* ---------------------------
   Static helper definitions
   --------------------------- */

/* Read `buf_len` bytes from register `reg` into buf using Generic_Comms.WriteRead.
   Since Generic_Comms.WriteRead does not return an error code in the original API,
   this helper performs basic plausibility checks:
   - buf must be non-NULL
   - after transaction, the buffer must not be all 0x00 or all 0xFF (heuristic)
   If a stronger error API exists for Generic_Comms, replace this with that call. */
static AQ11_Status_t AQ11_read_bytes(const uint8_t reg,
                                     uint8_t * const buf,
                                     const uint8_t buf_len)
{
    uint8_t write_buf = reg;
    uint8_t i;
    uint8_t all_zero = 1u;
    uint8_t all_ff = 1u;

    if ((buf == NULL) || (buf_len == 0u))
    {
        return AQ11_ERROR_NULL_PTR;
    }

    /* perform I2C transaction - left unchanged */
    Generic_Comms.WriteRead((uint8_t)AIRQUALITY11_DEVICE_ADDRESS_0,
                            &write_buf,
                            (uint8_t)1u,
                            buf,
                            buf_len);

    __delay_ms(10);

    /* basic plausibility checks */
    for (i = 0u; i < buf_len; ++i)
    {
        if (buf[i] != 0u)
        {
            all_zero = 0u;
        }
        if (buf[i] != 0xFFu)
        {
            all_ff = 0u;
        }
    }

    if ((all_zero != 0u) || (all_ff != 0u))
    {
        /* heuristic detection of bad comms */
        return AQ11_ERROR_COMM;
    }

    return AQ11_OK;
}
