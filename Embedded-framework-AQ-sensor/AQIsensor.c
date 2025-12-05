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

#include <xc.h>
#include "AQIsensor.h"



/* define custom functions, example provided below*/
void AQSensor_init_b(void * const pVoid)
{
    (void)pVoid;
}

void AQSensor_get_id_b(void * const pVoid)
{
    (void)pVoid;
}


void AQSensor_get_tvoc_b(void * const pVoid)
{
    (void)pVoid;
}

void AQSensor_get_co2_b(void * const pVoid)
{
    (void)pVoid;
}


void AQSensor_ctor(struct AQSensor * const self, uint16_t x, uint16_t y,uint16_t z)
{

    static struct AQSensorVtbl const vtbl =                                       //Create an instance of the virtual pointer table    
    {
        /* Add custom functions, example provided below*/
        &AQSensor_init_b,
        &AQSensor_get_id_b,
        &AQSensor_get_tvoc_b,
        &AQSensor_get_co2_b,
                
    };

    
    self->vptr = &vtbl;                                                         //vptr pointer points to instance vtbl
    
   
    self->ID = x;                                                               //load sensor values 
    self->TVOC = y;
    self->CO2 = z;
}

comms_i2c_interface_t Generic_Comms = {
    .Write = ,
    .Read = 
    .IsBusy = ,
    .WriteRead = ,
    .Tasks = ,
    .isInterruptDriven = 
};

// void __delay_ms(int NNN)                                                        //PIC32 doesn't have an inbuilt delay function
// {
// for (int i = 0; i < NNN*10000; i++)
//   asm("nop");
// }