#ifndef AQ11_H
#define AQ11_H

#include <stdint.h>
#include <stdbool.h>
#include "AQIsensor.h"

/* Registers (unchanged) */
#define AIRQUALITY11_REG_PART_ID               0x00u
#define AIRQUALITY11_REG_OPMODE                0x10u
#define AIRQUALITY11_REG_CONFIG                0x11u
#define AIRQUALITY11_REG_COMMAND               0x12u
#define AIRQUALITY11_REG_TEMP_IN               0x13u
#define AIRQUALITY11_REG_RH_IN                 0x15u
#define AIRQUALITY11_REG_DEVICE_STATUS         0x20u
#define AIRQUALITY11_REG_DATA_AQI_UBA          0x21u
#define AIRQUALITY11_REG_DATA_ETVOC            0x22u
#define AIRQUALITY11_REG_DATA_ECO2             0x24u
#define AIRQUALITY11_REG_DATA_AQI_S            0x26u
#define AIRQUALITY11_REG_DATA_T                0x30u
#define AIRQUALITY11_REG_DATA_RH               0x32u
#define AIRQUALITY11_REG_DATA_MISR             0x38u
#define AIRQUALITY11_REG_GPR_WRITE             0x40u
#define AIRQUALITY11_REG_GPR_READ              0x48u

/* Measure conditions */
#define AIRQUALITY11_TEMP_MIN                 (-40.0f)
#define AIRQUALITY11_TEMP_MAX                  (80.0f)
#define AIRQUALITY11_TEMP_CALC_KC              (273.15f)
#define AIRQUALITY11_TEMP_CALC_MLT             (64.0f)
#define AIRQUALITY11_HUM_MIN                   (5.0f)
#define AIRQUALITY11_HUM_MAX                   (95.0f)
#define AIRQUALITY11_HUM_CALC_MLT              (512.0f)

/* Other constants */
#define AIRQUALITY11_AQI_UBA_BITMASK            0x07u
#define AIRQUALITY11_PART_ID                   0x0161u
#define AIRQUALITY11_DEVICE_ADDRESS_0          0x52u
#define AIRQUALITY11_DEVICE_ADDRESS_1          0x53u
#define AIRQUALITY11_RESET                     0xF0u
#define AIRQUALITY11_IDLE                      0x01u
#define AIRQUALITY11_STANDARD                  0x02u
#define AIRQUALITY11_LOW_POWER                 0x03u
#define AIRQUALITY11_ULTRA_LOW_POWER           0x04u
#define AIRQUALITY11_DEEP_SLEEP                0x00u

/* Status codes returned by public functions */
typedef enum
{
    AQ11_OK = 0u,
    AQ11_ERROR_NULL_PTR,
    AQ11_ERROR_COMM,
    AQ11_ERROR_OUT_OF_RANGE,
    AQ11_ERROR_ID_MISMATCH,
    AQ11_ERROR_INVALID_PARAM
} AQ11_Status_t;

/* ENS161 generic interface (unchanged prototypes but left here for completeness) */
struct ENS161
{
    struct AQSensor parent;
};

void ENS161_ctor(struct ENS161 * const self,
                 uint16_t default_tvoc, uint16_t default_co2,
                 uint16_t ID);

void ENS161_init(void * const pVoid);
void ENS161_get_tvoc(void * const pVoid);
void ENS161_get_ID(void * const pVoid);
void ENS161_get_co2(void * const pVoid);

/* Sensor-specific functions - updated to return AQ11_Status_t and use out parameters where applicable */
AQ11_Status_t AQ11_interrupt(uint8_t * const value);
AQ11_Status_t AQ11_get_measure(float * const temperature, float * const humidity);
AQ11_Status_t AQ11_get_tvoc(uint16_t * const tvoc);
AQ11_Status_t AQ11_get_co2(uint16_t * const eco2);
AQ11_Status_t AQ11_get_aq(uint16_t * const aqi);
AQ11_Status_t AQ11_get_deviceID(uint16_t * const deviceID);
AQ11_Status_t AQ11_get_UBA(uint8_t * const uba);
AQ11_Status_t AQ11_get_status(uint8_t * const status);
AQ11_Status_t AQ11_set_measure(float temperature, float humidity);
AQ11_Status_t AQ11_set_cmd(uint8_t cmd);
AQ11_Status_t AQ11_set_config(uint8_t config);
AQ11_Status_t AQ11_set_opmode(uint8_t op);
AQ11_Status_t AQ11_defaultconfig(uint8_t temperature, uint8_t humidity);
AQ11_Status_t AQ11_init(void);

#endif /* AQ11_H */
