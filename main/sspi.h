#ifndef __SSPI_H_
#define __SSPI_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include "esp8266/spi_struct.h"
#include "esp8266/gpio_struct.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "ringbuf.h"

#include "esp8266/spi_struct.h"
#include "esp8266/gpio_struct.h"
#include "esp_system.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/spi.h"

#define SSPI_ERROR						0x00
#define SSPI_OK							0x01
#define SSPI_WAIT4BUSY_PERIOD			2000							// [x1ms]

#define SSPI_CS_PIN						4		
#define SSPI_SCK_PIN					14	
#define SSPI_MOSI_PIN					13	
#define SSPI_MISO_PIN					12	

#define SSPI_CS_HIGH					gpio_set_level(SSPI_CS_PIN,1)
#define SSPI_CS_LOW						gpio_set_level(SSPI_CS_PIN,0)
#define SSPI_SCK_HIGH					gpio_set_level(SSPI_SCK_PIN,1)
#define SSPI_SCK_LOW					gpio_set_level(SSPI_SCK_PIN,0)
#define SSPI_MOSI_HIGH					gpio_set_level(SSPI_MOSI_PIN,1)
#define SSPI_MOSI_LOW					gpio_set_level(SSPI_MOSI_PIN,0)
#define SSPI_MISO_READ					gpio_get_level(SSPI_MISO_PIN)


#define SSPI_HALF_CLOCK_TIME			2
void initSSPI(void);
void deinitSSPI(void);
uint8_t txByteSSPI(uint8_t data);
uint8_t rxByteSSPI(void);
void txSSPI(uint8_t * txdata,uint8_t * rxdata, uint32_t size);
void rxSSPI(uint8_t * data, uint32_t size);

#endif 
