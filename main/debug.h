#ifndef __DEBUG_H__
#define __DEBUG_H__

/* Includes **************************************************************** */
#include <stdint.h>
#include <stdarg.h>

/* Exported types ********************************************************** */
typedef enum 
{
	DSYS = 0x0000,
	
	DSIZE,

	DDEBUGEXT = 0x1000,
	DDEBUG = 0x2000,
	DINTERNAL = 0x0400,
	DAPPEND = 0x0800,
	DWARNING = 0x1000,
	DERROR = 0x2000,
	DNOTIFY = 0x4000,
	DHEADER = 0x8000,
} DEBUG_Modules_t;

/* Exported constants ****************************************************** */
#define LINUX_OS

#define DEVICE_ID_SIZE							12
#define DEVICE_ID_REG_ADDR						0x1FFF7590

#define SYSTEM_DEBUG
/* Exported macros ********************************************************* */
#define DEBUG_WAIT4TC_COUNT						100000

#define DEBUG_BLE_LOG_BUFFER_SIZE				1024
#define DEBUG_BLE_LOG_BUFFER_MASK				((DEBUG_BLE_LOG_BUFFER_SIZE)-1)

/* Exported variables ****************************************************** */

/* Exported functions ****************************************************** */
void initDEBUG(char * str, uint8_t ccode, uint32_t baudrate, char * app);
void printDEBUG(uint16_t sid, char *str, ... );

#endif /* __DEBUG_H__ */

/* ***************************** END OF FILE ******************************* */
