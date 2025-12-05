/**
 * Generic Driver Interfaces API Header File.
 *
 * @file generic_driver_interfaces.h
 *
 * @defgroup generic_driver_interfaces Generic Driver Interfaces
 *
 * @brief This contains the API interface for the Generic Driver Interfaces APIs
 *
 * @version Generic Driver Interfaces Version 1.0.0
 *
 * @version Package Version 1.0.0
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

#ifndef GENERIC_DRIVER_INTERFACES_H
#define GENERIC_DRIVER_INTERFACES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @ingroup generic_driver_interfaces
 * @struct comms_i2c_interface_t
 * @brief Struct for generic I2C driver communication APIs.
 */
typedef struct
{
    bool (*Write)(uint16_t i2cAddress, uint8_t *dataBuffer, size_t dataLength);
    bool (*Read)(uint16_t i2cAddress, uint8_t *dataBuffer, size_t dataLength);
    bool (*WriteRead)(uint16_t i2cAddress, uint8_t *wrDataBuffer, size_t wrDataLength, uint8_t *rdDataBuffer, size_t rdLength);
    void (*CallbackRegister)(void (*callbackHandler)(void));

    void (*Initialize)(void);
    void (*Deinitialize)(void);
    bool (*IsBusy)(void);
    void (*Tasks)(void);
    uint8_t(*ErrorGet)(void);
    bool isInterruptDriven;
} comms_i2c_interface_t;

#endif /* GENERIC_DRIVER_INTERFACES_H */
