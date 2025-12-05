#include "EN2.h"
#include <stdio.h> /* for printf */
#include <string.h> /* for memset */

/* -------------------------------------------------------------------------
 *  Generic Sensor Interface Implementation
 * ------------------------------------------------------------------------- */

void EN2_ctor(struct EN2 * const self, 
                    uint16_t default_tvoc,  uint16_t default_co2, 
                    uint16_t ID)
{
    static struct AQSensorVtbl const vtbl =
    {
        &EN2_init,
        NULL,
        &EN2_get_tvoc,  
        NULL
    };
    
    AQSensor_ctor(&self -> parent, default_tvoc, default_co2, ID);
    self -> parent.vptr = &vtbl;
}

void EN2_init(void * const pVoid){
    
    EN2sensor_init();  
}


void EN2_get_tvoc(void * const pVoid){
    
    struct EN2 * const self_ = (struct EN2 *)pVoid;
    EN2_get_air_quality(ENVIRONMENT2_SEL_SGP40,&self_->parent.TVOC);
}

//--------------------------------------------------------------------------------------------------------------
//**************************************************************************************************************
//--------------------------------------------------------------------------------------------------------------
//                                 Sensor Specific functions
//--------------------------------------------------------------------------------------------------------------
//**************************************************************************************************************
//--------------------------------------------------------------------------------------------------------------

#define FIX16_MAXIMUM         0x7FFFFFFF
#define FIX16_MINIMUM         0x80000000
#define FIX16_OVERFLOW        0x80000000
#define FIX16_ONE             0x00010000

static environment2_voc_algorithm_params voc_algorithm_params;
static fix16_t dev_voc_algorithm_mox_model_process ( environment2_voc_algorithm_params* params, fix16_t sraw );
static fix16_t dev_voc_algorithm_sigmoid_scaled_process ( environment2_voc_algorithm_params *params, fix16_t sample );
static fix16_t dev_voc_algorithm_adaptive_lowpass_process ( environment2_voc_algorithm_params *params, fix16_t sample );
static fix16_t dev_voc_algorithm_mean_variance_estimator_sigmoid_process ( environment2_voc_algorithm_params *params, fix16_t sample );
static void dev_voc_algorithm_mean_variance_estimator_calculate_gamma ( environment2_voc_algorithm_params *params, fix16_t voc_index_from_prior );
static void dev_voc_algorithm_mean_variance_estimator_process ( environment2_voc_algorithm_params *params, fix16_t sraw, fix16_t voc_index_from_prior );
static void dev_voc_algorithm_mox_model_set_parameters ( environment2_voc_algorithm_params *params, fix16_t sraw_std, fix16_t sraw_mean );
static void dev_voc_algorithm_mean_variance_estimator_set_parameters ( environment2_voc_algorithm_params *params, 
                                                                       fix16_t std_initial, 
                                                                       fix16_t tau_mean_variance_hours, 
                                                                       fix16_t gating_max_duration_minutes );
static void dev_voc_algorithm_mean_variance_estimator_sigmoid_set_parameters ( environment2_voc_algorithm_params *params, fix16_t l_val, fix16_t x0_val, fix16_t k_val );
static void dev_voc_algorithm_mean_variance_estimator_sigmoid_init ( environment2_voc_algorithm_params *params );
static void dev_voc_algorithm_mean_variance_estimator_init_instances ( environment2_voc_algorithm_params *params );
static void dev_voc_algorithm_mean_variance_estimator_init ( environment2_voc_algorithm_params *params );
static void dev_voc_algorithm_mox_model_init ( environment2_voc_algorithm_params *params );
static fix16_t dev_voc_algorithm_mean_variance_estimator_get_std ( environment2_voc_algorithm_params *params );
static fix16_t dev_voc_algorithm_mean_variance_estimator_get_mean ( environment2_voc_algorithm_params *params );
static void dev_voc_algorithm_sigmoid_scaled_set_parameters ( environment2_voc_algorithm_params *params, fix16_t offset );
static void dev_voc_algorithm_sigmoid_scaled_init ( environment2_voc_algorithm_params *params );
static void dev_voc_algorithm_adaptive_lowpass_set_parameters ( environment2_voc_algorithm_params *params );
static void dev_voc_algorithm_adaptive_lowpass_init ( environment2_voc_algorithm_params *params );
static void dev_voc_algorithm_init_instances ( environment2_voc_algorithm_params *params );

static fix16_t dev_fix16_div ( fix16_t in_arg0, fix16_t in_arg1 );
static fix16_t dev_fix16_mul ( fix16_t in_arg0, fix16_t in_arg1 );
static fix16_t dev_fix16_sqrt ( fix16_t in_value );
static fix16_t dev_fix16_exp ( fix16_t in_value );
static uint8_t dev_calc_crc ( uint8_t data_0, uint8_t data_1 );




//--------------------------------------------------------------------------------------------------------------
//**************************************************************************************************************
//--------------------------------------------------------------------------------------------------------------
//                                 EN2 Sensor functions                                              
//--------------------------------------------------------------------------------------------------------------
//**************************************************************************************************************
//--------------------------------------------------------------------------------------------------------------
fix16_t fix16_from_int ( int32_t a ) {
    return a * FIX16_ONE;
}

int32_t fix16_cast_to_int ( fix16_t a ) {
    return ( a >> 16 );
}

static uint8_t dev_calc_crc ( uint8_t data_0, uint8_t data_1 );
static void dev_voc_algorithm_init_instances ( environment2_voc_algorithm_params *params );

EN2_Status_t EN2_write(uint8_t device, uint16_t data)
{
    uint8_t data_buf[2];
    uint8_t address;
    bool ok = false;

    if (device == ENVIRONMENT2_SEL_SGP40)
    {
        address = ENVIRONMENT2_SGP40_SET_DEV_ADDR;
        data_buf[1] = (uint8_t)(data >>8);
        data_buf[2] = (uint8_t)(data);

        ok = Generic_Comms.Write(address, data_buf, 2U);

        (void)__delay_ms(200);

        if (ok != true)
        {
            (void)printf("EN2_write(): I2C write failed (SGP40)\n");
            return EN2_ERROR_COMM;
        }
        return EN2_OK;
    }

    (void)printf("EN2_write(): invalid device selection %u\n", (unsigned)device);
    return EN2_ERROR_PARAM;
}

EN2_Status_t EN2_read(uint8_t device, uint16_t data, uint8_t *read_buf)
{
    if (read_buf == NULL)
    {
        (void)printf("EN2_read(): NULL read_buf\n");
        return EN2_ERROR_NULL_PTR;
    }

    uint8_t data_buf[2];
    uint8_t address;
    bool ok = false;

    if (device == ENVIRONMENT2_SEL_SGP40)
    {
        address = ENVIRONMENT2_SGP40_SET_DEV_ADDR;
        data_buf[1] = (uint8_t)(data >>8);
        data_buf[2] = (uint8_t)(data);

        ok = Generic_Comms.WriteRead(address, data_buf, 2U, read_buf, 3U);
        (void)__delay_ms(200);

        if (ok != true)
        {
            (void)printf("EN2_read(): I2C WriteRead failed (SGP40)\n");
            return EN2_ERROR_COMM;
        }

        return EN2_OK;
    }
    else if (device == ENVIRONMENT2_SEL_SHT40)
    {
        address = ENVIRONMENT2_SHT40_SET_DEV_ADDR;
        data_buf[0] = (uint8_t)(data & 0xFF);

        ok = Generic_Comms.WriteRead(address, data_buf, 1U, read_buf, 6U);
        (void)__delay_ms(200);
        if (ok != true)
        {
            (void)printf("EN2_read(): I2C WriteRead failed (SHT40)\n");
            return EN2_ERROR_COMM;
        }

        return EN2_OK;
    }

    (void)printf("EN2_read(): invalid device selection %u\n", (unsigned)device);
    return EN2_ERROR_PARAM;
}

/* -------------------------------------------------------------------------
 *  High level sensor operations
 * ------------------------------------------------------------------------- */

EN2_Status_t EN2_get_temp_hum(uint8_t device, float *humidity, float *temperature)
{
    if ((humidity == NULL) || (temperature == NULL))
    {
        (void)printf("EN2_get_temp_hum(): NULL pointer\n");
        return EN2_ERROR_NULL_PTR;
    }

    if (device != ENVIRONMENT2_SEL_SHT40)
    {
        (void)printf("EN2_get_temp_hum(): invalid device selection %u\n", (unsigned)device);
        return EN2_ERROR_PARAM;
    }

    uint8_t data_buf[1];
    uint8_t read_buf[6];
    uint16_t t_ticks = 0U;
    uint16_t rh_ticks = 0U;
    uint8_t address = ENVIRONMENT2_SHT40_SET_DEV_ADDR;

    data_buf[0] = (uint8_t)ENVIRONMENT2_SHT40_CMD_MEASURE_T_RH_HIGH_PRECISION;

    /* Perform write/read and check comms + CRCs */
    if (Generic_Comms.Write(address, data_buf, 1U) != true)
    {
        (void)printf("EN2_get_temp_hum(): I2C write failed (SHT40)\n");
        return EN2_ERROR_COMM;
    }

    (void)__delay_ms(200);

    if (Generic_Comms.Read(address, read_buf, 6U) != true)
    {
        (void)printf("EN2_get_temp_hum(): I2C read failed (SHT40)\n");
        return EN2_ERROR_COMM;
    }

    (void)__delay_ms(200);


    t_ticks = (uint16_t)read_buf[0];
    t_ticks <<= 8;
    t_ticks |= (uint16_t)read_buf[1];

    rh_ticks = (uint16_t)read_buf[3];
    rh_ticks <<= 8;
    rh_ticks |= (uint16_t)read_buf[4];

    /* Convert per SHT datasheet */
    *temperature = (175.0f * ((float)t_ticks / 65535.0f)) - 45.0f;
    *humidity = (125.0f * ((float)rh_ticks / 65535.0f)) - 6.0f;


    return EN2_OK;
}

EN2_Status_t EN2_get_air_quality(uint8_t device, uint16_t *air_quality)
{
    if (air_quality == NULL)
    {
        (void)printf("EN2_get_air_quality(): NULL pointer\n");
        return EN2_ERROR_NULL_PTR;
    }

    if (device != ENVIRONMENT2_SEL_SGP40)
    {
        (void)printf("EN2_get_air_quality(): invalid device selection %u\n", (unsigned)device);
        return EN2_ERROR_PARAM;
    }

    uint8_t address = ENVIRONMENT2_SGP40_SET_DEV_ADDR;
    uint8_t data_buf[8];
    uint8_t read_buf[3];
    uint16_t tmp_hum = 0U;
    uint16_t tmp_temp = 0U;
    float temperature = 0.0f;
    float humidity = 0.0f;
    uint16_t result = 0U;

    /* Get current humidity/temperature first (validate) */
    EN2_Status_t st = EN2_get_temp_hum(ENVIRONMENT2_SEL_SHT40, &humidity, &temperature);
    if (st != EN2_OK)
    {
        (void)printf("EN2_get_air_quality(): failed to get temp/hum (err %d)\n", (int)st);
        return st; 
    }

    memset(data_buf, 0, sizeof(data_buf));
    data_buf[0] = (uint8_t)(ENVIRONMENT2_SGP40_CMD_MEASURE_RAW >> 8);
    data_buf[1] = (uint8_t)(ENVIRONMENT2_SGP40_CMD_MEASURE_RAW & 0xFF);

    tmp_hum = (uint16_t)(humidity * (65535.0f / 100.0f));
    data_buf[2] = (uint8_t)(tmp_hum >> 8);
    data_buf[3] = (uint8_t)(tmp_hum & 0xFF);
    data_buf[4] = dev_calc_crc(data_buf[2], data_buf[3]);

    tmp_temp = (uint16_t)((temperature + 45.0f) * (65535.0f / 175.0f));
    data_buf[5] = (uint8_t)(tmp_temp >> 8);
    data_buf[6] = (uint8_t)(tmp_temp & 0xFF);
    data_buf[7] = dev_calc_crc(data_buf[5], data_buf[6]);

    if (Generic_Comms.Write(address, data_buf, 8U) != true)
    {
        (void)printf("EN2_get_air_quality(): I2C write failed (SGP40)\n");
        return EN2_ERROR_COMM;
    }

    (void)__delay_ms(200);

    if (Generic_Comms.Read(address, read_buf, 3U) != true)
    {
        (void)printf("EN2_get_air_quality(): I2C read failed (SGP40)\n");
        return EN2_ERROR_COMM;
    }

    (void)__delay_ms(200);


    result = (uint16_t)read_buf[0];
    result <<= 8;
    result |= (uint16_t)read_buf[1];

    *air_quality = result;
    return EN2_OK;
}

EN2_Status_t EN2_SGP40_measure_test(uint8_t device, uint16_t *result)
{
    if (result == NULL)
    {
        (void)printf("EN2_SGP40_measure_test(): NULL pointer\n");
        return EN2_ERROR_NULL_PTR;
    }

    if (device != ENVIRONMENT2_SEL_SGP40)
    {
        (void)printf("EN2_SGP40_measure_test(): invalid device\n");
        return EN2_ERROR_PARAM;
    }

    uint8_t data_buf[2];
    uint8_t read_buf[3];
    uint8_t address = ENVIRONMENT2_SGP40_SET_DEV_ADDR;

    data_buf[0] = (uint8_t)(ENVIRONMENT2_SGP40_CMD_MEASURE_TEST >> 8);
    data_buf[1] = (uint8_t)(ENVIRONMENT2_SGP40_CMD_MEASURE_TEST & 0xFF);

    if (Generic_Comms.Write(address, data_buf, 2U) != true)
    {
        (void)printf("EN2_SGP40_measure_test(): I2C write failed\n");
        return EN2_ERROR_COMM;
    }

    (void)__delay_ms(250);

    if (Generic_Comms.Read(address, read_buf, 3U) != true)
    {
        (void)printf("EN2_SGP40_measure_test(): I2C read failed\n");
        return EN2_ERROR_COMM;
    }

    (void)__delay_ms(250);

    *result = (uint16_t)read_buf[0];
    *result <<= 8;
    *result |= (uint16_t)read_buf[1];

    return EN2_OK;
}

EN2_Status_t EN2_SGP40_heater_off(uint8_t device)
{
    if (device != ENVIRONMENT2_SEL_SGP40)
    {
        (void)printf("EN2_SGP40_heater_off(): invalid device\n");
        return EN2_ERROR_PARAM;
    }

    uint8_t address = ENVIRONMENT2_SGP40_SET_DEV_ADDR;
    uint8_t data_buf[2];
    data_buf[0] = (uint8_t)(ENVIRONMENT2_SGP40_CMD_HEATER_OFF >> 8);
    data_buf[1] = (uint8_t)(ENVIRONMENT2_SGP40_CMD_HEATER_OFF & 0xFF);

    if (Generic_Comms.Write(address, data_buf, 2U) != true)
    {
        (void)printf("EN2_SGP40_heater_off(): I2C write failed\n");
        return EN2_ERROR_COMM;
    }
    
    (void)__delay_ms(1000);

    return EN2_OK;
}

EN2_Status_t EN2_SGP40_softreset(uint8_t device)
{
    if (device != ENVIRONMENT2_SEL_SGP40)
    {
        (void)printf("EN2_SGP40_softreset(): invalid device\n");
        return EN2_ERROR_PARAM;
    }

    uint8_t address = ENVIRONMENT2_SGP40_SET_DEV_ADDR;
    uint8_t data_buf[2];
    data_buf[0] = (uint8_t)(ENVIRONMENT2_SGP40_CMD_SOFT_RESET >> 8);
    data_buf[1] = (uint8_t)(ENVIRONMENT2_SGP40_CMD_SOFT_RESET & 0xFF);

    if (Generic_Comms.Write(address, data_buf, 2U) != true)
    {
        (void)printf("EN2_SGP40_softreset(): I2C write failed\n");
        return EN2_ERROR_COMM;
    }
    return EN2_OK;
}

/* -------------------------------------------------------------------------
 *  VOC algorithm config / process (error-checked wrappers)
 * ------------------------------------------------------------------------- */

EN2_Status_t EN2_voc_algorithm_configuration(environment2_voc_algorithm_params* params)
{
    if (params == NULL)
    {
        (void)printf("EN2_voc_algorithm_configuration(): NULL pointer\n");
        return EN2_ERROR_NULL_PTR;
    }

    params->mVoc_Index_Offset = F16(VocAlgorithm_VOC_INDEX_OFFSET_DEFAULT);
    params->mTau_Mean_Variance_Hours = F16(VocAlgorithm_TAU_MEAN_VARIANCE_HOURS);
    params->mGating_Max_Duration_Minutes = F16(VocAlgorithm_GATING_MAX_DURATION_MINUTES);
    params->mSraw_Std_Initial = F16(VocAlgorithm_SRAW_STD_INITIAL);
    params->mUptime = F16(0.);
    params->mSraw = F16(0.);
    params->mVoc_Index = 0;
    /* internal initialization helpers (already defined below) */
    dev_voc_algorithm_init_instances(params);

    return EN2_OK;
}

EN2_Status_t EN2_config_sensors(void)
{
    EN2_Status_t st = EN2_voc_algorithm_configuration(&voc_algorithm_params);
    if (st != EN2_OK)
    {
        (void)printf("EN2_config_sensors(): voc configuration failed (err %d)\n", (int)st);
        return st;
    }
    return EN2_OK;
}

/* convert the original bool process to status return and check pointers */
EN2_Status_t EN2_voc_algorithm_process(environment2_voc_algorithm_params *params, int32_t sraw, int32_t *voc_index)
{
    if ((params == NULL) || (voc_index == NULL))
    {
        (void)printf("EN2_voc_algorithm_process(): NULL pointer\n");
        return EN2_ERROR_NULL_PTR;
    }

    if ((params->mUptime <= F16(VocAlgorithm_INITIAL_BLACKOUT)))
    {
        params->mUptime = (params->mUptime + F16(VocAlgorithm_SAMPLING_INTERVAL));
    }
    else
    {
        if (((sraw > 0) && (sraw < 65000)))
        {
            if ((sraw < 20001))
            {
                sraw = 20001;
            }
            else if ((sraw > 52767))
            {
                sraw = 52767;
            }
            params->mSraw = (fix16_from_int((sraw - 20000)));
        }
        params->mVoc_Index = dev_voc_algorithm_mox_model_process(params, params->mSraw);
        params->mVoc_Index = dev_voc_algorithm_sigmoid_scaled_process(params, params->mVoc_Index);
        params->mVoc_Index = dev_voc_algorithm_adaptive_lowpass_process(params, params->mVoc_Index);

        if ((params->mVoc_Index < F16(0.5)))
        {
            params->mVoc_Index = F16(0.5);
        }

        if ((params->mSraw > F16(0.)))
        {
            dev_voc_algorithm_mean_variance_estimator_process(params, params->mSraw, params->mVoc_Index);
            dev_voc_algorithm_mox_model_set_parameters(params,
                                                      dev_voc_algorithm_mean_variance_estimator_get_std(params),
                                                      dev_voc_algorithm_mean_variance_estimator_get_mean(params));
        }
    }

    *voc_index = (fix16_cast_to_int((params->mVoc_Index + F16(0.5))));
    return EN2_OK;
}

EN2_Status_t EN2_measure_voc_index_with_rh_t(uint8_t device, int32_t *voc_index, int32_t *relative_humidity, int32_t *temperature)
{
    if ((voc_index == NULL) || (relative_humidity == NULL) || (temperature == NULL))
    {
        (void)printf("EN2_measure_voc_index_with_rh_t(): NULL pointer\n");
        return EN2_ERROR_NULL_PTR;
    }

    if (device != ENVIRONMENT2_SEL_SHT40)
    {
        (void)printf("EN2_measure_voc_index_with_rh_t(): invalid device (expect SHT40)\n");
        return EN2_ERROR_PARAM;
    }

    uint16_t sraw = 0U;
    EN2_Status_t st;
//    int32_t temp_val = 0;
//    int32_t rh_val = 0;

    /* read temperature/humidity (SHT40) */
    st = EN2_get_temp_hum(ENVIRONMENT2_SEL_SHT40, (float *)relative_humidity, (float *)temperature);
    /* Note: EN2_get_temp_hum returns floats. We still propagate errors - user can convert units later */
    if (st != EN2_OK)
    {
        (void)printf("EN2_measure_voc_index_with_rh_t(): EN2_get_temp_hum failed (err %d)\n", (int)st);
        return st;
    }

    /* Get raw air quality from SGP40 */
    st = EN2_get_air_quality(ENVIRONMENT2_SEL_SGP40, &sraw);
    if (st != EN2_OK)
    {
        (void)printf("EN2_measure_voc_index_with_rh_t(): EN2_get_air_quality failed (err %d)\n", (int)st);
        return st;
    }
    (void)__delay_ms(500);

    /* process VOC algorithm */
    st = EN2_voc_algorithm_process(&voc_algorithm_params, (int32_t)sraw, voc_index);
    if (st != EN2_OK)
    {
        (void)printf("EN2_measure_voc_index_with_rh_t(): EN2_voc_algorithm_process failed (err %d)\n", (int)st);
        return st;
    }

    return EN2_OK;
}

EN2_Status_t EN2_get_voc_index(uint8_t device, int32_t *voc_index)
{
    if (voc_index == NULL)
    {
        (void)printf("EN2_get_voc_index(): NULL pointer\n");
        return EN2_ERROR_NULL_PTR;
    }

    /* read raw sraw */
    uint16_t sraw = 0U;
    EN2_Status_t st = EN2_get_air_quality(ENVIRONMENT2_SEL_SGP40, &sraw);
    if (st != EN2_OK)
    {
        (void)printf("EN2_get_voc_index(): EN2_get_air_quality failed (err %d)\n", (int)st);
        return st;
    }
    (void)__delay_ms(500);

    st = EN2_voc_algorithm_process(&voc_algorithm_params, (int32_t)sraw, voc_index);
    if (st != EN2_OK)
    {
        (void)printf("EN2_get_voc_index(): EN2_voc_algorithm_process failed (err %d)\n", (int)st);
        return st;
    }

    return EN2_OK;
}

/* -------------------------------------------------------------------------
 *  Initialization sequence
 * ------------------------------------------------------------------------- */

EN2_Status_t EN2sensor_init(void)
{
    (void)printf("Environment Click 2 Sensor Initialization started\n");
    (void)printf("Sensor Initializing...\n");

    uint16_t test_result = 0U;
    EN2_Status_t st = EN2_SGP40_measure_test(ENVIRONMENT2_SEL_SGP40, &test_result);
    if (st != EN2_OK)
    {
        (void)printf("EN2sensor_init(): measure test failed (comm err)\n");
        return st;
    }

    if (test_result == ENVIRONMENT2_SGP40_TEST_PASSED)
    {
        (void)printf("All tests passed Successfully\n");
        (void)__delay_ms(100);

        st = EN2_SGP40_heater_off(ENVIRONMENT2_SEL_SGP40);
        if (st != EN2_OK)
        {
            (void)printf("EN2sensor_init(): heater off failed (err %d)\n", (int)st);
            return st;
        }
        (void)printf("Heater off\n");
        (void)__delay_ms(100);

        st = EN2_config_sensors();
        if (st != EN2_OK)
        {
            (void)printf("EN2sensor_init(): sensor configuration failed (err %d)\n", (int)st);
            return st;
        }
        (void)printf("Sensor EN2 configuration successful\n\n\n");
        (void)__delay_ms(100);
    }
    else
    {
        (void)printf("One or more tests have Failed (0x%04X)\n", (unsigned)test_result);
        (void)printf("Sensor EN2 configuration failed\n\n\n");
        return EN2_ERROR_COMM;
    }

    return EN2_OK;
}





static uint8_t dev_calc_crc ( uint8_t data_0, uint8_t data_1 ) {
    uint8_t i_cnt;
    uint8_t j_cnt;
    uint8_t crc_data[ 2 ];
    uint8_t crc = 0xFF;
    
    crc_data[ 0 ] = data_0;
    crc_data[ 1 ] = data_1;
    
    for ( i_cnt = 0; i_cnt < 2; i_cnt++ ) {
        crc ^= crc_data[ i_cnt ];

        for ( j_cnt = 8; j_cnt > 0; --j_cnt ) {
            if ( crc & 0x80 ) {
                crc = ( crc << 1 ) ^ 0x31u;
            } else {
                crc = ( crc << 1 );
            }
        }
    }

    return crc;
}


static void dev_voc_algorithm_init_instances ( environment2_voc_algorithm_params *params ) {
    dev_voc_algorithm_mean_variance_estimator_init( params );
    dev_voc_algorithm_mean_variance_estimator_set_parameters( params, params->mSraw_Std_Initial, params->mTau_Mean_Variance_Hours, params->mGating_Max_Duration_Minutes );
    dev_voc_algorithm_mox_model_init( params );
    dev_voc_algorithm_mox_model_set_parameters( params, dev_voc_algorithm_mean_variance_estimator_get_std( params ), dev_voc_algorithm_mean_variance_estimator_get_mean( params ) );
    dev_voc_algorithm_sigmoid_scaled_init( params );
    dev_voc_algorithm_sigmoid_scaled_set_parameters( params, params->mVoc_Index_Offset );
    dev_voc_algorithm_adaptive_lowpass_init( params );
    dev_voc_algorithm_adaptive_lowpass_set_parameters( params );
}

static void dev_voc_algorithm_mean_variance_estimator_init ( environment2_voc_algorithm_params* params ) {
    dev_voc_algorithm_mean_variance_estimator_set_parameters( params, F16( 0. ), F16( 0. ), F16( 0. ) );
    dev_voc_algorithm_mean_variance_estimator_init_instances( params );
}

static void dev_voc_algorithm_mean_variance_estimator_set_parameters ( environment2_voc_algorithm_params* params, 
                                                                    fix16_t std_initial, 
                                                                    fix16_t tau_mean_variance_hours, 
                                                                    fix16_t gating_max_duration_minutes ) {

    params->m_Mean_Variance_Estimator__Gating_Max_Duration_Minutes = gating_max_duration_minutes;
    params->m_Mean_Variance_Estimator___Initialized = false;
    params->m_Mean_Variance_Estimator___Mean = F16( 0. );
    params->m_Mean_Variance_Estimator___Sraw_Offset = F16( 0. );
    params->m_Mean_Variance_Estimator___Std = std_initial;
    params->m_Mean_Variance_Estimator___Gamma = ( dev_fix16_div( F16( ( VocAlgorithm_MEAN_VARIANCE_ESTIMATOR__GAMMA_SCALING * ( VocAlgorithm_SAMPLING_INTERVAL / 3600. ) ) ),
                                                ( tau_mean_variance_hours + F16( ( VocAlgorithm_SAMPLING_INTERVAL / 3600. ) ) ) ) );
    params->m_Mean_Variance_Estimator___Gamma_Initial_Mean = F16( ( ( VocAlgorithm_MEAN_VARIANCE_ESTIMATOR__GAMMA_SCALING * VocAlgorithm_SAMPLING_INTERVAL ) / 
                                                                    ( VocAlgorithm_TAU_INITIAL_MEAN + VocAlgorithm_SAMPLING_INTERVAL ) ) );
    params->m_Mean_Variance_Estimator___Gamma_Initial_Variance = F16( ( ( VocAlgorithm_MEAN_VARIANCE_ESTIMATOR__GAMMA_SCALING * VocAlgorithm_SAMPLING_INTERVAL ) /
                                                                        ( VocAlgorithm_TAU_INITIAL_VARIANCE + VocAlgorithm_SAMPLING_INTERVAL ) ) );
    params->m_Mean_Variance_Estimator__Gamma_Mean = F16( 0. );
    params->m_Mean_Variance_Estimator__Gamma_Variance = F16( 0. );
    params->m_Mean_Variance_Estimator___Uptime_Gamma = F16( 0. );
    params->m_Mean_Variance_Estimator___Uptime_Gating = F16( 0. );
    params->m_Mean_Variance_Estimator___Gating_Duration_Minutes = F16( 0. );
}

static void dev_voc_algorithm_mox_model_init ( environment2_voc_algorithm_params* params ) {
    dev_voc_algorithm_mox_model_set_parameters( params, F16( 1. ), F16( 0. ) );
}

static void dev_voc_algorithm_mox_model_set_parameters ( environment2_voc_algorithm_params *params, fix16_t SRAW_STD, fix16_t SRAW_MEAN ) {
    params->m_Mox_Model__Sraw_Std = SRAW_STD;
    params->m_Mox_Model__Sraw_Mean = SRAW_MEAN;
}

static void dev_voc_algorithm_sigmoid_scaled_init ( environment2_voc_algorithm_params* params ) {
    dev_voc_algorithm_sigmoid_scaled_set_parameters( params, F16( 0. ) );
}

static void dev_voc_algorithm_sigmoid_scaled_set_parameters ( environment2_voc_algorithm_params* params, fix16_t offset ) {
    params->m_Sigmoid_Scaled__Offset = offset;
}


static void dev_voc_algorithm_adaptive_lowpass_init ( environment2_voc_algorithm_params *params ) {
    dev_voc_algorithm_adaptive_lowpass_set_parameters( params );
}

static void dev_voc_algorithm_adaptive_lowpass_set_parameters ( environment2_voc_algorithm_params* params ) {
    params->m_Adaptive_Lowpass__A1 = F16( ( VocAlgorithm_SAMPLING_INTERVAL / ( VocAlgorithm_LP_TAU_FAST + VocAlgorithm_SAMPLING_INTERVAL ) ) );
    params->m_Adaptive_Lowpass__A2 = F16( ( VocAlgorithm_SAMPLING_INTERVAL / ( VocAlgorithm_LP_TAU_SLOW + VocAlgorithm_SAMPLING_INTERVAL ) ) );
    params->m_Adaptive_Lowpass___Initialized = false;
}

static fix16_t dev_voc_algorithm_mean_variance_estimator_get_std ( environment2_voc_algorithm_params* params ) {
    return params->m_Mean_Variance_Estimator___Std;
}

static fix16_t dev_voc_algorithm_mean_variance_estimator_get_mean ( environment2_voc_algorithm_params* params ) {
    return ( params->m_Mean_Variance_Estimator___Mean + params->m_Mean_Variance_Estimator___Sraw_Offset );
}

static void dev_voc_algorithm_mean_variance_estimator_init_instances ( environment2_voc_algorithm_params* params ) {
    dev_voc_algorithm_mean_variance_estimator_sigmoid_init( params );
}

static void dev_voc_algorithm_mean_variance_estimator_sigmoid_init ( environment2_voc_algorithm_params* params ) {
    dev_voc_algorithm_mean_variance_estimator_sigmoid_set_parameters( params, F16( 0. ), F16( 0. ), F16( 0. ) );
}

static void dev_voc_algorithm_mean_variance_estimator_sigmoid_set_parameters ( environment2_voc_algorithm_params *params, fix16_t L, fix16_t X0, fix16_t K ) {

    params->m_Mean_Variance_Estimator___Sigmoid__L = L;
    params->m_Mean_Variance_Estimator___Sigmoid__K = K;
    params->m_Mean_Variance_Estimator___Sigmoid__X0 = X0;
}

static fix16_t dev_fix16_div ( fix16_t a, fix16_t b ) {
    if ( b == 0 )
        return FIX16_MINIMUM;

    uint32_t remainder = ( a >= 0 ) ? a : ( -a );
    uint32_t divider = ( b >= 0 ) ? b : ( -b );

    uint32_t quotient = 0;
    uint32_t _bit = 0x10000;

    while ( divider < remainder ) {
        divider <<= 1;
        _bit <<= 1;
    }

#ifndef FIXMATH_NO_OVERFLOW
    if ( !_bit )
        return FIX16_OVERFLOW;
#endif

    if ( divider & 0x80000000 ) {
        if ( remainder >= divider ) {
            quotient |= _bit;
            remainder -= divider;
        }
        divider >>= 1;
        _bit >>= 1;
    }

    while ( _bit && remainder ) {
        if ( remainder >= divider ) {
            quotient |= _bit;
            remainder -= divider;
        }

        remainder <<= 1;
        _bit >>= 1;
    }

#ifndef FIXMATH_NO_ROUNDING
    if ( remainder >= divider ) {
        quotient++;
    }
#endif

    fix16_t result = quotient;

    if ( ( a ^ b ) & 0x80000000 ) {
#ifndef FIXMATH_NO_OVERFLOW
        if ( result == FIX16_MINIMUM )
            return FIX16_OVERFLOW;
#endif

        result = -result;
    }

    return result;
}

static fix16_t dev_voc_algorithm_mox_model_process ( environment2_voc_algorithm_params *params, fix16_t sraw ) {

    return ( dev_fix16_mul( ( dev_fix16_div( ( sraw - params->m_Mox_Model__Sraw_Mean ), 
                                             ( -( params->m_Mox_Model__Sraw_Std + F16( VocAlgorithm_SRAW_STD_BONUS ) ) ) ) ), 
                                            F16(VocAlgorithm_VOC_INDEX_GAIN ) ) );
}

static fix16_t dev_voc_algorithm_sigmoid_scaled_process ( environment2_voc_algorithm_params *params, fix16_t sample ) {
    fix16_t x;
    fix16_t shift;

    x = ( dev_fix16_mul( F16( VocAlgorithm_SIGMOID_K ), ( sample - F16( VocAlgorithm_SIGMOID_X0 ) ) ) );
    if ( ( x < F16( -50. ) ) ) {
        return F16( VocAlgorithm_SIGMOID_L );
    } else if ( ( x > F16( 50. ) ) ) {
        return F16( 0. );
    } else {
        if ( ( sample >= F16( 0. ) ) ) {
            shift = ( dev_fix16_div( ( F16( VocAlgorithm_SIGMOID_L ) - ( dev_fix16_mul( F16( 5. ), params->m_Sigmoid_Scaled__Offset ) ) ), F16( 4. ) ) );
            return ( ( dev_fix16_div( ( F16(VocAlgorithm_SIGMOID_L ) + shift ), ( F16( 1. ) + dev_fix16_exp( x ) ) ) ) - shift );
        } else {
            return ( dev_fix16_mul( ( dev_fix16_div( params->m_Sigmoid_Scaled__Offset, 
                                                     F16( VocAlgorithm_VOC_INDEX_OFFSET_DEFAULT ) ) ), 
                                                    ( dev_fix16_div( F16( VocAlgorithm_SIGMOID_L ),
                                                    ( F16(1.) + dev_fix16_exp( x ) ) ) ) ) );
        }
    }
}

static fix16_t dev_voc_algorithm_adaptive_lowpass_process ( environment2_voc_algorithm_params *params, fix16_t sample ) {
    fix16_t abs_delta;
    fix16_t F1;
    fix16_t tau_a;
    fix16_t a3;

    if ( ( params->m_Adaptive_Lowpass___Initialized == false ) ) {
        params->m_Adaptive_Lowpass___X1 = sample;
        params->m_Adaptive_Lowpass___X2 = sample;
        params->m_Adaptive_Lowpass___X3 = sample;
        params->m_Adaptive_Lowpass___Initialized = true;
    }
    params->m_Adaptive_Lowpass___X1 = ( ( dev_fix16_mul( ( F16( 1. ) - params->m_Adaptive_Lowpass__A1 ),
                                                         params->m_Adaptive_Lowpass___X1)) + ( dev_fix16_mul( params->m_Adaptive_Lowpass__A1, sample ) ) );
    params->m_Adaptive_Lowpass___X2 = ( ( dev_fix16_mul( ( F16( 1. ) - params->m_Adaptive_Lowpass__A2 ),
                                                         params->m_Adaptive_Lowpass___X2 ) ) + ( dev_fix16_mul( params->m_Adaptive_Lowpass__A2, sample ) ) );
    abs_delta = ( params->m_Adaptive_Lowpass___X1 - params->m_Adaptive_Lowpass___X2 );
    if ( ( abs_delta < F16( 0. ) ) ) {
        abs_delta = ( -abs_delta );
    }
    F1 = dev_fix16_exp( ( dev_fix16_mul( F16( VocAlgorithm_LP_ALPHA ), abs_delta ) ) );
    tau_a = ( ( dev_fix16_mul( F16( ( VocAlgorithm_LP_TAU_SLOW - VocAlgorithm_LP_TAU_FAST ) ), F1 ) ) + F16( VocAlgorithm_LP_TAU_FAST ) );
    a3 = ( dev_fix16_div( F16( VocAlgorithm_SAMPLING_INTERVAL ), ( F16( VocAlgorithm_SAMPLING_INTERVAL ) + tau_a ) ) );
    params->m_Adaptive_Lowpass___X3 = ( ( dev_fix16_mul( ( F16( 1. ) - a3 ), params->m_Adaptive_Lowpass___X3 ) ) + ( dev_fix16_mul( a3, sample ) ) );
    
    return params->m_Adaptive_Lowpass___X3;
}

static void dev_voc_algorithm_mean_variance_estimator_process ( environment2_voc_algorithm_params *params, fix16_t sraw, fix16_t voc_index_from_prior ) {
    fix16_t delta_sgp;
    fix16_t c;
    fix16_t additional_scaling;

    if ( ( params->m_Mean_Variance_Estimator___Initialized == false ) ) {
        params->m_Mean_Variance_Estimator___Initialized = true;
        params->m_Mean_Variance_Estimator___Sraw_Offset = sraw;
        params->m_Mean_Variance_Estimator___Mean = F16( 0. );
    } else {
        if ( ( ( params->m_Mean_Variance_Estimator___Mean >= F16( 100. ) ) || ( params->m_Mean_Variance_Estimator___Mean <= F16( -100. ) ) ) ) {
            params->m_Mean_Variance_Estimator___Sraw_Offset = ( params->m_Mean_Variance_Estimator___Sraw_Offset + params->m_Mean_Variance_Estimator___Mean );
            params->m_Mean_Variance_Estimator___Mean = F16( 0. );
        }
        sraw = ( sraw - params->m_Mean_Variance_Estimator___Sraw_Offset );
        dev_voc_algorithm_mean_variance_estimator_calculate_gamma( params, voc_index_from_prior );
        delta_sgp = ( dev_fix16_div( ( sraw - params->m_Mean_Variance_Estimator___Mean ), F16( VocAlgorithm_MEAN_VARIANCE_ESTIMATOR__GAMMA_SCALING ) ) );
        if ( ( delta_sgp < F16( 0. ) ) ) {
            c = ( params->m_Mean_Variance_Estimator___Std - delta_sgp );
        } else {
            c = ( params->m_Mean_Variance_Estimator___Std + delta_sgp );
        }
        additional_scaling = F16( 1. );
        if ( ( c > F16( 1440. ) ) ) {
            additional_scaling = F16( 4. );
        }
        params->m_Mean_Variance_Estimator___Std = ( dev_fix16_mul( dev_fix16_sqrt( ( dev_fix16_mul( additional_scaling, 
                                                                                                    ( F16( VocAlgorithm_MEAN_VARIANCE_ESTIMATOR__GAMMA_SCALING) - params->m_Mean_Variance_Estimator__Gamma_Variance ) ) ) ),
                                                                   dev_fix16_sqrt( ( ( dev_fix16_mul( params->m_Mean_Variance_Estimator___Std, 
                                                                                                      ( dev_fix16_div ( params->m_Mean_Variance_Estimator___Std, ( dev_fix16_mul (F16(VocAlgorithm_MEAN_VARIANCE_ESTIMATOR__GAMMA_SCALING), additional_scaling ) ) ) ) ) ) + ( dev_fix16_mul( ( dev_fix16_div( ( dev_fix16_mul( params->m_Mean_Variance_Estimator__Gamma_Variance, delta_sgp ) ), additional_scaling ) ), delta_sgp ) ) ) ) ) );
        params->m_Mean_Variance_Estimator___Mean = ( params->m_Mean_Variance_Estimator___Mean + ( dev_fix16_mul( params->m_Mean_Variance_Estimator__Gamma_Mean, delta_sgp ) ) );
    }
}


static fix16_t dev_fix16_mul ( fix16_t inArg0, fix16_t inArg1 ) {
    int32_t A = ( inArg0 >> 16 ), C = ( inArg1 >> 16 );
    uint32_t B = ( inArg0 & 0xFFFF ), D = ( inArg1 & 0xFFFF );
    int32_t AC = A * C;
    int32_t AD_CB = A * D + C * B;
    uint32_t BD = B * D;
    int32_t product_hi = AC + ( AD_CB >> 16 );
    uint32_t ad_cb_temp = AD_CB << 16;
    uint32_t product_lo = BD + ad_cb_temp;
    
    if ( product_lo < BD )
        product_hi++;

#ifndef FIXMATH_NO_OVERFLOW
    if ( product_hi >> 31 != product_hi >> 15 )
        return FIX16_OVERFLOW;
#endif

#ifdef FIXMATH_NO_ROUNDING
    return ( product_hi << 16 ) | ( product_lo >> 16 );
#else
    uint32_t product_lo_tmp = product_lo;
    product_lo -= 0x8000;
    product_lo -= ( uint32_t )product_hi >> 31;
    if ( product_lo > product_lo_tmp )
        product_hi--;

    fix16_t result = ( product_hi << 16 ) | ( product_lo >> 16 );
    result += 1;
    return result;
#endif
}

static fix16_t dev_fix16_exp ( fix16_t x ) {
#define NUM_EXP_VALUES 4
    static const fix16_t exp_pos_values[ NUM_EXP_VALUES ] = { F16( 2.7182818 ), F16( 1.1331485 ), F16( 1.0157477 ), F16( 1.0019550 ) };
    static const fix16_t exp_neg_values[ NUM_EXP_VALUES ] = { F16( 0.3678794 ), F16( 0.8824969 ), F16( 0.9844964 ), F16( 0.9980488 ) };
    const fix16_t* exp_values;
    fix16_t res, arg;
    uint16_t i;

    if ( x >= F16( 10.3972 ) )
        return FIX16_MAXIMUM;
    if ( x <= F16( -11.7835 ) )
        return 0;

    if ( x < 0 ) {
        x = -x;
        exp_values = exp_neg_values;
    } else {
        exp_values = exp_pos_values;
    }

    res = FIX16_ONE;
    arg = FIX16_ONE;
    for ( i = 0; i < NUM_EXP_VALUES; i++ ) {
        while ( x >= arg ) {
            res = dev_fix16_mul( res, exp_values[ i ] );
            x -= arg;
        }
        arg >>= 3;
    }
    return res;
}


static void dev_voc_algorithm_mean_variance_estimator_calculate_gamma ( environment2_voc_algorithm_params *params, fix16_t voc_index_from_prior ) {
    fix16_t uptime_limit;
    fix16_t sigmoid_gamma_mean;
    fix16_t gamma_mean;
    fix16_t gating_threshold_mean;
    fix16_t sigmoid_gating_mean;
    fix16_t sigmoid_gamma_variance;
    fix16_t gamma_variance;
    fix16_t gating_threshold_variance;
    fix16_t sigmoid_gating_variance;

    uptime_limit = F16( ( VocAlgorithm_MEAN_VARIANCE_ESTIMATOR__FIX16_MAX - VocAlgorithm_SAMPLING_INTERVAL ) );
    if ( ( params->m_Mean_Variance_Estimator___Uptime_Gamma < uptime_limit ) ) {
        params->m_Mean_Variance_Estimator___Uptime_Gamma = ( params->m_Mean_Variance_Estimator___Uptime_Gamma + F16( VocAlgorithm_SAMPLING_INTERVAL ) );
    }
    if ( ( params->m_Mean_Variance_Estimator___Uptime_Gating < uptime_limit ) ) {
        params->m_Mean_Variance_Estimator___Uptime_Gating = ( params->m_Mean_Variance_Estimator___Uptime_Gating + F16( VocAlgorithm_SAMPLING_INTERVAL ) );
    }
    dev_voc_algorithm_mean_variance_estimator_sigmoid_set_parameters( params, F16( 1. ), F16( VocAlgorithm_INIT_DURATION_MEAN ), F16( VocAlgorithm_INIT_TRANSITION_MEAN ) );
    sigmoid_gamma_mean = dev_voc_algorithm_mean_variance_estimator_sigmoid_process( params, params->m_Mean_Variance_Estimator___Uptime_Gamma );
    gamma_mean = ( params->m_Mean_Variance_Estimator___Gamma + ( dev_fix16_mul( ( params->m_Mean_Variance_Estimator___Gamma_Initial_Mean - params->m_Mean_Variance_Estimator___Gamma ),
                                                                                sigmoid_gamma_mean ) ) );
    gating_threshold_mean = ( F16( VocAlgorithm_GATING_THRESHOLD ) + ( dev_fix16_mul( F16( ( VocAlgorithm_GATING_THRESHOLD_INITIAL - VocAlgorithm_GATING_THRESHOLD ) ),
                                                                                      dev_voc_algorithm_mean_variance_estimator_sigmoid_process( params, params->m_Mean_Variance_Estimator___Uptime_Gating ) ) ) );
    dev_voc_algorithm_mean_variance_estimator_sigmoid_set_parameters( params, F16( 1. ), gating_threshold_mean, F16( VocAlgorithm_GATING_THRESHOLD_TRANSITION ) );
    sigmoid_gating_mean = dev_voc_algorithm_mean_variance_estimator_sigmoid_process( params, voc_index_from_prior );
    params->m_Mean_Variance_Estimator__Gamma_Mean = ( dev_fix16_mul( sigmoid_gating_mean, gamma_mean ) );
    dev_voc_algorithm_mean_variance_estimator_sigmoid_set_parameters( params, F16( 1. ), F16( VocAlgorithm_INIT_DURATION_VARIANCE ), F16( VocAlgorithm_INIT_TRANSITION_VARIANCE ) );
    sigmoid_gamma_variance = dev_voc_algorithm_mean_variance_estimator_sigmoid_process( params, params->m_Mean_Variance_Estimator___Uptime_Gamma );
    gamma_variance = ( params->m_Mean_Variance_Estimator___Gamma + ( dev_fix16_mul( ( params->m_Mean_Variance_Estimator___Gamma_Initial_Variance - 
                                                                                      params->m_Mean_Variance_Estimator___Gamma ),
                                                                   ( sigmoid_gamma_variance - sigmoid_gamma_mean ) ) ) );
    gating_threshold_variance = ( F16( VocAlgorithm_GATING_THRESHOLD ) + ( dev_fix16_mul( F16( ( VocAlgorithm_GATING_THRESHOLD_INITIAL - VocAlgorithm_GATING_THRESHOLD ) ),
                                                                                          dev_voc_algorithm_mean_variance_estimator_sigmoid_process( params, params->m_Mean_Variance_Estimator___Uptime_Gating ) ) ) );
    dev_voc_algorithm_mean_variance_estimator_sigmoid_set_parameters( params, F16( 1. ), gating_threshold_variance, F16( VocAlgorithm_GATING_THRESHOLD_TRANSITION ) );
    sigmoid_gating_variance = dev_voc_algorithm_mean_variance_estimator_sigmoid_process( params, voc_index_from_prior );
    params->m_Mean_Variance_Estimator__Gamma_Variance = ( dev_fix16_mul( sigmoid_gating_variance, gamma_variance ) );
    params->m_Mean_Variance_Estimator___Gating_Duration_Minutes = ( params->m_Mean_Variance_Estimator___Gating_Duration_Minutes + 
                                                                   ( dev_fix16_mul( F16( ( VocAlgorithm_SAMPLING_INTERVAL / 60.) ), 
                                                                   ( ( dev_fix16_mul( ( F16( 1. ) - sigmoid_gating_mean ), 
                                                                       F16( ( 1. + VocAlgorithm_GATING_MAX_RATIO ) ) ) ) - F16( VocAlgorithm_GATING_MAX_RATIO ) ) ) ) );
    if ( ( params->m_Mean_Variance_Estimator___Gating_Duration_Minutes < F16( 0. ) ) ) {
        params->m_Mean_Variance_Estimator___Gating_Duration_Minutes = F16( 0. );
    }
    if ( ( params->m_Mean_Variance_Estimator___Gating_Duration_Minutes > params->m_Mean_Variance_Estimator__Gating_Max_Duration_Minutes ) ) {
        params->m_Mean_Variance_Estimator___Uptime_Gating = F16( 0. );
    }
}

static fix16_t dev_fix16_sqrt ( fix16_t x ) {
    uint32_t num = x;
    uint32_t result = 0;
    uint32_t _bit;
    uint8_t n;

    _bit = ( uint32_t )1 << 30;
    while ( _bit > num )
        _bit >>= 2;


    for ( n = 0; n < 2; n++ ) {
        while ( _bit ) {
            if ( num >= result + _bit ) {
                num -= result + _bit;
                result = ( result >> 1 ) + _bit;
            } else {
                result = ( result >> 1 );
            }
            _bit >>= 2;
        }

        if ( n == 0 ) {
            if ( num > 65535 ) {
                num -= result;
                num = ( num << 16 ) - 0x8000;
                result = ( result << 16 ) + 0x8000;
            } else {
                num <<= 16;
                result <<= 16;
            }

            _bit = 1 << 14;
        }
    }

#ifndef FIXMATH_NO_ROUNDING
    if ( num > result ) {
        result++;
    }
#endif

    return ( fix16_t )result;
}

static fix16_t dev_voc_algorithm_mean_variance_estimator_sigmoid_process ( environment2_voc_algorithm_params *params, fix16_t sample ) {
    fix16_t x;

    x = ( dev_fix16_mul( params->m_Mean_Variance_Estimator___Sigmoid__K, ( sample - params->m_Mean_Variance_Estimator___Sigmoid__X0 ) ) );
    if ( ( x < F16( -50. ) ) ) {
        return params->m_Mean_Variance_Estimator___Sigmoid__L;
    } else if ( ( x > F16( 50. ) ) ) {
        return F16( 0. );
    } else {
        return ( dev_fix16_div( params->m_Mean_Variance_Estimator___Sigmoid__L, ( F16( 1. ) + dev_fix16_exp( x ) ) ) );
    }
}