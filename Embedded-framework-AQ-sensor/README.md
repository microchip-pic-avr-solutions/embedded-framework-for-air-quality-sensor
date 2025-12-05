<!-- Please do not change this logo with link -->

<a target="_blank" href="https://www.microchip.com/" id="top-of-page">
   <picture>
      <source media="(prefers-color-scheme: light)" srcset="../images/mchp_logo_light.png" width="350">
      <source media="(prefers-color-scheme: dark)" srcset="../images/mchp_logo_dark.png" width="350">
      <img alt="Microchip Technologies Inc." src="https://www.microchip.com/content/experience-fragments/mchp/en_us/site/header/master/_jcr_content/root/responsivegrid/header/logo.coreimg.100.300.png/1605828081463/microchip.png">
   </picture>
</a>

# Embedded-framework-for-air-quality-sensor

### Project content 
| Name  | Content |
| ------------- | ------------- |
| generic_driver_interfaces | generic driver melody |
| AQ11.c | Source code for ENS161 sensor |
| AQ11.h | Header file for ENS161 sensor  |
| AQIsensor.c | Generic API source code file |
| AQIsensor.h | Generic API header file  |
| CCS811.c | Source code for CCS811 sensor |
| CCS811.h | Header file for CCS811 sensor |
| EN2.c | Source code for SGP40 and SHT40 sensor |
| EN2.h | Header file for SGP40 and SHT40 sensor  |
| SGP30.c | Source code for SGP30 sensor  |
| SGP30.h | Header file for SGP30 sensors |


### Sensor Informaiton
1. [Air Quality 11](https://www.mikroe.com/air-quality-11-click): 
    - The board features ScioSense [ENS161](https://download.mikroe.com/documents/datasheets/ENS161_datasheet.pdf), a digital multi-gas sensor based on a metal oxide (MOX) technology    
    - It detects a wide range of Volatile Organic Compounds (VOCs) and calculates equivalent CO2 (eCO2), Total VOCs (TVOC), and Air Quality Index (AQI) values  
    - Offering flexible integration, it communicates via selectable I2C or SPI interfaces through the standard mikroBUS™ socket
    - Please note that the Air Qaulity 11 has additional GPIOs required compared to other sensors

2. [Air Quality 3](https://www.mikroe.com/air-quality-3-click): 
    - The board features a [CCS811](https://download.mikroe.com/documents/datasheets/CCS811.pdf) digital gas sensor for monitoring indoor air quality (IAQ)    
    - It measures Total Volatile Organic Compounds (TVOC) concentrations and equivalent CO2 (eCO2) levels   
    - Communicating via the I2C interface, it easily integrates with host systems using the standard mikroBUS™ socket
    - Please note that the Air Qaulity 3 has additional GPIOs required compared to other sensors

3. [Environmental Click 2](https://www.mikroe.com/environment-2-click#:~:text=This%20Click%20board%E2%84%A2%20can,via%20the%20VIO%20SEL%20jumper.): 
    - The board combines Sensirion's [SHT40](https://sensirion.com/media/documents/33FD6951/67EB9032/HT_DS_Datasheet_SHT4x_5.pdf) humidity/temperature sensor and the [SGP40](https://sensirion.com/media/documents/296373BB/6203C5DF/Sensirion_Gas_Sensors_Datasheet_SGP40.pdf) MOx-based gas sensor   
    - It provides accurate relative humidity and temperature readings alongside a processed VOC Index for indoor air quality assessment   
    - Communicating via I2C, it supports both 3.3V and 5V logic levels for easy integration using the mikroBUS™ socket

4. [Air Quality 4](https://www.mikroe.com/air-quality-4-click):
    - The board features Sensirion [SGP30](https://download.mikroe.com/documents/datasheets/SGP30_Datasheet.pdf), a multi-pixel metal-oxide gas sensor   
    - It measures indoor air quality, providing readings for Total Volatile Organic Compounds (TVOC) and equivalent CO2 (eCO2)
    - Communicating via the I2C interface, it integrates with 3.3V or 5V systems on the mikroBUS™ standard