#ifndef CCS811_H
#define CCS811_H

#include <stdint.h>
#include <stdbool.h>
#include "AQIsensor.h"

/* -------------------------------------------------------------------------
 *  Device Address and Register Map
 * ------------------------------------------------------------------------- */
#define CCS811_I2C_ADDR                 (0x5Au)
#define AIRQUALITY3_APP_ERASE           (0xF1u)
#define AIRQUALITY3_APP_DATA            (0xF2u)
#define AIRQUALITY3_APP_VERIFY          (0xF3u)
#define AIRQUALITY3_APP_START           (0xF4u)

#define AIRQUALITY3_REG_STATUS          (0x00u)
#define AIRQUALITY3_REG_MEAS_MODE       (0x01u)
#define AIRQUALITY3_REG_ALG_DATA        (0x02u)
#define AIRQUALITY3_REG_RAW_DATA        (0x03u)
#define AIRQUALITY3_REG_ENV_DATA        (0x05u)
#define AIRQUALITY3_REG_BASELINE        (0x11u)
#define AIRQUALITY3_REG_ID              (0x20u)
#define AIRQUALITY3_REG_HW_VERSION      (0x21u)
#define AIRQUALITY3_REG_FW_BOOT_VERSION (0x23u)
#define AIRQUALITY3_REG_FW_APP_VERSION  (0x24u)
#define AIRQUALITY3_REG_ERROR_ID        (0xE0u)
#define AIRQUALITY3_REG_SW_RESET        (0xFFu)

/* -------------------------------------------------------------------------
 *  Status Codes
 * ------------------------------------------------------------------------- */
typedef enum
{
    CCS811_OK = 0u,
    CCS811_ERROR_NULL_PTR,
    CCS811_ERROR_COMM,
    CCS811_ERROR_PARAM,
    CCS811_ERROR_TIMEOUT
} CCS811_Status_t;

/* -------------------------------------------------------------------------
 *  Sensor Result Structure
 * ------------------------------------------------------------------------- */
typedef struct
{
    uint16_t CO2;
    uint16_t TVOC;
    uint8_t  STATUS;
    uint8_t  ERROR_ID;
    uint16_t RAW_DATA;
} CCS811_Result_t;

/* -------------------------------------------------------------------------
 *  Generic Sensor Interface Class
 * ------------------------------------------------------------------------- */
struct CCS
{
    struct AQSensor parent;
};

/* -------------------------------------------------------------------------
 *  Constructors and Virtual Functions
 * ------------------------------------------------------------------------- */
void CCS_ctor(struct CCS * const self,
              uint16_t default_tvoc,
              uint16_t default_co2,
              uint16_t ID);

void CCS_init(void * const pVoid);
void CCS_get_tvoc(void * const pVoid);
void CCS_get_co2(void * const pVoid);
void CCS_get_ID(void * const pVoid);

/* -------------------------------------------------------------------------
 *  Low-Level I²C Operations
 * ------------------------------------------------------------------------- */
CCS811_Status_t CCS_Write(uint8_t * data_out, uint8_t data_len);
CCS811_Status_t CCS_Read(uint8_t data_reg, uint8_t * const data_in, uint8_t data_len);

/* -------------------------------------------------------------------------
 *  Sensor-Specific APIs
 * ------------------------------------------------------------------------- */
CCS811_Status_t CCS811_init(void);
uint8_t         CCS811_get_ID(void);
uint8_t         CCS811_CO2_TVOC(void);
void            APP_START(void);
void            Set_Measure_Mode(uint8_t mode);
uint8_t         GetStatus(void);
void            Hardware_Reset(void);
void            Wake_Click(void);

/* -------------------------------------------------------------------------
 *  External Measurement Result Variable
 * ------------------------------------------------------------------------- */
extern CCS811_Result_t AirQuality_Data;

#endif /* CCS811_H */
