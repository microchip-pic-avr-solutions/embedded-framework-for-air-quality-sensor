#include "SGP30.h"
#include <stdio.h>

/* -------------------------------------------------------------------------
 *  Local delay helpers (MISRA-compliant)
 * ------------------------------------------------------------------------- */
static void transfer_delay(void)
{
    (void)__delay_ms(10);
}

static void long_transfer_delay(void)
{
    (void)__delay_ms(100);
}

/* -------------------------------------------------------------------------
 *  Generic Sensor Interface Implementation
 * ------------------------------------------------------------------------- */
void SGP_ctor(struct SGP * const self,
              uint16_t default_tvoc,
              uint16_t default_co2,
              uint16_t ID)
{
    static const struct AQSensorVtbl vtbl =
    {
        &SGP_init,
        &SGP_get_ID,
        &SGP_get_tvoc,
        &SGP_get_co2
    };

    if (self != NULL)
    {
        AQSensor_ctor(&self->parent, default_tvoc, default_co2, ID);
        self->parent.vptr = &vtbl;
    }
}

/* -------------------------------------------------------------------------
 *  Virtual Interface Methods
 * ------------------------------------------------------------------------- */
void SGP_init(void * const pVoid)
{
    (void)pVoid;
    (void)SGP30_init();
}

void SGP_get_ID(void * const pVoid)
{
    if (pVoid != NULL)
    {
        struct SGP * const self_ = (struct SGP *)pVoid;
        (void)SGP30_getid(&self_->parent.ID);
    }
}

void SGP_get_tvoc(void * const pVoid)
{
    if (pVoid != NULL)
    {
        struct SGP * const self_ = (struct SGP *)pVoid;
        (void)SGP30_CO2_TVOC(&self_->parent.CO2, &self_->parent.TVOC);
    }
}

void SGP_get_co2(void * const pVoid)
{
    if (pVoid != NULL)
    {
        struct SGP * const self_ = (struct SGP *)pVoid;
        (void)SGP30_CO2_TVOC(&self_->parent.CO2, &self_->parent.TVOC);
    }
}

/* -------------------------------------------------------------------------
 *  Sensor-Specific Low-Level Functions
 * ------------------------------------------------------------------------- */
SGP30_Status_t SGP30_softreset(void)
{
    uint8_t tx_buf[2] = {SGP30_SOFTRESET_CMD_MSB, SGP30_SOFTRESET_CMD_LSB};
    return (Generic_Comms.Write(SGP30_I2C_ADDR, tx_buf, 2U) == true)
           ? SGP30_OK : SGP30_ERROR_COMM;
}

SGP30_Status_t SGP30_getid(uint16_t * const iddata)
{
    if (iddata == NULL)
    {
        return SGP30_ERROR_NULL_PTR;
    }

    uint8_t tx_buf[2] = {SGP30_CMD_GET_ID_MSB, SGP30_CMD_GET_ID_LSB};
    uint8_t rx_buf[6] = {0};

    if (Generic_Comms.Write(SGP30_I2C_ADDR, tx_buf, 2U) != true)
    {
        return SGP30_ERROR_COMM;
    }

    transfer_delay();

    if (Generic_Comms.Read(SGP30_I2C_ADDR, rx_buf, 6U) != true)
    {
        return SGP30_ERROR_COMM;
    }

    transfer_delay();

    *iddata = ((uint16_t)rx_buf[0] << 8) | rx_buf[1];

    if (*iddata != 0U)
    {
        (void)printf("SGP30 sensor detected (ID: 0x%04X)\n", (unsigned int)*iddata);
    }

    return SGP30_OK;
}

SGP30_Status_t SGP30_getversion(uint8_t * const version)
{
    if (version == NULL)
    {
        return SGP30_ERROR_NULL_PTR;
    }

    uint8_t tx_buf[2] = {SGP30_CMD_GET_VERSION_MSB, SGP30_CMD_GET_VERSION_LSB};

    if (Generic_Comms.Write(SGP30_I2C_ADDR, tx_buf, 2U) != true)
    {
        return SGP30_ERROR_COMM;
    }

    transfer_delay();

    if (Generic_Comms.Read(SGP30_I2C_ADDR, version, 3U) != true)
    {
        return SGP30_ERROR_COMM;
    }

    transfer_delay();
    return SGP30_OK;
}

SGP30_Status_t SGP30_init(void)
{
    (void)printf("SGP30 initialization started...\n");

    uint8_t tx_buf[2] = {SGP30_CMD_INIT_MSB, SGP30_CMD_INIT_LSB};

    if (Generic_Comms.Write(SGP30_I2C_ADDR, tx_buf, 2U) != true)
    {
        return SGP30_ERROR_COMM;
    }

    (void)printf("SGP30 initialization complete.\n");
    return SGP30_OK;
}

SGP30_Status_t SGP30_measure_quality(uint8_t * const measuredata)
{
    if (measuredata == NULL)
    {
        return SGP30_ERROR_NULL_PTR;
    }

    uint8_t tx_buf[2] = {SGP30_CMD_MEASURE_IAQ_MSB, SGP30_CMD_MEASURE_IAQ_LSB};

    if (Generic_Comms.Write(SGP30_I2C_ADDR, tx_buf, 2U) != true)
    {
        return SGP30_ERROR_COMM;
    }

    (void)__delay_ms(100);
    if (Generic_Comms.Read(SGP30_I2C_ADDR, measuredata, 6U) != true)
    {
        return SGP30_ERROR_COMM;
    }

    (void)__delay_ms(1000);
    return SGP30_OK;
}

SGP30_Status_t SGP30_CO2_TVOC(uint16_t * const CO2data, uint16_t * const TVOCdata)
{
    if ((CO2data == NULL) || (TVOCdata == NULL))
    {
        return SGP30_ERROR_NULL_PTR;
    }

    uint8_t airsignal[6] = {0};
    SGP30_Status_t ret = SGP30_measure_quality(airsignal);
    if (ret != SGP30_OK)
    {
        return ret;
    }

    *CO2data  = ((uint16_t)airsignal[0] << 8) | airsignal[1];
    *TVOCdata = ((uint16_t)airsignal[3] << 8) | airsignal[4];

    transfer_delay();
    return SGP30_OK;
}

SGP30_Status_t SGP30_measure_signal(uint8_t * const airdata)
{
    if (airdata == NULL)
    {
        return SGP30_ERROR_NULL_PTR;
    }

    uint8_t tx_buf[2] = {SGP30_CMD_MEASURE_RAW_MSB, SGP30_CMD_MEASURE_RAW_LSB};

    if (Generic_Comms.Write(SGP30_I2C_ADDR, tx_buf, 2U) != true)
    {
        return SGP30_ERROR_COMM;
    }

    transfer_delay();
    long_transfer_delay();

    if (Generic_Comms.Read(SGP30_I2C_ADDR, airdata, 6U) != true)
    {
        return SGP30_ERROR_COMM;
    }

    transfer_delay();
    return SGP30_OK;
}

SGP30_Status_t SGP30_H2_ethon(uint16_t * const H2data, uint16_t * const ethondata)
{
    if ((H2data == NULL) || (ethondata == NULL))
    {
        return SGP30_ERROR_NULL_PTR;
    }

    uint8_t airsignal[6] = {0};
    SGP30_Status_t ret = SGP30_measure_signal(airsignal);
    if (ret != SGP30_OK)
    {
        return ret;
    }

    *H2data     = ((uint16_t)airsignal[0] << 8) | airsignal[1];
    *ethondata  = ((uint16_t)airsignal[3] << 8) | airsignal[4];

    transfer_delay();
    return SGP30_OK;
}

SGP30_Status_t SGP30_get_baseline(uint8_t * const readair)
{
    if (readair == NULL)
    {
        return SGP30_ERROR_NULL_PTR;
    }

    uint8_t tx_buf[2] = {SGP30_CMD_GET_BASELINE_MSB, SGP30_CMD_GET_BASELINE_LSB};

    if (Generic_Comms.Write(SGP30_I2C_ADDR, tx_buf, 2U) != true)
    {
        return SGP30_ERROR_COMM;
    }

    transfer_delay();

    if (Generic_Comms.Read(SGP30_I2C_ADDR, readair, 6U) != true)
    {
        return SGP30_ERROR_COMM;
    }

    transfer_delay();
    return SGP30_OK;
}

SGP30_Status_t SGP30_set_baseline(void)
{
    uint8_t tx_buf[8] = {0};

    SGP30_Status_t ret = SGP30_get_baseline(&tx_buf[2]);
    if (ret != SGP30_OK)
    {
        return ret;
    }

    tx_buf[0] = SGP30_CMD_SET_BASELINE_MSB;
    tx_buf[1] = SGP30_CMD_SET_BASELINE_LSB;

    if (Generic_Comms.Write(SGP30_I2C_ADDR, tx_buf, 8U) != true)
    {
        return SGP30_ERROR_COMM;
    }

    transfer_delay();
    return SGP30_OK;
}
