#ifndef EN2_H
#define EN2_H

#include "AQIsensor.h"
#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 *  EN2 Status Enumeration (modeled after SGP30)
 * ------------------------------------------------------------------------- */
typedef enum
{
    EN2_OK = 0u,
    EN2_ERROR_NULL_PTR,
    EN2_ERROR_COMM,
    EN2_ERROR_PARAM,
    EN2_ERROR_OUT_OF_RANGE
} EN2_Status_t;

/* -------------------------------------------------------------------------
 *  Generic Sensor Interface Implementation
 * ------------------------------------------------------------------------- */
struct EN2
{
    struct AQSensor parent;
    
};

void EN2_ctor(struct EN2 * const self, 
  uint16_t default_tvoc,  uint16_t default_co2, 
  uint16_t ID);

void EN2_init(void * const pVoid);
void EN2_get_tvoc(void * const pVoid);

/* -------------------------------------------------------------------------
 *  Sensor Specific functions / constants
 * ------------------------------------------------------------------------- */

typedef uint32_t fix16_t;

/* SGP40 cmds */
#define ENVIRONMENT2_SGP40_CMD_MEASURE_RAW                       0x260F
#define ENVIRONMENT2_SGP40_CMD_MEASURE_TEST                      0x280E
#define ENVIRONMENT2_SGP40_CMD_HEATER_OFF                        0x3615
#define ENVIRONMENT2_SGP40_CMD_SOFT_RESET                        0x0006

/* SHT40 cmds */
#define ENVIRONMENT2_SHT40_CMD_MEASURE_T_RH_HIGH_PRECISION       0xFD
#define ENVIRONMENT2_SHT40_CMD_MEASURE_T_RH_MEDIUM_PRECISION     0xF6
#define ENVIRONMENT2_SHT40_CMD_MEASURE_T_RH_LOWEST_PRECISION     0xE0
#define ENVIRONMENT2_SHT40_CMD_READ_SERIAL                       0x89
#define ENVIRONMENT2_SHT40_CMD_SOFT_RESET                        0x94
#define ENVIRONMENT2_SHT40_CMD_ACTIVATE_HIGHEST_HEATER_1SEC      0x39
#define ENVIRONMENT2_SHT40_CMD_ACTIVATE_HIGHEST_HEATER_0_1SEC    0x32
#define ENVIRONMENT2_SHT40_CMD_ACTIVATE_MEDIUM_HEATER_1SEC       0x2F
#define ENVIRONMENT2_SHT40_CMD_ACTIVATE_MEDIUM_HEATER_0_1SEC     0x24
#define ENVIRONMENT2_SHT40_CMD_ACTIVATE_LOWEST_HEATER_1SEC       0x1E
#define ENVIRONMENT2_SHT40_CMD_ACTIVATE_LOWEST_HEATER_0_1SEC     0x15

/* device addresses */
#define ENVIRONMENT2_SGP40_SET_DEV_ADDR                          0x59u
#define ENVIRONMENT2_SHT40_SET_DEV_ADDR                          0x44u

/* device selection */
#define ENVIRONMENT2_SEL_SGP40                                   0x00u
#define ENVIRONMENT2_SEL_SHT40                                   0x01u

/* fixed point macros & voc algorithm constants (unchanged) */
#define F16(x) \
    ((fix16_t)(((x) >= 0) ? ((x)*65536.0 + 0.5) : ((x)*65536.0 - 0.5)))
#define VocAlgorithm_SAMPLING_INTERVAL (1.)
#define VocAlgorithm_INITIAL_BLACKOUT (45.)
#define VocAlgorithm_VOC_INDEX_GAIN (230.)
#define VocAlgorithm_SRAW_STD_INITIAL (50.)
#define VocAlgorithm_SRAW_STD_BONUS (220.)
#define VocAlgorithm_TAU_MEAN_VARIANCE_HOURS (12.)
#define VocAlgorithm_TAU_INITIAL_MEAN (20.)
#define VocAlgorithm_INIT_DURATION_MEAN ((3600. * 0.75))
#define VocAlgorithm_INIT_TRANSITION_MEAN (0.01)
#define VocAlgorithm_TAU_INITIAL_VARIANCE (2500.)
#define VocAlgorithm_INIT_DURATION_VARIANCE ((3600. * 1.45))
#define VocAlgorithm_INIT_TRANSITION_VARIANCE (0.01)
#define VocAlgorithm_GATING_THRESHOLD (340.)
#define VocAlgorithm_GATING_THRESHOLD_INITIAL (510.)
#define VocAlgorithm_GATING_THRESHOLD_TRANSITION (0.09)
#define VocAlgorithm_GATING_MAX_DURATION_MINUTES ((60. * 3.))
#define VocAlgorithm_GATING_MAX_RATIO (0.3)
#define VocAlgorithm_SIGMOID_L (500.)
#define VocAlgorithm_SIGMOID_K (-0.0065)
#define VocAlgorithm_SIGMOID_X0 (213.)
#define VocAlgorithm_VOC_INDEX_OFFSET_DEFAULT (100.)
#define VocAlgorithm_LP_TAU_FAST (20.0)
#define VocAlgorithm_LP_TAU_SLOW (500.0)
#define VocAlgorithm_LP_ALPHA (-0.2)
#define VocAlgorithm_PERSISTENCE_UPTIME_GAMMA ((3. * 3600.))
#define VocAlgorithm_MEAN_VARIANCE_ESTIMATOR__GAMMA_SCALING (64.)
#define VocAlgorithm_MEAN_VARIANCE_ESTIMATOR__FIX16_MAX (32767.)

#define ENVIRONMENT2_SGP40_TEST_PASSED                           0xD400u
#define ENVIRONMENT2_SGP40_TEST_FAILED                           0x4B00u

/* VOC algorithm parameters struct */
typedef struct {
    fix16_t mVoc_Index_Offset;
    fix16_t mTau_Mean_Variance_Hours;
    fix16_t mGating_Max_Duration_Minutes;
    fix16_t mSraw_Std_Initial;
    fix16_t mUptime;
    fix16_t mSraw;
    fix16_t mVoc_Index;
    fix16_t m_Mean_Variance_Estimator__Gating_Max_Duration_Minutes;
    bool m_Mean_Variance_Estimator___Initialized;
    fix16_t m_Mean_Variance_Estimator___Mean;
    fix16_t m_Mean_Variance_Estimator___Sraw_Offset;
    fix16_t m_Mean_Variance_Estimator___Std;
    fix16_t m_Mean_Variance_Estimator___Gamma;
    fix16_t m_Mean_Variance_Estimator___Gamma_Initial_Mean;
    fix16_t m_Mean_Variance_Estimator___Gamma_Initial_Variance;
    fix16_t m_Mean_Variance_Estimator__Gamma_Mean;
    fix16_t m_Mean_Variance_Estimator__Gamma_Variance;
    fix16_t m_Mean_Variance_Estimator___Uptime_Gamma;
    fix16_t m_Mean_Variance_Estimator___Uptime_Gating;
    fix16_t m_Mean_Variance_Estimator___Gating_Duration_Minutes;
    fix16_t m_Mean_Variance_Estimator___Sigmoid__L;
    fix16_t m_Mean_Variance_Estimator___Sigmoid__K;
    fix16_t m_Mean_Variance_Estimator___Sigmoid__X0;
    fix16_t m_Mox_Model__Sraw_Std;
    fix16_t m_Mox_Model__Sraw_Mean;
    fix16_t m_Sigmoid_Scaled__Offset;
    fix16_t m_Adaptive_Lowpass__A1;
    fix16_t m_Adaptive_Lowpass__A2;
    bool m_Adaptive_Lowpass___Initialized;
    fix16_t m_Adaptive_Lowpass___X1;
    fix16_t m_Adaptive_Lowpass___X2;
    fix16_t m_Adaptive_Lowpass___X3;
} environment2_voc_algorithm_params;

/* -------------------------------------------------------------------------
 *  Public sensor-specific API (now error-checked)
 * ------------------------------------------------------------------------- */
EN2_Status_t EN2_write(uint8_t device, uint16_t data);
EN2_Status_t EN2_read(uint8_t device, uint16_t data, uint8_t *read_buf);
EN2_Status_t EN2_get_temp_hum(uint8_t device, float *humidity, float *temperature);
EN2_Status_t EN2_get_air_quality(uint8_t device, uint16_t *air_quality);
EN2_Status_t EN2_SGP40_measure_test(uint8_t device, uint16_t *result);
EN2_Status_t EN2_SGP40_heater_off(uint8_t device);
EN2_Status_t EN2_SGP40_softreset(uint8_t device);
EN2_Status_t EN2_voc_algorithm_configuration(environment2_voc_algorithm_params* params);
EN2_Status_t EN2_config_sensors(void);
EN2_Status_t EN2_voc_algorithm_process(environment2_voc_algorithm_params *params, int32_t sraw, int32_t *voc_index);
EN2_Status_t EN2_measure_voc_index_with_rh_t(uint8_t device, int32_t *voc_index, int32_t *relative_humidity, int32_t *temperature);
EN2_Status_t EN2_get_voc_index(uint8_t device, int32_t *voc_index);
EN2_Status_t EN2sensor_init(void);

#endif /* EN2_H */
