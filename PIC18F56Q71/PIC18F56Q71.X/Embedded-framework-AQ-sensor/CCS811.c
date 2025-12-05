#include "CCS811.h"
#include <stdio.h>

/* -------------------------------------------------------------------------
 *  Global Variables
 * ------------------------------------------------------------------------- */
CCS811_Result_t AirQuality_Data = {0u, 0u, 0u, 0u, 0u};
static uint8_t tx_buf[8] = {0};



/* -------------------------------------------------------------------------
 *  Generic Sensor Interface Implementation
 * ------------------------------------------------------------------------- */
void CCS_ctor(struct CCS * const self,
              uint16_t default_tvoc,
              uint16_t default_co2,
              uint16_t ID)
{
    static const struct AQSensorVtbl vtbl =
    {
        &CCS_init,
        &CCS_get_ID,
        &CCS_get_tvoc,
        &CCS_get_co2
    };

    if (self != NULL)
    {
        AQSensor_ctor(&self->parent, default_tvoc, default_co2, ID);
        self->parent.vptr = &vtbl;
    }
}

void CCS_init(void * const pVoid)
{
    (void)pVoid;
    (void)CCS811_init();
}

void CCS_get_ID(void * const pVoid)
{
    if (pVoid != NULL)
    {
        struct CCS * const self_ = (struct CCS *)pVoid;
        self_->parent.ID = CCS811_get_ID();
    }
}

void CCS_get_tvoc(void * const pVoid)
{
    if (pVoid != NULL)
    {
        struct CCS * const self_ = (struct CCS *)pVoid;
        if (CCS811_CO2_TVOC() != 0U)
        {
            self_->parent.TVOC = AirQuality_Data.TVOC;
        }
    }
}

void CCS_get_co2(void * const pVoid)
{
    if (pVoid != NULL)
    {
        struct CCS * const self_ = (struct CCS *)pVoid;
        if (CCS811_CO2_TVOC() != 0U)
        {
            self_->parent.CO2 = AirQuality_Data.CO2;
        }
    }
}

/* -------------------------------------------------------------------------
 *  Low-Level I²C R/W Wrappers with Checks
 * ------------------------------------------------------------------------- */
CCS811_Status_t CCS_Write( uint8_t * data_out, uint8_t data_len)
{
    if ((data_out == NULL) || (data_len == 0U))
    {
        return CCS811_ERROR_NULL_PTR;
    }

    if (Generic_Comms.Write(CCS811_I2C_ADDR, data_out, data_len) != true)
    {
        return CCS811_ERROR_COMM;
    }

    return CCS811_OK;
}

CCS811_Status_t CCS_Read(uint8_t data_reg, uint8_t * const data_in, uint8_t data_len)
{
    if ((data_in == NULL) || (data_len == 0U))
    {
        return CCS811_ERROR_NULL_PTR;
    }

    tx_buf[0] = data_reg;

    if (Generic_Comms.Write(CCS811_I2C_ADDR, tx_buf, 1U) != true)
    {
        return CCS811_ERROR_COMM;
    }

    __delay_ms(1U);

    if (Generic_Comms.Read(CCS811_I2C_ADDR, data_in, data_len) != true)
    {
        return CCS811_ERROR_COMM;
    }

    __delay_ms(1U);
    return CCS811_OK;
}

/* -------------------------------------------------------------------------
 *  Sensor Initialization Sequence
 * ------------------------------------------------------------------------- */
CCS811_Status_t CCS811_init(void)
{
    (void)printf("Air Quality 3 - CCS811 Initialization started\n");
    (void)printf("Sensor Initializing...\n");

    Wake_Click();
    APP_START();

    __delay_ms(1U);
    AirQuality_Data.STATUS = GetStatus();
    Set_Measure_Mode(0x01U);
    __delay_ms(1000U);

    (void)printf("Sensor CCS811 Initialization complete\n\n\n");
    return CCS811_OK;
}

/* -------------------------------------------------------------------------
 *  Sensor ID and Data Functions
 * ------------------------------------------------------------------------- */
uint8_t CCS811_get_ID(void)
{
    uint8_t ID = 0U;

    if (CCS_Read(AIRQUALITY3_REG_ID, &ID, 1U) != CCS811_OK)
    {
        return 0U;
    }

    if (ID == 0x81U)
    {
        (void)printf("CCS811 sensor detected\n");
    }

    return ID;
}

uint8_t CCS811_CO2_TVOC(void)
{
    uint8_t ALG_RESULT[8] = {0};
    uint8_t ready = 0U;

    while (CCSINT1_GetValue() != 0U)
    {
        /* wait for interrupt low */
    }

    if (CCS_Read(AIRQUALITY3_REG_ALG_DATA, ALG_RESULT, 8U) != CCS811_OK)
    {
        return 0U;
    }

    if ((ALG_RESULT[4] & 0x08U) != 0U)
    {
        AirQuality_Data.CO2       = ((uint16_t)ALG_RESULT[0] << 8) | ALG_RESULT[1];
        AirQuality_Data.TVOC      = ((uint16_t)ALG_RESULT[2] << 8) | ALG_RESULT[3];
        AirQuality_Data.RAW_DATA  = AirQuality_Data.CO2;
        AirQuality_Data.STATUS    = ALG_RESULT[4];
        AirQuality_Data.ERROR_ID  = ALG_RESULT[5];
        ready = 1U;
    }

    return ready;
}

/* -------------------------------------------------------------------------
 *  Command Helpers
 * ------------------------------------------------------------------------- */
void APP_START(void)
{
    tx_buf[0] = AIRQUALITY3_APP_START;
    (void)CCS_Write(tx_buf, 1U);
}

void Set_Measure_Mode(uint8_t mode)
{
    uint8_t register_value = (uint8_t)((mode << 4) | 0x08U);
    tx_buf[0] = AIRQUALITY3_REG_MEAS_MODE;
    tx_buf[1] = register_value;
    (void)CCS_Write(tx_buf, 2U);
}

uint8_t GetStatus(void)
{
    uint8_t status = 0U;
    (void)CCS_Read(AIRQUALITY3_REG_STATUS, &status, 1U);
    return status;
}

/* -------------------------------------------------------------------------
 *  Hardware Control Helpers
 * ------------------------------------------------------------------------- */
void Hardware_Reset(void)
{
    CCSRST_SetLow();
    __delay_ms(2000U);
    CCSRST_SetHigh();
    __delay_ms(200U);
}

void Wake_Click(void)
{
    CCSSS_SetHigh();
    Hardware_Reset();
    CCSSS_SetLow();
}
