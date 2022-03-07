#include "debug.h"
#include "misc.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"

/* Private types *********************************************************** */

/* Private constants ******************************************************* */

/* Private macros ********************************************************** */
#define BUF_SIZE (1024)

/* Private variables ******************************************************* */


volatile uint8_t g_core_id[17] = {0};
volatile uint8_t g_logmsg_ccode = '5';
const uint8_t c_mod[DSIZE][32] =
{
    {"SYS"},

};

volatile uint8_t g_debug_block_flag = 1;


static void initUART(uint32_t baudrate);
static void putcharUART(uint8_t data);
static void sprintUART(uint8_t * str);

void initDEBUG(char * str, uint8_t ccode, uint32_t baudrate, char * app)
{
	uint8_t	k;
	
	
	g_logmsg_ccode = ccode;
	for(k=0;k<15;k++)
	{
		g_core_id[k] = str[k];
		if(str[k] == 0)
		{
			break;
		}
	}

	initUART(baudrate);

    // clear terminal
    putcharUART(0x1b);
    putcharUART(0x5b);
    putcharUART(0x32);
    putcharUART(0x4a);

    printDEBUG(DAPPEND,"\n\n");
	printDEBUG(DAPPEND | DHEADER,"wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww\n");
	printDEBUG(DAPPEND | DHEADER,"w %s\n",app);
	printDEBUG(DAPPEND | DHEADER ,"wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww\n");
}

void printDEBUG(uint16_t sid, char *str, ... )
{
//#ifndef SYSTEM_DEBUG
	//return;
//#endif

	uint8_t rstr[40];													// 33 max -> 32 ASCII for 32 bits and NULL
	uint16_t k = 0;
	uint16_t arg_type;
	uint32_t utmp32;
	uint32_t * p_uint32;
	char * p_char;
	va_list vl;

#ifdef LINUX_OS
	if((sid & DAPPEND) == 0x00)
	{
		putcharUART('\e');
		putcharUART('[');
		putcharUART('3');
		putcharUART(g_logmsg_ccode);
		putcharUART('m');

		for(k=0; k<16; k++)
		{
			putcharUART(g_core_id[k]);
		}


		putcharUART('\e');
		putcharUART('[');
		putcharUART('3');
		putcharUART('4');
		putcharUART('m');
		putcharUART('-');
		putcharUART('>');
		putcharUART(' ');
		uint16_t idx = sid & 0x03FF;
		k = 0;
		while(c_mod[idx][k] != '\0')
		{
			putcharUART((uint8_t)c_mod[idx][k]);
			k++;
		}

		putcharUART(':');
		putcharUART(' ');
		
		putcharUART('\e');
		putcharUART('[');
		putcharUART('0');
		putcharUART('m');
	}
	if(sid & DERROR)
	{
		putcharUART('\e');
		putcharUART('[');
		putcharUART('3');
		putcharUART('1');
		putcharUART('m');
	}
	else if(sid & (DDEBUG))
	{
		putcharUART('\e');
		putcharUART('[');
		putcharUART('3');
		putcharUART('2');
		putcharUART('m');
	}
	else if(sid & (DWARNING))
	{
		putcharUART('\e');
		putcharUART('[');
		putcharUART('3');
		putcharUART('3');
		putcharUART('m');
	}
	else if(sid & (DNOTIFY))
	{
		putcharUART('\e');
		putcharUART('[');
		putcharUART('3');
		putcharUART('6');
		putcharUART('m');
	}
	else if(sid & (DHEADER))
	{
		putcharUART('\e');
		putcharUART('[');
		putcharUART('3');
		putcharUART('5');
		putcharUART('m');
	}


#endif	


	k = 0;
	//va_start(vl, 10);													// always pass the last named parameter to va_start, for compatibility with older compilers
	va_start(vl, str);													// always pass the last named parameter to va_start, for compatibility with older compilers
	while(str[k] != 0x00)
	{
		if(str[k] == '%')
		{
			if(str[k+1] != 0x00)
			{
				switch(str[k+1])
				{
					case('b'):
					{
						// binary
						if(str[k+2] == 'b')
						{
							// byte
							utmp32 = va_arg(vl, int);
							arg_type = (PRINT_ARG_TYPE_BINARY_BYTE);
						}
						else if(str[k+2] == 'h')
						{
							// half word
							utmp32 = va_arg(vl, int);
							arg_type = (PRINT_ARG_TYPE_BINARY_HALFWORD);
						}
						else if(str[k+2] == 'w')
						{
							// word
							utmp32 = va_arg(vl, uint32_t);
							arg_type = (PRINT_ARG_TYPE_BINARY_WORD);
						}
						else
						{
							// default word
							utmp32 = va_arg(vl, uint32_t);
							arg_type = (PRINT_ARG_TYPE_BINARY_WORD);
							k--;
						}

						k++;
						p_uint32 = &utmp32;
						break;
					}
					case('d'):
					{
						// decimal
						if(str[k+2] == 'b')
						{
							// byte
							utmp32 = va_arg(vl, int);
							arg_type = (PRINT_ARG_TYPE_DECIMAL_BYTE);
						}
						else if(str[k+2] == 'h')
						{
							// half word
							utmp32 = va_arg(vl, int);
							arg_type = (PRINT_ARG_TYPE_DECIMAL_HALFWORD);
						}
						else if(str[k+2] == 'w')
						{
							// word
							utmp32 = va_arg(vl, uint32_t);
							arg_type = (PRINT_ARG_TYPE_DECIMAL_WORD);
						}
						else
						{
							// default word
							utmp32 = va_arg(vl, uint32_t);
							arg_type = (PRINT_ARG_TYPE_DECIMAL_WORD);
							k--;
						}

						k++;
						p_uint32 = &utmp32;
						break;
					}
					case('c'):
					{
						// character
						char tchar = va_arg(vl, int);
						putcharUART(tchar);
						arg_type = (PRINT_ARG_TYPE_CHARACTER);
						break;
					}
					case('s'):
					{
						// string
						p_char = va_arg(vl, char *);
						sprintUART((uint8_t *)p_char);
						arg_type = (PRINT_ARG_TYPE_STRING);
						break;
					}
					case('f'):
					{
						// float
						uint64_t utmp64 = va_arg(vl, uint64_t);			// convert double to float representation IEEE 754
						uint32_t tmp1 = utmp64&0x00000000FFFFFFFF;
						tmp1 = tmp1>>29;
						utmp32 = utmp64>>32;
						utmp32 &= 0x07FFFFFF;
						utmp32 = utmp32<<3;
						utmp32 |= tmp1;
						if(utmp64 & 0x8000000000000000)
							utmp32 |= 0x80000000;

						if(utmp64 & 0x4000000000000000)
							utmp32 |= 0x40000000;

						p_uint32 = &utmp32;

						arg_type = (PRINT_ARG_TYPE_FLOAT);
						//arg_type = (PRINT_ARG_TYPE_HEXADECIMAL_WORD);
						//arg_type = (PRINT_ARG_TYPE_BINARY_WORD);
						break;
					}
					case('x'):
					{
						// hexadecimal
						if(str[k+2] == 'b')
						{
							// byte
							utmp32 = (uint32_t)va_arg(vl, int);
							arg_type = (PRINT_ARG_TYPE_HEXADECIMAL_BYTE);
						}
						else if(str[k+2] == 'h')
						{
							// half word
							utmp32 = (uint32_t)va_arg(vl, int);
							arg_type = (PRINT_ARG_TYPE_HEXADECIMAL_HALFWORD);
						}
						else if(str[k+2] == 'w')
						{
							// word
							utmp32 = va_arg(vl, uint32_t);
							arg_type = (PRINT_ARG_TYPE_HEXADECIMAL_WORD);
						}
						else
						{
							utmp32 = va_arg(vl, uint32_t);
							arg_type = (PRINT_ARG_TYPE_HEXADECIMAL_WORD);
							k--;
						}

						k++;
						p_uint32 = &utmp32;
						break;
					}
					default:
					{
						utmp32 = 0;
						p_uint32 = &utmp32;
						arg_type = (PRINT_ARG_TYPE_UNKNOWN);
						break;
					}
				}

				if(arg_type&(PRINT_ARG_TYPE_MASK_CHAR_STRING))
				{
					getStr4NumMISC(arg_type, p_uint32, rstr);
					sprintUART(rstr);
				}
				k++;
			}
		}
		else
		{
			// not a '%' char -> print the char
			putcharUART(str[k]);
			if (str[k] == '\n')
			{
				putcharUART('\r');
			}
		}
		k++;
	}

	va_end(vl);

#ifdef LINUX_OS
	if(sid & 0xFC00)
	{
		putcharUART('\e');
		putcharUART('[');
		putcharUART('0');
		putcharUART('m');
	}
#endif	
	
}


void initUART(uint32_t baudrate)
{
   // Configure parameters of an UART driver,
    // communication pins and install the driver
    uart_config_t uart_config =
	{
        .baud_rate = baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);
}

void putcharUART(uint8_t data)
{/// print one character to UART1
    uart_write_bytes(UART_NUM_0,(const char*) & data, 1);
}

void printUART(char *str, ... )
{ /// print text and up to 10 arguments!
    uint8_t rstr[40];													// 33 max -> 32 ASCII for 32 bits and NULL 
    uint16_t k = 0;
	uint16_t arg_type;
	uint32_t utmp32;
	uint32_t * p_uint32; 
	char * p_char;
	va_list vl;
	
	//va_start(vl, 10);													// always pass the last named parameter to va_start, for compatibility with older compilers
	va_start(vl, str);													// always pass the last named parameter to va_start, for compatibility with older compilers
	while(str[k] != 0x00)
	{
		if(str[k] == '%')
		{
			if(str[k+1] != 0x00)
			{
				switch(str[k+1])
				{
					case('b'):
					{// binary
						if(str[k+2] == 'b')
						{// byte
							utmp32 = va_arg(vl, int);
							arg_type = (PRINT_ARG_TYPE_BINARY_BYTE);
						}
						else if(str[k+2] == 'h')
						{// half word
							utmp32 = va_arg(vl, int);
							arg_type = (PRINT_ARG_TYPE_BINARY_HALFWORD);
						}
						else if(str[k+2] == 'w')
						{// word	
							utmp32 = va_arg(vl, uint32_t);
							arg_type = (PRINT_ARG_TYPE_BINARY_WORD);
						}
						else
						{// default word
							utmp32 = va_arg(vl, uint32_t);
							arg_type = (PRINT_ARG_TYPE_BINARY_WORD);
							k--;
						}
						
						k++;	
						p_uint32 = &utmp32;
						break;
					}
					case('d'):
					{// decimal
						if(str[k+2] == 'b')
						{// byte
							utmp32 = va_arg(vl, int);
							arg_type = (PRINT_ARG_TYPE_DECIMAL_BYTE);
						}
						else if(str[k+2] == 'h')
						{// half word
							utmp32 = va_arg(vl, int);
							arg_type = (PRINT_ARG_TYPE_DECIMAL_HALFWORD);
						}
						else if(str[k+2] == 'w')
						{// word	
							utmp32 = va_arg(vl, uint32_t);
							arg_type = (PRINT_ARG_TYPE_DECIMAL_WORD);
						}
						else
						{// default word
							utmp32 = va_arg(vl, uint32_t);
							arg_type = (PRINT_ARG_TYPE_DECIMAL_WORD);
							k--;
						}
						
						k++;	
						p_uint32 = &utmp32;
						break;
					}
					case('c'):
					{// character
						char tchar = va_arg(vl, int);	
						putcharUART(tchar);
						arg_type = (PRINT_ARG_TYPE_CHARACTER);
						break;
					}
					case('s'):
					{// string 
						p_char = va_arg(vl, char *);	
						sprintUART((uint8_t *)p_char);
						arg_type = (PRINT_ARG_TYPE_STRING);
						break;
					}
					case('f'):
					{// float
						uint64_t utmp64 = va_arg(vl, uint64_t);			// convert double to float representation IEEE 754
						uint32_t tmp1 = utmp64&0x00000000FFFFFFFF;
						tmp1 = tmp1>>29;
						utmp32 = utmp64>>32;
						utmp32 &= 0x07FFFFFF;
						utmp32 = utmp32<<3;
						utmp32 |= tmp1;
						if(utmp64 & 0x8000000000000000)
							utmp32 |= 0x80000000;
							
						if(utmp64 & 0x4000000000000000)
							utmp32 |= 0x40000000;
							
						p_uint32 = &utmp32;
						
						arg_type = (PRINT_ARG_TYPE_FLOAT);
						//arg_type = (PRINT_ARG_TYPE_HEXADECIMAL_WORD);
						//arg_type = (PRINT_ARG_TYPE_BINARY_WORD);
						break;
					}
					case('x'):
					{// hexadecimal 
						if(str[k+2] == 'b')
						{// byte
							utmp32 = (uint32_t)va_arg(vl, int);
							arg_type = (PRINT_ARG_TYPE_HEXADECIMAL_BYTE);
						}
						else if(str[k+2] == 'h')
						{// half word
							utmp32 = (uint32_t)va_arg(vl, int);
							arg_type = (PRINT_ARG_TYPE_HEXADECIMAL_HALFWORD);
						}
						else if(str[k+2] == 'w')
						{// word	
							utmp32 = va_arg(vl, uint32_t);
							arg_type = (PRINT_ARG_TYPE_HEXADECIMAL_WORD);
						}
						else
						{
							utmp32 = va_arg(vl, uint32_t);
							arg_type = (PRINT_ARG_TYPE_HEXADECIMAL_WORD);
							k--;
						}
						
						k++;
						p_uint32 = &utmp32;
						break;
					}
					default:
					{
						utmp32 = 0;
						p_uint32 = &utmp32;
						arg_type = (PRINT_ARG_TYPE_UNKNOWN);
						break;
					}
				}
					
				if(arg_type&(PRINT_ARG_TYPE_MASK_CHAR_STRING))	
				{
					getStr4NumMISC(arg_type, p_uint32, rstr);
					sprintUART(rstr);	
				}
				k++;
			}
		}
		else
		{// not a '%' char -> print the char
			putcharUART(str[k]);
			if (str[k] == '\n')
				putcharUART('\r');
		}
		k++;
	}
	
	va_end(vl);
		
	return;
}

void sprintUART(uint8_t * str)
{
	uint16_t k = 0;
	
	while (str[k] != '\0')
    {
        putcharUART(str[k]);
        if (str[k] == '\n')
            putcharUART('\r');
        k++;
		
        if (k == MAX_PRINT_STRING_SIZE)
            break;
    }
    
}




