#ifndef AQI_H
#define AQI_H
/*******************************************************************************

DISCLAIMER OF LIABILITY REGARDING CODE AND SERVICES

Upon your request and for your convenience Microchip Technology Incorporated
("Microchip") has modified your code.  Your acceptance and/or use of this
modified code constitutes agreement to the terms and conditions of this notice.

CODE ACCOMPANYING THIS MESSAGE AND RELATED SERVICES ARE SUPPLIED BY MICROCHIP "AS IS".
NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
TO, IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE APPLY TO THIS CODE, ITS INTERACTION WITH MICROCHIP�S PRODUCTS,
COMBINATION WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

YOU ACKNOWLEDGE AND AGREE THAT, IN NO EVENT, SHALL MICROCHIP BE
LIABLE, WHETHER IN CONTRACT, WARRANTY, TORT (INCLUDING NEGLIGENCE OR BREACH OF
STATUTORY DUTY), STRICT LIABILITY, INDEMNITY, CONTRIBUTION, OR OTHERWISE, FOR ANY
INDIRECT, SPECIAL, PUNITIVE, EXEMPLARY, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE,
FOR COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE CODE, HOWSOEVER CAUSED,
EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.
TO THE FULLEST EXTENT ALLOWABLE BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS
IN ANY WAY RELATED TO THIS CODE, SHALL NOT EXCEED THE PRICE YOU PAID DIRECTLY TO
MICROCHIP SPECIFICALLY TO HAVE THIS CODE DEVELOPED.

You agree that you are solely responsible for testing the code and determining its
suitability.  Microchip has no obligation to modify, test, certify, or support the code.
*******************************************************************************/
#include "generic_driver_interfaces/generic_driver_interfaces.h"
#define _XTAL_FREQ    8000000
#include <stdio.h>

/* provide the file paths for pins and i2c based on your folder location"*/


/* map the pins to the functions (only needed for CCS811 and AQ11 sensors)
 * Example provided below: 
 * 
 * CCS811 connected to MikroE port 1 and AQ11 connected to MikroE port 2 of Curiosity Base board 
#define CCSINT1_GetValue()      GPIO_PA06_Get()      
#define CCSSS_SetHigh()         GPIO_PA18_Set()          
#define CCSSS_SetLow()          GPIO_PA18_Clear()  
#define CCSRST_SetHigh()        GPIO_PA07_Set()     
#define CCSRST_SetLow()         GPIO_PA07_Clear()   
 
 
#define AQSS_SetHigh()          GPIO_PA28_Set()
#define AQPWM_SetHigh()         GPIO_PA04_Set()
#define AQINT_GetValue()        GPIO_PA04_Get()         */
  


struct AQSensor
{
    struct AQSensorVtbl const * vptr;
    uint16_t TVOC;
    uint16_t CO2;
    uint16_t ID;
};


struct AQSensorVtbl
{
    /* Add custom functions to the vtbl, example provided below*/
    void(*init)(void * const pVoid);
    void(*get_id)(void * const pVoid);
    void(*get_tvoc)(void * const pVoid);
    void(*get_co2)(void * const pVoid);
    
};



void AQSensor_ctor(struct AQSensor * const self, uint16_t x, uint16_t y,uint16_t z);


/* define custom inline functions, example provided below*/
static inline void AQSensor_init(void * const pVoid){
    struct AQSensor * const self = (struct AQSensor *)pVoid;
    self->vptr->init(pVoid);
}

static inline void AQSensor_get_id(void * const pVoid){
    struct AQSensor * const self = (struct AQSensor *)pVoid;
    self->vptr->get_id(pVoid);
}

static inline void AQSensor_get_tvoc(void * const pVoid){
    struct AQSensor * const self = (struct AQSensor *)pVoid;
    self->vptr->get_tvoc(pVoid);
}

static inline void AQSensor_get_co2(void * const pVoid){
    struct AQSensor * const self = (struct AQSensor *)pVoid;
    self->vptr->get_co2(pVoid);
}


extern comms_i2c_interface_t Generic_Comms;

// void __delay_ms(int NNN);
// #endif