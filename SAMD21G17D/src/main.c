/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "Embedded-framework-AQ-sensor/SGP30.h"
#include "Embedded-framework-AQ-sensor/EN2.h"
#include "Embedded-framework-AQ-sensor/CCS811.h"
#include "Embedded-framework-AQ-sensor/AQ11.h"


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
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
    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

