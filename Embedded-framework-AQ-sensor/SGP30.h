#ifndef SGP30_H
#define SGP30_H

#include <stdint.h>
#include <stdbool.h>
#include "AQIsensor.h"

/* -------------------------------------------------------------------------
 *  I²C Device Addresses and Command Codes
 * ------------------------------------------------------------------------- */
#define SGP30_I2C_ADDR                 (0x58u)
#define SGP30_SOFTRESET_CMD_MSB        (0x00u)
#define SGP30_SOFTRESET_CMD_LSB        (0x06u)
#define SGP30_CMD_GET_ID_MSB           (0x36u)
#define SGP30_CMD_GET_ID_LSB           (0x82u)
#define SGP30_CMD_GET_VERSION_MSB      (0x20u)
#define SGP30_CMD_GET_VERSION_LSB      (0x2Fu)
#define SGP30_CMD_INIT_MSB             (0x20u)
#define SGP30_CMD_INIT_LSB             (0x03u)
#define SGP30_CMD_MEASURE_IAQ_MSB      (0x20u)
#define SGP30_CMD_MEASURE_IAQ_LSB      (0x08u)
#define SGP30_CMD_MEASURE_RAW_MSB      (0x20u)
#define SGP30_CMD_MEASURE_RAW_LSB      (0x50u)
#define SGP30_CMD_GET_BASELINE_MSB     (0x20u)
#define SGP30_CMD_GET_BASELINE_LSB     (0x15u)
#define SGP30_CMD_SET_BASELINE_MSB     (0x20u)
#define SGP30_CMD_SET_BASELINE_LSB     (0x1Eu)

/* -------------------------------------------------------------------------
 *  Status Enumeration
 * ------------------------------------------------------------------------- */
typedef enum
{
    SGP30_OK = 0u,
    SGP30_ERROR_NULL_PTR,
    SGP30_ERROR_COMM,
    SGP30_ERROR_PARAM,
    SGP30_ERROR_OUT_OF_RANGE
} SGP30_Status_t;

/* -------------------------------------------------------------------------
 *  Sensor Class
 * ------------------------------------------------------------------------- */
struct SGP
{
    struct AQSensor parent;
};

/* Constructor */
void SGP_ctor(struct SGP * const self,
              uint16_t default_tvoc,
              uint16_t default_co2,
              uint16_t ID);

/* Virtual functions */
void SGP_init(void * const pVoid);
void SGP_get_tvoc(void * const pVoid);
void SGP_get_ID(void * const pVoid);
void SGP_get_co2(void * const pVoid);

/* -------------------------------------------------------------------------
 *  Sensor-specific API (error-checked)
 * ------------------------------------------------------------------------- */
SGP30_Status_t SGP30_softreset(void);
SGP30_Status_t SGP30_getid(uint16_t * const iddata);
SGP30_Status_t SGP30_getversion(uint8_t * const version);
SGP30_Status_t SGP30_init(void);
SGP30_Status_t SGP30_measure_quality(uint8_t * const measuredata);
SGP30_Status_t SGP30_CO2_TVOC(uint16_t * const CO2data,
                              uint16_t * const TVOCdata);
SGP30_Status_t SGP30_measure_signal(uint8_t * const airdata);
SGP30_Status_t SGP30_H2_ethon(uint16_t * const H2data,
                              uint16_t * const ethondata);
SGP30_Status_t SGP30_get_baseline(uint8_t * const readair);
SGP30_Status_t SGP30_set_baseline(void);

#endif /* SGP30_H */
