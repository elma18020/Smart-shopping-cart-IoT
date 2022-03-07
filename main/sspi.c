#include "sspi.h"
#include "debug.h"

void initSSPI(void)
{
	//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
	// Software Emulation of SPI 
	/// PA4  = SSPI CS
	/// PA1  = SSPI SCK
	/// PA2  = SSPI MOSI
	/// PA3  = SSPI MISO
	//------------------------------------------------------------------
	gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
	//
    io_conf.pin_bit_mask =  (1 << SSPI_SCK_PIN) | (1 << SSPI_CS_PIN) | (1 << SSPI_MOSI_PIN);
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
	//
    io_conf.pin_bit_mask =  (1 << SSPI_MISO_PIN);
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

	SSPI_CS_HIGH;
	SSPI_SCK_LOW;
	SSPI_MOSI_HIGH;
	
#ifdef SYSTEM_DEBUG
	/* printUART("-> SSPI: ready...\n"); */
#endif
}	
uint8_t txByteSSPI(uint8_t data)
{
	/* printDEBUG(DSYS,"SPI tx byte[%xb]\n",data); */
	uint8_t k;
	uint8_t rx_data = 0x0000 ;
	for(k=0;k<8;k++)
	{
		SSPI_SCK_LOW;
		if(data & 0x80)
		{
			SSPI_MOSI_HIGH;
		}
		else
		{
			SSPI_MOSI_LOW;
		}
		data = data<<1;
		rx_data = rx_data << 1;
		rx_data |= SSPI_MISO_READ ? 0x01 : 0x00;
		// delay10ns(SSPI_HALF_CLOCK_TIME);
		__asm__("nop;nop;");				// 8-nops 0.5us
		/* __asm__("nop;nop;nop;nop;nop;nop;nop;nop;");				// 8-nops 0.5us */
		// __asm__("sev;nop;nop;nop;nop;nop;nop;nop;nop;sev;");				// 8-nops 0.5us
		SSPI_SCK_HIGH;
		//delay10ns(SSPI_HALF_CLOCK_TIME);
		/* __asm__("nop;nop;nop;nop;");				// 8-nops 0.5us */
		__asm__("nop;nop;nop;nop;");				// 8-nops 0.5us
		/* __asm__("nop;nop;nop;nop;nop;nop;nop;nop;");				// 8-nops 0.5us */
		// __asm__("sev;nop;nop;nop;nop;nop;nop;nop;nop;sev;");				// 8-nops 0.5us
	}
	SSPI_SCK_LOW;
	return rx_data;
}

uint8_t rxByteSSPI(void)
{
	uint8_t k;
	uint8_t data = 0;
	for(k=0;k<8;k++)
	{
		data = data<<1;
		SSPI_SCK_LOW;
		
		
		//delay10ns(SSPI_HALF_CLOCK_TIME);
		//__asm__("sev;nop;nop;nop;nop;nop;nop;nop;nop;sev;");				// 8-nops 0.5us
		/* __asm__("nop;nop;nop;nop;");				// 8-nops 0.5us */
		__asm__("nop;nop;nop;nop;nop;nop;nop;nop;");				// 8-nops 0.5us
		// __asm__("sev;nop;nop;nop;nop;nop;nop;nop;nop;sev;");				// 8-nops 0.5us
		// __asm__("sev;nop;nop;nop;nop;nop;nop;nop;nop;sev;");				// 8-nops 0.5us
		// __asm__("sev;nop;nop;nop;nop;nop;nop;nop;nop;sev;");				// 8-nops 0.5us
		// __asm__("sev;nop;nop;nop;nop;nop;nop;nop;nop;sev;");				// 8-nops 0.5us
		SSPI_SCK_HIGH;
		/* delay10ns(SSPI_HALF_CLOCK_TIME); */
		/* __asm__("nop;nop;nop;nop;");				// 8-nops 0.5us */
		__asm__("nop;nop;nop;nop;nop;nop;nop;nop;");				// 8-nops 0.5us
		// __asm__("sev;nop;nop;nop;nop;nop;nop;nop;nop;sev;");				// 8-nops 0.5us
		// __asm__("sev;nop;nop;nop;nop;nop;nop;nop;nop;sev;");				// 8-nops 0.5us
		// __asm__("sev;nop;nop;nop;nop;nop;nop;nop;nop;sev;");				// 8-nops 0.5us
		// __asm__("sev;nop;nop;nop;nop;nop;nop;nop;nop;sev;");				// 8-nops 0.5us
		if(SSPI_MISO_READ)
		{
			data |= 0x01;
		}
	}
	SSPI_SCK_LOW;
	
	return data;
}

void txSSPI(uint8_t * txdata,uint8_t * rxdata, uint32_t size)
{
	uint32_t k;
	for(k=0;k<size;k++)
	{
		rxdata[k] = txByteSSPI(txdata[k]);
	}
}

void rxSSPI(uint8_t * data, uint32_t size)
{
	uint32_t k;
	for(k=0;k<size;k++)
	{
		data[k] = rxByteSSPI();
	}
}




