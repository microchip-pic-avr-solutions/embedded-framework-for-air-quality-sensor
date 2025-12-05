 /*
 * MAIN Generated Driver File
 * 
 * @file main.c
 * 
 * @defgroup main MAIN
 * 
 * @brief This is the generated driver implementation file for the MAIN driver.
 *
 * @version MAIN Driver Version 1.0.2
 *
 * @version Package Version: 3.1.2
*/

/*
© [2025] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/
#include "mcc_generated_files/system/system.h"
#include "Embedded-framework-AQ-sensor/SGP30.h"
#include "Embedded-framework-AQ-sensor/EN2.h"
#include "Embedded-framework-AQ-sensor/CCS811.h"
#include "Embedded-framework-AQ-sensor/AQ11.h"
/*
    Main application
*/

int main(void)
{
    SYSTEM_Initialize();
    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts 
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global Interrupts 
    // Use the following macros to: 

    // Enable the Global Interrupts 
    INTERRUPT_GlobalInterruptEnable(); 

    // Disable the Global Interrupts 
    //INTERRUPT_GlobalInterruptDisable(); 

    struct SGP a1;
    struct EN2 b1;
    struct CCS c1;
    struct ENS161 d1;
    SGP_ctor(&a1, 0, 0, 0);
    EN2_ctor(&b1, 0, 0, 0);
    CCS_ctor(&c1, 0, 0, 0);
    ENS161_ctor(&d1, 0, 0, 0);
    
    AQSensor_init(&a1);
    AQSensor_init(&b1);
    AQSensor_init(&c1);
    AQSensor_init(&d1);
    
    AQSensor_get_id(&a1);
    AQSensor_get_id(&c1);
    AQSensor_get_id(&d1);
    
    while(1)
    {
        AQSensor_get_tvoc(&a1);
        AQSensor_get_co2(&a1);        
        AQSensor_get_tvoc(&b1);
        AQSensor_get_tvoc(&c1);
        AQSensor_get_co2(&c1);
        AQSensor_get_tvoc(&d1);
        AQSensor_get_co2(&d1);
        printf("SGP30 \t\tTVOC:%d CO2:%d \n",a1.parent.TVOC,a1.parent.CO2);
        printf("EN2 \t\tVOC:%d \n",b1.parent.TVOC);
        printf("AQ11 \t\tTVOC:%d CO2:%d \n",d1.parent.TVOC,d1.parent.CO2);
        printf("CCS811 \t\tTVOC:%d CO2:%d \n\n\n",c1.parent.TVOC,c1.parent.CO2);
    }    
}